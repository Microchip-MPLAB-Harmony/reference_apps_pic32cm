/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/
//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#include <string.h>
#include "atca_basic.h"
#include "app_nvm.h"

#define APP_PROTOCOL_HEADER_MAX_SIZE                13

/* Definitions */
#define APP_ERASE_PAGE_SIZE                         (256L)
#define APP_PROGRAM_PAGE_SIZE                       (256L)
#define NVM_USER_ROW_START                          (0x00804000UL)

#define APP_IMAGE_CONFIG_ADDR                       (0x00400000UL)
#define APP_IMAGE_CONFIG_SIZE                       (9L)

#define APP_IMAGE_DETAIL_ADDR                       (0x00400100UL)
#define APP_IMAGE_DETAIL_SIZE                       (2L)


uint32_t imageDetails[APP_IMAGE_DETAIL_SIZE];
uint32_t imageConfig[APP_IMAGE_CONFIG_SIZE];

// Bootloader Commands
typedef enum
{
    APP_BL_COMMAND_UNLOCK       = 0xA0,
    APP_BL_COMMAND_PROGRAM      = 0xA1,
    APP_BL_COMMAND_VERIFY       = 0xA2,
    APP_BL_COMMAND_RESET        = 0xA3,
    APP_BL_FLASH_STATUS         = 0xA4,
    APP_BL_CMD_DEVCFG_DATA      = 0xA5,
} APP_BL_COMMAND;

// Bootloader Response Codes
typedef enum
{
    BL_RESP_OK          = 0x50,
    BL_RESP_ERROR       = 0x51,
    BL_RESP_INVALID     = 0x52,
    BL_RESP_SIGN_OK     = 0x53,
    BL_RESP_SIGN_FAIL   = 0x54,
    BL_RESP_FLASH_OK    = 0x55,
    BL_RESP_FLASH_FAIL  = 0x56,
} APP_BL_RESPONSE;

// Host Application's State Machine states
typedef enum
{
    APP_STATE_INIT,
    APP_STATE_WAIT_SW_PRESS,
    APP_STATE_TRANSFER_APP_IMAGE,
    APP_STATE_UPDATE_APP_IMAGE,
    APP_STATE_SEND_UNLOCK_COMMAND,
    APP_STATE_WAIT_UNLOCK_COMMAND_TRANSFER_COMPLETE,
    APP_STATE_SEND_WRITE_COMMAND,
    APP_STATE_WAIT_WRITE_COMMAND_TRANSFER_COMPLETE,
    APP_STATE_SEND_DEV_CONFIG_COMMAND,
    APP_STATE_WAIT_DEV_CONFIG_COMMAND_TRANSFER_COMPLETE,
    APP_STATE_SEND_VERIFY_COMMAND,
    APP_STATE_WAIT_VERIFY_COMMAND_TRANSFER_COMPLETE,
    APP_STATE_SEND_RESET_COMMAND,
    APP_STATE_WAIT_RESET_COMMAND_TRANSFER_COMPLETE,
    APP_STATE_SUCCESSFUL,
    APP_STATE_ERROR,
    APP_STATE_IDLE,
} APP_STATES;

// Host Application UART Transfer Status
typedef enum
{
    APP_TRANSFER_STATUS_IN_PROGRESS,
    APP_TRANSFER_STATUS_SUCCESS,
    APP_TRANSFER_STATUS_ERROR,
    APP_TRANSFER_STATUS_IDLE,
} APP_TRANSFER_STATUS;

// Host Application Data
typedef struct
{
    APP_STATES                      state;
    volatile APP_TRANSFER_STATUS    readStatus;
    volatile APP_TRANSFER_STATUS    writeStatus;
    uint32_t                        appMemStartAddr;
    uint32_t                        nBytesWritten;
    uint8_t                         wrBuffer[APP_PROGRAM_PAGE_SIZE + APP_PROTOCOL_HEADER_MAX_SIZE + 16];
    uint8_t                         rdBuffer[APP_PROTOCOL_HEADER_MAX_SIZE];
} APP_DATA;

APP_DATA                            appData;

// Bootloader Guard Bytes
static uint8_t btl_guard[4] = {0x4D, 0x43, 0x48, 0x50};

extern ATCAIfaceCfg atecc608_0_init_data;

atca_aes_gcm_ctx_t ctx;

uint8_t plaintext[256];
uint8_t ciphertext[256];
uint8_t digest[32];
uint8_t signature[64];
uint8_t public_key[64];
uint8_t ciphertag[16];
uint8_t tag[16];

uint16_t key_id = 0x05;
uint8_t key_block = 0;

// AES Encryption Parameters

// Initialization Vector
uint8_t iv[] =  {
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b
                };

// AES GCM Nonce
uint8_t nonce[] = {
                    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
                };

// Read Call-backs
static void APP_UART_Read_Event_Handler(uintptr_t context )
{
    APP_TRANSFER_STATUS* readStatus = (APP_TRANSFER_STATUS*)context;

    if(SERCOM4_USART_ErrorGet() == USART_ERROR_NONE)
    {
        switch(appData.rdBuffer[0])
        {
            case BL_RESP_OK:
            case BL_RESP_SIGN_OK:
            {
                *readStatus = APP_TRANSFER_STATUS_SUCCESS;
                break;
            }

            case BL_RESP_ERROR:
            case BL_RESP_INVALID:
            case BL_RESP_SIGN_FAIL:
            {
                *readStatus = APP_TRANSFER_STATUS_ERROR;
                break;
            }
        }
    }
}

// Write Call-backs
static void APP_UART_Write_Event_Handler(uintptr_t context )
{
    APP_TRANSFER_STATUS* writeStatus = (APP_TRANSFER_STATUS*)context;

    if(SERCOM4_USART_ErrorGet() == USART_ERROR_NONE)
    {
        if (writeStatus)
        {
            *writeStatus = APP_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (writeStatus)
        {
            *writeStatus = APP_TRANSFER_STATUS_ERROR;
        }
    }
}

// Secure Element Initialization
static bool APP_ECC_Init ( void )
{
    uint8_t serial_number[9];

    /* Initialize CAL */
    if (atcab_init(&atecc608_0_init_data) != ATCA_SUCCESS)
    {
        return false;
    }

    /* Request the Serial Number */
    if (atcab_read_serial_number(serial_number) != ATCA_SUCCESS)
    {
        return false;
    }

    return true;
}

// Initialization API for Host Application
static void APP_Initialize(void)
{
    appData.appMemStartAddr = imageDetails[0];
    appData.nBytesWritten   = 0;
}

//Flash Unlock Command Transfer API for Target Device
static uint32_t APP_UnlockCommandSend(uint32_t appStartAddr, uint32_t appSize)
{
    uint32_t nTxBytes = 0;

    appData.wrBuffer[nTxBytes++] = btl_guard[0];
    appData.wrBuffer[nTxBytes++] = btl_guard[1];
    appData.wrBuffer[nTxBytes++] = btl_guard[2];
    appData.wrBuffer[nTxBytes++] = btl_guard[3];
    appData.wrBuffer[nTxBytes++] = 0x8;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = APP_BL_COMMAND_UNLOCK;
    appData.wrBuffer[nTxBytes++] = (appStartAddr);
    appData.wrBuffer[nTxBytes++] = (appStartAddr >> 8);
    appData.wrBuffer[nTxBytes++] = (appStartAddr >> 16);
    appData.wrBuffer[nTxBytes++] = (appStartAddr >> 24);
    appData.wrBuffer[nTxBytes++] = (appSize);
    appData.wrBuffer[nTxBytes++] = (appSize >> 8);
    appData.wrBuffer[nTxBytes++] = (appSize >> 16);
    appData.wrBuffer[nTxBytes++] = (appSize >> 24);

    return nTxBytes;
}

// Data Encryption Transfer API for Device/Target Application Image Chunks
static uint32_t APP_ImageDataWrite(uint32_t memAddr, uint32_t nBytes)
{
    uint32_t nTxBytes = 0;
    uint32_t buf_offset = 0;

    nBytes = nBytes + sizeof(tag) + 4;

    appData.wrBuffer[nTxBytes++] = btl_guard[0];
    appData.wrBuffer[nTxBytes++] = btl_guard[1];
    appData.wrBuffer[nTxBytes++] = btl_guard[2];
    appData.wrBuffer[nTxBytes++] = btl_guard[3];
    appData.wrBuffer[nTxBytes++] = (nBytes);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 8);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 16);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 24);
    appData.wrBuffer[nTxBytes++] = APP_BL_COMMAND_PROGRAM;
    appData.wrBuffer[nTxBytes++] = (memAddr);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 8);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 16);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 24);

    memcpy(plaintext,(void*)(memAddr), APP_PROGRAM_PAGE_SIZE);

    /* Initialize the AES GCM Mode to encrypt plain text*/
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv)) != ATCA_SUCCESS)
    {
        return false;
    }

    do
    {
        if(atcab_aes_gcm_encrypt_update(&ctx, &plaintext[buf_offset], 16,
                &ciphertext[buf_offset]) != ATCA_SUCCESS)
        {
            return false;
        }

        else
        {
            buf_offset += 16;
        }

    }while (buf_offset < APP_PROGRAM_PAGE_SIZE);

    if(atcab_aes_gcm_encrypt_finish(&ctx, tag, sizeof(tag)) != ATCA_SUCCESS)
    {
        return false;
    }

    memcpy(&appData.wrBuffer[nTxBytes], ciphertext, sizeof(ciphertext));
    nTxBytes += sizeof(ciphertext);

    /* Initialize the AES GCM Mode to encrypt tag */
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv)) != ATCA_SUCCESS)
    {
        return false;
    }

    if(atcab_aes_gcm_encrypt_update(&ctx, tag, 16, ciphertag) != ATCA_SUCCESS)
    {
        return false;
    }

    memcpy(&appData.wrBuffer[nTxBytes], ciphertag, sizeof(ciphertag));
    nTxBytes += sizeof(ciphertag);

    return nTxBytes;
}

// Data Encryption Transfer API for Device/Target Configuration Bits
static uint32_t APP_DevConfigCommandSend(uint32_t memAddr, uint32_t nBytes)
{
    uint32_t k;
    uint32_t nTxBytes = 0;
    uint32_t buf_offset = 0;

    uint8_t *ptr = (uint8_t *) &imageConfig;

    /* Fill up the remaining bytes for flash page write*/
    for (k = 0; k < nBytes; k++)
    {
        if(k < sizeof(imageConfig))
        {
            plaintext[k] = ptr[k] ;
        }

        else
        {
            plaintext[k] = 0xFF;
        }
    }

    nBytes = nBytes + sizeof(tag) + 4;

    appData.wrBuffer[nTxBytes++] = btl_guard[0];
    appData.wrBuffer[nTxBytes++] = btl_guard[1];
    appData.wrBuffer[nTxBytes++] = btl_guard[2];
    appData.wrBuffer[nTxBytes++] = btl_guard[3];
    appData.wrBuffer[nTxBytes++] = (nBytes);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 8);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 16);
    appData.wrBuffer[nTxBytes++] = (nBytes >> 24);
    appData.wrBuffer[nTxBytes++] = APP_BL_CMD_DEVCFG_DATA;
    appData.wrBuffer[nTxBytes++] = (memAddr);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 8);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 16);
    appData.wrBuffer[nTxBytes++] = (memAddr >> 24);

    /* Initialize the AES GCM Mode to encrypt plain text */
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv)) != ATCA_SUCCESS)
    {
        return false;
    }

    do
    {
        if(atcab_aes_gcm_encrypt_update(&ctx, &plaintext[buf_offset], 16,
                &ciphertext[buf_offset]) != ATCA_SUCCESS)
        {
            return false;
        }

        else
        {
            buf_offset += 16;
        }

    }while (buf_offset < APP_PROGRAM_PAGE_SIZE);

    if(atcab_aes_gcm_encrypt_finish(&ctx, tag, sizeof(tag)) != ATCA_SUCCESS)
    {
        return false;
    }

    memcpy(&appData.wrBuffer[nTxBytes], ciphertext, sizeof(ciphertext));
    nTxBytes += sizeof(ciphertext);

    /* Initialize the AES GCM Mode to encrypt tag*/
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv)) != ATCA_SUCCESS)
    {
        return false;
    }

    if(atcab_aes_gcm_encrypt_update(&ctx, tag, 16, ciphertag) != ATCA_SUCCESS)
    {
        return false;
    }

    memcpy(&appData.wrBuffer[nTxBytes], ciphertag, sizeof(ciphertag));
    nTxBytes += sizeof(ciphertag);

    return nTxBytes;
}

//Digital Signature Generation API for Application (Target) Image
static bool APP_GenerateSignature(void)
{
    uint32_t buf_offset = 0;

    /* Start sha256 */
    if (atcab_sha_start() != ATCA_SUCCESS)
    {
        return false;
    }

    do
    {
        if(atcab_sha_update((void*)(imageDetails[0] + buf_offset)) != ATCA_SUCCESS)
        {
            return false;
        }

        else
        {
            buf_offset += SHA_DATA_MAX;
        }

    }while (buf_offset < imageDetails[1]);

    if(atcab_sha_end(digest, 0, NULL) != ATCA_SUCCESS)
    {
        return false;
    }

    /* Generate key pair */
    if ( atcab_get_pubkey(0, public_key) != ATCA_SUCCESS)
    {
        return false;
    }

    /* Sign message */
    if ( atcab_sign(0, digest, signature) != ATCA_SUCCESS)
    {
        return false;
    }

    return true;
}

// Verification Command Transfer API for the Target Device
static uint32_t APP_VerifyCommandSend(void)
{
    uint32_t nTxBytes = 0;

    if(APP_GenerateSignature() == false)
    {
        return false;
    }

    appData.wrBuffer[nTxBytes++] = btl_guard[0];
    appData.wrBuffer[nTxBytes++] = btl_guard[1];
    appData.wrBuffer[nTxBytes++] = btl_guard[2];
    appData.wrBuffer[nTxBytes++] = btl_guard[3];
    appData.wrBuffer[nTxBytes++] = 0x80;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = APP_BL_COMMAND_VERIFY;

    memcpy(&appData.wrBuffer[nTxBytes], signature, sizeof(signature));
    nTxBytes += sizeof(signature);

    memcpy(&appData.wrBuffer[nTxBytes], public_key, sizeof(public_key));
    nTxBytes += sizeof(public_key);

    return nTxBytes;
}

// Reset Command Transfer API for Target Device
static uint32_t APP_ResetCommandSend(void)
{
    uint32_t nTxBytes = 0;

    appData.wrBuffer[nTxBytes++] = btl_guard[0];
    appData.wrBuffer[nTxBytes++] = btl_guard[1];
    appData.wrBuffer[nTxBytes++] = btl_guard[2];
    appData.wrBuffer[nTxBytes++] = btl_guard[3];
    appData.wrBuffer[nTxBytes++] = 0x4;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = APP_BL_COMMAND_RESET;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;
    appData.wrBuffer[nTxBytes++] = 0x0;

    return nTxBytes;
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    uint32_t nTxBytes;
    float temp;

    /* Initialize all modules */
    SYS_Initialize ( NULL );    

    while ( true )
    {
        switch (appData.state)
        {
            case APP_STATE_INIT:
            {                
                LED0_Off();
                LED1_Off();
                
                SYSTICK_TimerStart();
    
                printf("\n\r####### Host Application #######\n\r");
                printf("-> Press SW0 to transfer the firmware (Host -> Client) \n\r");
                printf("-> Press SW1 to upgrade the client firmware (PC -> Host) \n\r");
                printf("\n\r Note: Disconnect the console for upgrade the client firmware \n\r");                

                appData.state = APP_STATE_WAIT_SW_PRESS;
                break;
            }


            case APP_STATE_WAIT_SW_PRESS:
            {
                if (SW0_Get() == SW0_STATE_PRESSED)
                {                  
                    NVMCTRL_Read(imageDetails, sizeof(imageDetails), APP_IMAGE_DETAIL_ADDR);
                    while(NVMCTRL_IsBusy() == true);
                                        
                    NVMCTRL_Read(imageConfig, sizeof(imageConfig), APP_IMAGE_CONFIG_ADDR);
                    while(NVMCTRL_IsBusy() == true);
                    
                    if ((imageDetails[0] != 0xFFFFFFFF) && (imageDetails[1] != 0xFFFFFFFF))
                    {
                        appData.state = APP_STATE_TRANSFER_APP_IMAGE;
                        LED0_On();
                    }
                    
                    else
                    {
                        printf("\n\r No firmware found in the Host. Upgrade the client firmware from PC \n\r");    
                        appData.state = APP_STATE_INIT;
                    }                    
                }
                
                if (SW1_Get() == SW1_STATE_PRESSED)
                {
                    appData.state = APP_STATE_UPDATE_APP_IMAGE;
                    LED1_On();
                }
                
                SYSTICK_DelayMs(250);
                break;
            }
            
            
            case APP_STATE_TRANSFER_APP_IMAGE:
            {
                APP_Initialize();
                
                if(APP_ECC_Init() == false)
                {
                    printf("\n\r Secure Element Initialization Failed...!\n\r");
                }
                    
                SERCOM4_USART_WriteCallbackRegister( APP_UART_Write_Event_Handler,
                        (uintptr_t)&appData.writeStatus );
                
                SERCOM4_USART_ReadCallbackRegister( APP_UART_Read_Event_Handler,
                        (uintptr_t)&appData.readStatus );
                
                appData.state = APP_STATE_SEND_UNLOCK_COMMAND;
                break;
            }
            
            
            case APP_STATE_UPDATE_APP_IMAGE:
            {
                if(APP_NVM_UART_Tasks() == true)
                {
                    LED1_Off();
                    
                    appData.state = APP_STATE_INIT;                    
                }

                break;
            }


            case APP_STATE_SEND_UNLOCK_COMMAND:
            {
                nTxBytes = APP_UnlockCommandSend(imageDetails[0], imageDetails[1]);

                appData.writeStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
                appData.readStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

                memset(appData.rdBuffer, 0, sizeof(appData.rdBuffer));

                SERCOM4_USART_Write(&appData.wrBuffer[0], nTxBytes);
                SERCOM4_USART_Read(&appData.rdBuffer[0], 1);

                printf("\n\r Flash Region Unlock ");

                appData.state = APP_STATE_WAIT_UNLOCK_COMMAND_TRANSFER_COMPLETE;
                break;
            }


            case APP_STATE_WAIT_UNLOCK_COMMAND_TRANSFER_COMPLETE:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    printf(" ....Success \n\n\r\r");
                    
                    appData.state = APP_STATE_SEND_WRITE_COMMAND;
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    printf(" ....Failed \n\n\r\r");
                    
                    appData.state = APP_STATE_ERROR;
                }
                break;
            }


            case APP_STATE_SEND_WRITE_COMMAND:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    nTxBytes = APP_ImageDataWrite((appData.appMemStartAddr + appData.nBytesWritten), APP_PROGRAM_PAGE_SIZE);

                    if(nTxBytes != false)
                    {
                        appData.writeStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
                        appData.readStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

                        memset(appData.rdBuffer, 0, sizeof(appData.rdBuffer));

                        SERCOM4_USART_Write(&appData.wrBuffer[0], nTxBytes);
                        SERCOM4_USART_Read(&appData.rdBuffer[0], 2);

                        appData.state = APP_STATE_WAIT_WRITE_COMMAND_TRANSFER_COMPLETE;
                    }

                    else
                    {
                        appData.state = APP_STATE_ERROR;
                    }
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    appData.state = APP_STATE_ERROR;
                }
                break;
            }


            case APP_STATE_WAIT_WRITE_COMMAND_TRANSFER_COMPLETE:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    if(appData.rdBuffer[1] == BL_RESP_FLASH_OK)
                    {
                        appData.nBytesWritten += APP_PROGRAM_PAGE_SIZE;

                        temp = (appData.nBytesWritten / (float) imageDetails[1]) * 100;

                        printf("\r Programming: ");

                        for(int i=0; i<temp/5; i++)
                        {
                            printf("%c",178);
                        }

                        printf(" %d%% ", (int)temp);

                        if (appData.nBytesWritten == imageDetails[1])
                        {
                            printf(" ....Completed \n\r");

                            // All the pages are written.
                            appData.state = APP_STATE_SEND_DEV_CONFIG_COMMAND;
                        }

                        else
                        {
                            // Continue to write to the Erased Page
                            appData.state = APP_STATE_SEND_WRITE_COMMAND;
                        }
                    }

                    else
                    {
                        printf(" ....Failed \n\n\r\r");
                        
                        appData.state = APP_STATE_ERROR;
                    }
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    appData.state = APP_STATE_ERROR;
                }

                break;
            }


            case APP_STATE_SEND_DEV_CONFIG_COMMAND:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    nTxBytes = APP_DevConfigCommandSend(NVM_USER_ROW_START, APP_PROGRAM_PAGE_SIZE);

                    if (nTxBytes != false)
                    {

                        appData.writeStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
                        appData.readStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

                        memset(appData.rdBuffer, 0, sizeof(appData.rdBuffer));

                        SERCOM4_USART_Write(&appData.wrBuffer[0], nTxBytes);
                        SERCOM4_USART_Read(&appData.rdBuffer[0], 2);

                        printf("\n\r Device Configuration Bits Transfer");

                        appData.state = APP_STATE_WAIT_DEV_CONFIG_COMMAND_TRANSFER_COMPLETE;
                    }

                    else
                    {
                        appData.state = APP_STATE_ERROR;
                    }

                    break;
                }
            }


            case APP_STATE_WAIT_DEV_CONFIG_COMMAND_TRANSFER_COMPLETE:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    if(appData.rdBuffer[1] == BL_RESP_FLASH_OK)
                    {
                        printf(" ....Success \n\r");

                        appData.state = APP_STATE_SEND_VERIFY_COMMAND;
                    }

                    else
                    {
                        printf(" ....Failed \n\n\r\r");
                        
                        appData.state = APP_STATE_ERROR;
                    }
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    appData.state = APP_STATE_ERROR;
                }
                break;
            }


            case APP_STATE_SEND_VERIFY_COMMAND:
            {
                nTxBytes = APP_VerifyCommandSend();

                if (nTxBytes != false)
                {
                    appData.writeStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
                    appData.readStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

                    memset(appData.rdBuffer, 0, sizeof(appData.rdBuffer));

                    SERCOM4_USART_Write(&appData.wrBuffer[0], nTxBytes);
                    SERCOM4_USART_Read(&appData.rdBuffer[0], 1);

                    printf("\n\r Verification");

                    appData.state = APP_STATE_WAIT_VERIFY_COMMAND_TRANSFER_COMPLETE;
                }

                else
                {
                    appData.state = APP_STATE_ERROR;
                }

                break;
            }


            case APP_STATE_WAIT_VERIFY_COMMAND_TRANSFER_COMPLETE:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    printf(" ....Success \n\r");

                    appData.state = APP_STATE_SEND_RESET_COMMAND;
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    printf(" ....Failed \n\n\r\r");
                    
                    appData.state = APP_STATE_ERROR;
                }

                break;
            }

            case APP_STATE_SEND_RESET_COMMAND:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    nTxBytes = APP_ResetCommandSend();

                    appData.writeStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
                    appData.readStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

                    memset(appData.rdBuffer, 0, sizeof(appData.rdBuffer));

                    SERCOM4_USART_Write(&appData.wrBuffer[0], nTxBytes);
                    SERCOM4_USART_Read(&appData.rdBuffer[0], 1);

                    printf("\n\r Reboot");

                    appData.state = APP_STATE_WAIT_RESET_COMMAND_TRANSFER_COMPLETE;
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    appData.state = APP_STATE_ERROR;
                }

                break;
            }

            case APP_STATE_WAIT_RESET_COMMAND_TRANSFER_COMPLETE:
            {
                if ((appData.writeStatus == APP_TRANSFER_STATUS_SUCCESS) &&
                    (appData.readStatus == APP_TRANSFER_STATUS_SUCCESS))
                {
                    printf(" ....Success \n\r");

                    atcab_release();

                    appData.state = APP_STATE_SUCCESSFUL;
                }

                else if ((appData.writeStatus == APP_TRANSFER_STATUS_ERROR) ||
                    (appData.readStatus == APP_TRANSFER_STATUS_ERROR))
                {
                    printf(" ....Failed \n\n\r\r");
                    
                    appData.state = APP_STATE_ERROR;
                }
                break;
            }


            case APP_STATE_SUCCESSFUL:
            {  
                LED0_Off();
                
                printf("\n\r Secure Firmware Upgrade is completed \n\r");
                
                appData.state = APP_STATE_IDLE;
                break;
            }

            case APP_STATE_ERROR:
            {
                appData.state = APP_STATE_IDLE;
                break;
            }

            case APP_STATE_IDLE:
            {
                break;
            }

            default:
                break;
        }
    }

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/


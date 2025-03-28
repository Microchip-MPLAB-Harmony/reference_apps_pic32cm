/*******************************************************************************
  UART Bootloader Source File

  File Name:
    bootloader_uart.c

  Summary:
    This file contains source code necessary to execute UART bootloader.

  Description:
    This file contains source code necessary to execute UART bootloader.
    It implements bootloader protocol which uses UART peripheral to download
    application firmware into internal flash from HOST-PC.
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
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h"
#include "bootloader_common.h"
#include <device.h>
#include "atca_basic.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#define GUARD_OFFSET            0
#define CMD_OFFSET              2
#define ADDR_OFFSET             0
#define SIZE_OFFSET             1
#define DATA_OFFSET             1
#define TAG_OFFSET              64
#define SIGN_OFFSET             0
#define PUB_KEY_OFFSET          16

#define CMD_SIZE                1
#define GUARD_SIZE              4
#define SIZE_SIZE               4
#define OFFSET_SIZE             4
#define TAG_SIZE                16
#define HEADER_SIZE             (GUARD_SIZE + SIZE_SIZE + CMD_SIZE)
#define DATA_SIZE               ERASE_BLOCK_SIZE

#define WORDS(x)                ((int)((x) / sizeof(uint32_t)))

#define OFFSET_ALIGN_MASK       (~ERASE_BLOCK_SIZE + 1)
#define SIZE_ALIGN_MASK         (~PAGE_SIZE + 1)

#define BTL_GUARD               (0x5048434DUL)

// Bootloader Commands
enum
{
    BL_CMD_UNLOCK       = 0xa0,
    BL_CMD_DATA         = 0xa1,
    BL_CMD_VERIFY       = 0xa2,
    BL_CMD_RESET        = 0xa3,
    BL_FLASH_STATUS     = 0xa4,
    BL_CMD_DEVCFG_DATA  = 0xa5,
    BL_CMD_READ_VERSION = 0xa6,
};

// Bootloader Response Codes
enum
{
    BL_RESP_OK          = 0x50,
    BL_RESP_ERROR       = 0x51,
    BL_RESP_INVALID     = 0x52,
    BL_RESP_SIGN_OK     = 0x53,
    BL_RESP_SIGN_FAIL   = 0x54,
    BL_RESP_FLASH_OK    = 0x55,
    BL_RESP_FLASH_FAIL  = 0x56,
};

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

// Bootloader Guard Bytes
static uint8_t btl_guard[4] = {0x4D, 0x43, 0x48, 0x50};

static uint32_t input_buffer[WORDS(OFFSET_SIZE + DATA_SIZE + TAG_SIZE)];

static uint32_t flash_data[WORDS(DATA_SIZE)];
static uint32_t flash_addr          = 0;

static uint32_t unlock_begin        = 0;
static uint32_t unlock_end          = 0;
static uint32_t data_size           = 0;

static uint8_t  input_command       = 0;

static bool     packet_received     = false;
static bool     flash_data_ready    = false;
static bool     cipher_data_ready   = false;

static bool     uartBLActive        = false;

typedef bool (*FLASH_ERASE_FPTR)(uint32_t address);

typedef bool (*FLASH_WRITE_FPTR)(uint32_t* data, const uint32_t address);

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

// AES Decryption Parameters

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

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Local Functions
// *****************************************************************************
// *****************************************************************************


/* Function to receive application firmware via UART/USART */
static void input_task(void)
{
    static uint32_t ptr             = 0;
    static uint32_t size            = 0;
    static bool     header_received = false;
    uint8_t         *byte_buf       = (uint8_t *)&input_buffer[0];
    uint8_t         input_data      = 0;

    if (packet_received == true)
    {
        return;
    }

    if (SERCOM4_USART_ReceiverIsReady() == false)
    {
        return;
    }

    input_data = SERCOM4_USART_ReadByte();

    /* Check if 100 ms have elapsed */
    if (SYSTICK_TimerPeriodHasExpired())
    {
        header_received = false;
        ptr = 0;
    }

    if (header_received == false)
    {
        byte_buf[ptr] = input_data;
        ptr++;

        // Check for each guard byte and discard if mismatch
        if (ptr <= GUARD_SIZE)
        {
            if (input_data != btl_guard[ptr-1])
            {
                ptr = 0;
            }
        }
        else if (ptr == HEADER_SIZE)
        {
            if (input_buffer[GUARD_OFFSET] != BTL_GUARD)
            {
                SERCOM4_USART_WriteByte(BL_RESP_ERROR);
            }
            else
            {
                size            = input_buffer[SIZE_OFFSET];
                input_command   = (uint8_t)input_buffer[CMD_OFFSET];
                header_received = true;
                uartBLActive    = true;
            }

            ptr = 0;
        }
        else
        {
            /* Nothing to do */
        }

    }
    else if (header_received == true)
    {
        if (ptr < size)
        {
            byte_buf[ptr++] = input_data;
        }

        if (ptr == size)
        {
            data_size = size;
            ptr = 0;
            size = 0;
            packet_received = true;
            header_received = false;
        }
    }
    else
    {
        /* Nothing to do */
    }

    SYSTICK_TimerRestart();
}

/* Function to process the received command */
static void command_task(void)
{
    if (BL_CMD_UNLOCK == input_command)
    {
        uint32_t begin  = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        uint32_t end    = begin + (input_buffer[SIZE_OFFSET] & SIZE_ALIGN_MASK);

        if (end > begin && end <= (FLASH_START + FLASH_LENGTH))
        {
            unlock_begin = begin;
            unlock_end = end;
            SERCOM4_USART_WriteByte(BL_RESP_OK);
        }
        else
        {
            unlock_begin = 0;
            unlock_end = 0;
            SERCOM4_USART_WriteByte(BL_RESP_ERROR);
        }
    }

    else if ((BL_CMD_DATA == input_command) || (BL_CMD_DEVCFG_DATA == input_command))
    {
        flash_addr = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        if (((BL_CMD_DATA == input_command) && (unlock_begin <= flash_addr && flash_addr < unlock_end))
            || ((BL_CMD_DEVCFG_DATA == input_command) && ((flash_addr >= NVMCTRL_USERROW_START_ADDRESS) && (flash_addr < (NVMCTRL_USERROW_START_ADDRESS + NVMCTRL_USERROW_SIZE))))
            || ((BL_CMD_DEVCFG_DATA == input_command) && ((flash_addr >= NVMCTRL_BOCORROW_START_ADDRESS) && (flash_addr < (NVMCTRL_BOCORROW_START_ADDRESS + NVMCTRL_BOCORROW_SIZE))))
           )
        {
            memcpy(ciphertext, (uint8_t *)&input_buffer[DATA_OFFSET], sizeof(ciphertext));
            memcpy(ciphertag, (uint8_t *)&input_buffer[DATA_OFFSET + TAG_OFFSET], sizeof(ciphertag));

            cipher_data_ready = true;

            SERCOM4_USART_WriteByte(BL_RESP_OK);
        }
        else
        {
            SERCOM4_USART_WriteByte(BL_RESP_ERROR);
        }
    }

    else if (BL_CMD_VERIFY == input_command)
    {
        uint32_t buf_offset = 0;
        uint32_t addr_idx = 0; 
        uint32_t end = unlock_end - unlock_begin;

        bool is_verified = false;

        uint8_t *ptr = (uint8_t *) unlock_begin;

        memcpy(signature, (uint8_t *)&input_buffer[SIGN_OFFSET], sizeof(signature));
        memcpy(public_key, (uint8_t *)&input_buffer[PUB_KEY_OFFSET], sizeof(public_key));

        /* Start sha256 */
        if (atcab_sha_start() != ATCA_SUCCESS)
        {
            SERCOM4_USART_WriteByte(BL_RESP_SIGN_FAIL);
        }

        do
        {
            if(atcab_sha_update(ptr + buf_offset) != ATCA_SUCCESS)
            {
                SERCOM4_USART_WriteByte(BL_RESP_SIGN_FAIL);
                break;
            }

            else
            {
                buf_offset += SHA_DATA_MAX;
            }

        }while (buf_offset < end);

        if(atcab_sha_end(digest, 0, NULL) != ATCA_SUCCESS)
        {
            SERCOM4_USART_WriteByte(BL_RESP_SIGN_FAIL);
        }

        /* Verify signature */
        if ( atcab_verify_extern(digest, signature, public_key, &is_verified) != ATCA_SUCCESS)
        {
            SERCOM4_USART_WriteByte(BL_RESP_SIGN_FAIL);            
        }
        
        if (is_verified == false)
        {
            SERCOM4_USART_WriteByte(BL_RESP_SIGN_FAIL);
            
            addr_idx = unlock_begin;
            
            // Erase all the received data
            do
            {
                if(addr_idx % NVMCTRL_FLASH_ROWSIZE == 0)
                {
                    /* Erase the row */
                    NVMCTRL_RowErase(addr_idx);
                    while(NVMCTRL_IsBusy());
                }

                addr_idx += NVMCTRL_FLASH_PAGESIZE;
                
            }while (addr_idx < unlock_end);  
        }

        SERCOM4_USART_WriteByte(BL_RESP_SIGN_OK);
    }

    else if (BL_CMD_RESET == input_command)
    {
        SERCOM4_USART_WriteByte(BL_RESP_OK);

        while(SERCOM4_USART_TransmitComplete() == false)
        {
            /* Nothing to do */
        }

        atcab_release();

        bootloader_TriggerReset();
    }
    else
    {
        SERCOM4_USART_WriteByte(BL_RESP_INVALID);
    }

    packet_received = false;
}

static void decrpytion_task(void)
{
    uint32_t buf_offset = 0;

    bool is_verified = false;

    /* Initialize the AES GCM Mode to decrypt cipher tag*/
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv))
            != ATCA_SUCCESS)
    {
        cipher_data_ready = false;
        flash_data_ready = false;

        SERCOM4_USART_WriteByte(BL_RESP_ERROR);
    }

    if(atcab_aes_gcm_decrypt_update(&ctx, ciphertag, 16, tag) != ATCA_SUCCESS)
    {
        cipher_data_ready = false;
        flash_data_ready = false;

        SERCOM4_USART_WriteByte(BL_RESP_ERROR);
    }

    /* Initialize the AES GCM Mode to decrypt cipher text*/
    if (atcab_aes_gcm_init(&ctx, key_id, key_block, iv, sizeof(iv))
            != ATCA_SUCCESS)
    {
        cipher_data_ready = false;
        flash_data_ready = false;

        SERCOM4_USART_WriteByte(BL_RESP_ERROR);
    }

    do
    {
        if(atcab_aes_gcm_decrypt_update(&ctx, &ciphertext[buf_offset], 16,
                &plaintext[buf_offset]) != ATCA_SUCCESS)
        {
            cipher_data_ready = false;
            flash_data_ready = false;

            SERCOM4_USART_WriteByte(BL_RESP_ERROR);
            break;
        }

        else
        {
            buf_offset += 16;
        }

    }while (buf_offset < DATA_SIZE);

    if(atcab_aes_gcm_decrypt_finish(&ctx, tag, sizeof(tag), &is_verified)
            != ATCA_SUCCESS)
    {
        cipher_data_ready = false;
        flash_data_ready = false;

        SERCOM4_USART_WriteByte(BL_RESP_ERROR);
    }

    else
    {
        memcpy(flash_data, (uint32_t *)&plaintext, DATA_SIZE);

        flash_data_ready = true;
        cipher_data_ready = false;
    }
}

/* Function to program received application firmware data into internal flash */
static void flash_task(void)
{
    uint32_t addr           = flash_addr;
    uint32_t bytes_written  = 0;
    uint32_t write_idx      = 0;

    // data_size = Actual data bytes to write + Address 4 Bytes
    uint32_t bytes_to_write = (data_size - 16 - 4);

    FLASH_ERASE_FPTR flash_erase_fptr = (FLASH_ERASE_FPTR)NVMCTRL_RowErase;
    FLASH_WRITE_FPTR flash_write_fptr = (FLASH_WRITE_FPTR)NVMCTRL_PageWrite;

    if ((addr >= unlock_begin && addr < unlock_end))
    {
        if (addr < APP_START_ADDRESS)
        {
            NVMCTRL_SecureRegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_BOOTLOADER);
            
            while(NVMCTRL_IsBusy() == true);
        }
        
        else
        {
            NVMCTRL_SecureRegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_APPLICATION);

            while(NVMCTRL_IsBusy() == true);
        }
    }

    // Check if the address falls in Device Configuration Space
    if (!(addr >= unlock_begin && addr < unlock_end))
    {
        if ((addr >= NVMCTRL_USERROW_START_ADDRESS) && (addr < (NVMCTRL_USERROW_START_ADDRESS + NVMCTRL_USERROW_SIZE)))
        {
            flash_erase_fptr = (FLASH_ERASE_FPTR)NVMCTRL_USER_ROW_RowErase;
            flash_write_fptr = (FLASH_WRITE_FPTR)NVMCTRL_USER_ROW_PageWrite;
        }
        else if ((addr >= NVMCTRL_BOCORROW_START_ADDRESS) && (addr < (NVMCTRL_BOCORROW_START_ADDRESS + NVMCTRL_BOCORROW_SIZE)))
        {
            flash_erase_fptr = (FLASH_ERASE_FPTR)NVMCTRL_BOCOR_ROW_RowErase;
            flash_write_fptr = (FLASH_WRITE_FPTR)NVMCTRL_BOCOR_ROW_PageWrite;
        }
        else
        {
            /* nothing to do */
        }
    }
    /* Erase the Current sector */
    (void) flash_erase_fptr(addr);

    /* Receive Next Bytes while waiting for memory to be ready */
    while(NVMCTRL_IsBusy() == true);

    for (bytes_written = 0; bytes_written < bytes_to_write; bytes_written += PAGE_SIZE)
    {
        (void) flash_write_fptr(&flash_data[write_idx], addr);

        /* Receive Next Bytes while waiting for memory to be ready */
        while(NVMCTRL_IsBusy() == true);

        addr += PAGE_SIZE;
        write_idx += WORDS(PAGE_SIZE);
    }

    SERCOM4_USART_WriteByte(BL_RESP_FLASH_OK);

    flash_data_ready = false;
}

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Global Functions
// *****************************************************************************
// *****************************************************************************

bool APP_ECC_Init (void)
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

void bootloader_UART_Tasks(void)
{
    do
    {
        input_task();

        if(cipher_data_ready)
        {
           decrpytion_task();
        }
        else if (flash_data_ready)
        {
            flash_task();
        }
        else if (packet_received)
        {
            command_task();
        }
        else
        {
            /* Nothing to do */
        }
    } while (uartBLActive);
}

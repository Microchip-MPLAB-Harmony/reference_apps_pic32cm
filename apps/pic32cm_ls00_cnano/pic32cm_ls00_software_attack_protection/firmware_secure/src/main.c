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
#include "non_secure_app_image_pic32cm_ls00_cnano.h"

#define APP_IMAGE_SIZE          sizeof(image_pattern)
#define APP_IMAGE_END_ADDR      (APP_IMAGE_START_ADDR + APP_IMAGE_SIZE)
#define NON_SECURE_APP_ADDR     (TZ_START_NS)

static uint8_t *appStart = (uint8_t *)NON_SECURE_APP_ADDR;
static uint8_t *dataStart = (uint8_t *)NVMCTRL_DATAFLASH_START_ADDRESS;

/* typedef for non-secure callback functions */
typedef void (*funcptr_void) (void) __attribute__((cmse_nonsecure_call));

uint8_t firmware_digest_0[64];
uint8_t firmware_digest_1[32];

typedef struct 
{
    /* Digest result of SHA256 */
    uint32_t digest[8];
    /* Length of the message */
    uint64_t length;
    /* Holds the size of the remaining part of data */
    uint32_t remain_size;
    /* Buffer of remaining part of data (512 bits data block) */
    uint8_t remain_ram[64];
    /* RAM buffer of 256 bytes used by crya_sha_process */
    uint32_t process_buf[64];
    
} SHA256_CTX;


SHA256_CTX sha256_ctx;


typedef void (*crya_sha256_init_t) (SHA256_CTX *context);
typedef void (*crya_sha256_update_t) (SHA256_CTX *context, const unsigned char *data, size_t length);
typedef void (*crya_sha256_final_t) (SHA256_CTX *context, unsigned char output[32]);

#define crya_sha256_init ((crya_sha256_init_t) (0x02006810 | 0x1))
#define crya_sha256_update ((crya_sha256_update_t) (0x02006814 | 0x1))
#define crya_sha256_final ((crya_sha256_final_t) (0x02006818 | 0x1))


static void sha256_hash(SHA256_CTX *ctx, const uint8_t *message, uint32_t length, 
        unsigned char digest[32])
{
    uint8_t dataBuf[64];
    
    uint32_t bufIdx = 0;
    
    crya_sha256_init(ctx);
    
    do
    {     
        memcpy(dataBuf, &message[bufIdx], 64);
        
        crya_sha256_update(ctx, dataBuf, sizeof(dataBuf));
        bufIdx += 64;
        
    }while (bufIdx < APP_IMAGE_SIZE);

    crya_sha256_final(ctx, digest);
}


static void flash_write(uint32_t addr, uint8_t *buf, uint32_t size)
{      
    uint32_t end_addr = addr + size;
    
    if((addr & NVMCTRL_DATAFLASH_START_ADDRESS) == NVMCTRL_DATAFLASH_START_ADDRESS)
    {
        /* Unlock the Secure Data Flash region */
        NVMCTRL_RegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_DATA);
        while(NVMCTRL_IsBusy());
    }
    
    else
    {
        /* Unlock the Non-Secure Flash region */
        NVMCTRL_RegionUnlock(NVMCTRL_MEMORY_REGION_APPLICATION);
        while(NVMCTRL_IsBusy());
    }
    

    do
    {
        if(addr % NVMCTRL_FLASH_ROWSIZE == 0)
        {
            /* Erase the row */
            NVMCTRL_RowErase(addr);
            while(NVMCTRL_IsBusy());
        }

        /* Program 64 byte page */
        NVMCTRL_PageWrite((uint32_t *)(buf), addr);
        while(NVMCTRL_IsBusy());
        
        addr += NVMCTRL_FLASH_PAGESIZE;
        buf += NVMCTRL_FLASH_PAGESIZE;
        
    }while (addr < end_addr);     
}


static bool non_secure_app_verify(void)
{    
    sha256_hash(&sha256_ctx, appStart, APP_IMAGE_SIZE, firmware_digest_1);

    if(memcmp(dataStart, firmware_digest_1, 32) != 0) 
    {        
        printf("Firmware is Corrupted....!");
        printf("\n\r\n\r");
        
        printf("Firmware Digest after tamper detection:");
        printf("\n\r\n\r");

        for(int i=0; i<32; i++)
        {
            printf("0x%X ", dataStart[i]);

            if((i%8 == 0) && (i != 0))
            {
                printf("\n\r");
            }
        }
        
        flash_write(TZ_START_NS, (uint8_t *)&image_pattern, sizeof(image_pattern));
        
        sha256_hash(&sha256_ctx, image_pattern, APP_IMAGE_SIZE, firmware_digest_1);
        
        printf("\n\r\n\r");
        printf("Restored Firmware Digest:");      
        printf("\n\r\n\r");

        for(int i=0; i<32; i++)
        {
            printf("0x%X ", firmware_digest_1[i]);

            if((i%8 == 0) && (i != 0))
            {
                printf("\n\r");
            }
        }
        
        

        printf("\n\r\n\r");
        printf("Genuine Firmware is restored");

    }

    else
    {        
        return false;
    }
    
    return true;
}
 

void timeout_handler(RTC_TIMER32_INT_MASK intCause, uintptr_t context)
{
    if(RTC_TIMER32_INT_MASK_CMP0  == (RTC_TIMER32_INT_MASK_CMP0 & intCause ))
    {
        if(non_secure_app_verify() == true)
        {
            SYSTICK_DelayMs(2000);

            NVIC_SystemReset();
        }
    }

    if (RTC_TIMER32_INT_MASK_TAMPER == (intCause & RTC_TIMER32_INT_MASK_TAMPER))
    {        
        RTC_REGS->MODE2.RTC_TAMPID = RTC_TAMPID_Msk;
         
        printf("Software Attack Detected");
        printf("\n\r\n\r");
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    uint32_t msp_ns = *((uint32_t *)(TZ_START_NS));
    volatile funcptr_void NonSecure_ResetHandler;

    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    sha256_hash(&sha256_ctx, image_pattern, APP_IMAGE_SIZE, firmware_digest_0);
    
    flash_write(NVMCTRL_DATAFLASH_START_ADDRESS, firmware_digest_0, sizeof(firmware_digest_0));
        
    SYSTICK_TimerStart();
            
    RTC_Timer32CallbackRegister(timeout_handler,0);
    RTC_Timer32Start();
    
    printf("\n\r---------------------------------------------------------");
    printf("\n\r           Software Attack Protection Demo               ");
    printf("\n\r---------------------------------------------------------\n\r");
      
    if(non_secure_app_verify() != true)
    {
        printf("\n\rFirmware is Genuine");
        printf("\n\r\n\r");
    }

    if (msp_ns != 0xFFFFFFFF)
    {
        /* Set non-secure main stack (MSP_NS) */
        __TZ_set_MSP_NS(msp_ns);

        /* Get non-secure reset handler */
        NonSecure_ResetHandler = (funcptr_void)(*((uint32_t *)((TZ_START_NS) + 4U)));

        /* Start non-secure state software application */
        NonSecure_ResetHandler();
    }

    while ( true )
    {
    }

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

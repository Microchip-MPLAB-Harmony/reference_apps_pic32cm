/*******************************************************************************
  App NVM Common Source File

  File Name:
    app_nvm.c

  Summary:
    This file contains source code necessary to execute UART bootloader.

  Description:
    This file contains source code necessary to execute UART bootloader.
    It implements bootloader protocol which uses UART peripheral to download
    application firmware into internal flash from HOST-PC.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "app_nvm.h"
#include "definitions.h"
#include <device.h>

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
/* Bootloader Major and Minor version sent for a Read Version command (MAJOR.MINOR)*/
#define BTL_MAJOR_VERSION       3U
#define BTL_MINOR_VERSION       7U

#define GUARD_OFFSET            0
#define CMD_OFFSET              2
#define ADDR_OFFSET             0
#define SIZE_OFFSET             1
#define DATA_OFFSET             1U
#define CRC_OFFSET              0

#define CMD_SIZE                1U
#define GUARD_SIZE              4U
#define SIZE_SIZE               4U
#define OFFSET_SIZE             4
#define CRC_SIZE                4
#define HEADER_SIZE             (GUARD_SIZE + SIZE_SIZE + CMD_SIZE)
#define DATA_SIZE               NVMCTRL_FLASH_ROWSIZE
#define PAGE_SIZE               NVMCTRL_FLASH_PAGESIZE

#define WORDS(x)                ((uint32_t)((x) / sizeof(uint32_t)))

#define OFFSET_ALIGN_MASK       (~NVMCTRL_FLASH_ROWSIZE + 1U)
#define SIZE_ALIGN_MASK         (~NVMCTRL_FLASH_PAGESIZE + 1U)

#define BTL_GUARD               (0x5048434DUL)

#define BL_CMD_UNLOCK           0xa0U
#define BL_CMD_DATA             0xa1U
#define BL_CMD_VERIFY           0xa2U
#define BL_CMD_RESET            0xa3U
#define BL_CMD_DEVCFG_DATA      0xa5U
#define BL_CMD_READ_VERSION     0xa6U

#define FIRMWARE_START          (0x9000UL)  
#define FIRMWARE_LENGTH         (0x77000UL) // 476 KB of Client Firmware

#define FIRMWARE_DATA_0_START   (NVMCTRL_DATAFLASH_START_ADDRESS)  // First Row for Configuration Bits Details
#define FIRMWARE_DATA_0_LENGTH  NVMCTRL_DATAFLASH_ROWSIZE

#define FIRMWARE_DATA_1_START   (NVMCTRL_DATAFLASH_START_ADDRESS + NVMCTRL_DATAFLASH_ROWSIZE)  // Second Row -> Client Firmware Details
#define FIRMWARE_DATA_1_LENGTH  NVMCTRL_DATAFLASH_PAGESIZE


enum
{
    BL_RESP_OK          = 0x50,
    BL_RESP_ERROR       = 0x51,
    BL_RESP_INVALID     = 0x52,
    BL_RESP_CRC_OK      = 0x53,
    BL_RESP_CRC_FAIL    = 0x54,
};

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

static uint8_t btl_guard[4] = {0x4D, 0x43, 0x48, 0x50};

static uint32_t input_buffer[WORDS(OFFSET_SIZE + DATA_SIZE)];
static uint32_t flash_data[WORDS(DATA_SIZE)];

static uint32_t flash_addr          = 0;
static uint32_t unlock_begin        = 0;
static uint32_t unlock_end          = 0;
static uint32_t data_size           = 0;

static uint8_t  input_command       = 0;

static bool     packet_received     = false;
static bool     flash_data_ready    = false;
static bool     uartBLActive        = false;
static bool     updateDone          = false;

typedef bool (*FLASH_ERASE_FPTR)(uint32_t address);

typedef bool (*FLASH_WRITE_FPTR)(uint32_t* data, const uint32_t address);


// *****************************************************************************
// *****************************************************************************
// Section: APP NVM Local Functions
// *****************************************************************************
// 

// Function to Generate CRC by reading the firmware programmed into internal flash
uint32_t CRCGenerate(uint32_t start_addr, uint32_t size)
{
    uint32_t   i, j, value;
    uint32_t   crc_tab[256];
    uint32_t   crc = 0xffffffffU;
    uint8_t    data;

    for (i = 0; i < 256U; i++)
    {
        value = i;

        for (j = 0; j < 8U; j++)
        {
            if ((value & 1U) != 0U)
            {
                value = (value >> 1) ^ 0xEDB88320U;
            }

            else
            {
                value >>= 1;
            }
        }

        crc_tab[i] = value;
    }

    for (i = 0; i < size; i++)
    {
        data = *(uint8_t *)(start_addr + i);

        crc = crc_tab[(crc ^ data) & 0xffU] ^ (crc >> 8);
    }

    return crc;
}


uint16_t __WEAK bootloader_GetVersion( void )
{
    /* Function can be override with custom implementation */
    uint16_t btlVersion = (((BTL_MAJOR_VERSION & (uint16_t)0xFFU) << 8) | (BTL_MINOR_VERSION & (uint16_t)0xFFU));

    return btlVersion;
}

/* Function to program received application firmware size into internal data flash */
static void data_flash_task(void)
{   
    NVMCTRL_SecureRegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_DATA);
    while(NVMCTRL_IsBusy() == true);
        
    NVMCTRL_RowErase(FIRMWARE_DATA_1_START);
    while(NVMCTRL_IsBusy() == true);   
            
    for (int bytes_written = 0; bytes_written < FIRMWARE_DATA_1_LENGTH/4; bytes_written++)
    {
        // Application Start Address
        if(bytes_written == 0)
        {
            flash_data[bytes_written] = unlock_begin;
        }
        
        // Application Size
        else if(bytes_written == 1)
        {
            flash_data[bytes_written] = unlock_end - unlock_begin;
        }
        
        else
        {
            flash_data[bytes_written] = 0xFFFFFFFF;
        }
    
    }

    NVMCTRL_PageWrite(flash_data, FIRMWARE_DATA_1_START);        
    while(NVMCTRL_IsBusy() == true);
   
}



/* Function to receive application firmware via UART */
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

    if (SERCOM3_USART_ReceiverIsReady() == false)
    {
        return;
    }

    input_data = (uint8_t)SERCOM3_USART_ReadByte();

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
            if (input_data != btl_guard[ptr-1U])
            {
                ptr = 0;
            }
        }

        else if (ptr == HEADER_SIZE)
        {
            if (input_buffer[GUARD_OFFSET] != BTL_GUARD)
            {
                SERCOM3_USART_WriteByte(BL_RESP_ERROR);
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
    uint32_t i;

    if (BL_CMD_UNLOCK == input_command)
    {
        uint32_t begin  = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        uint32_t end    = begin + (input_buffer[SIZE_OFFSET] & SIZE_ALIGN_MASK);

        if (end > begin && end <= (FIRMWARE_START + FIRMWARE_LENGTH))
        {
            unlock_begin = begin;
            unlock_end = end;

            SERCOM3_USART_WriteByte(BL_RESP_OK);
            
            updateDone = false;
        }

        else
        {
            unlock_begin = 0;
            unlock_end = 0;

            SERCOM3_USART_WriteByte(BL_RESP_ERROR);
        }
    }
   
    else if ((BL_CMD_DATA == input_command) || (BL_CMD_DEVCFG_DATA == input_command))
    {
        flash_addr = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        if (((BL_CMD_DATA == input_command) && (unlock_begin <= flash_addr && flash_addr < unlock_end))
            || ((BL_CMD_DEVCFG_DATA == input_command) && ((flash_addr >= FIRMWARE_DATA_0_START) && (flash_addr < (FIRMWARE_DATA_0_START + FIRMWARE_DATA_0_LENGTH)))))
        {
            for (i = 0; i < WORDS(DATA_SIZE); i++)
            {
                flash_data[i] = input_buffer[i + DATA_OFFSET];
            }

            flash_data_ready = true;
        }

        else
        {
            SERCOM3_USART_WriteByte(BL_RESP_ERROR);
        }
    }
    else if (BL_CMD_READ_VERSION == input_command)
    {
        SERCOM3_USART_WriteByte(BL_RESP_OK);

        uint16_t btlVersion = bootloader_GetVersion();
        uint16_t btlVer = ((btlVersion >> 8U) & 0xFFU);

        SERCOM3_USART_WriteByte((int)btlVer);
        
        btlVer = (btlVersion & 0xFFU);
        
        SERCOM3_USART_WriteByte((int)btlVer);
    }

    else if (BL_CMD_VERIFY == input_command)
    {
        uint32_t crc        = input_buffer[CRC_OFFSET];
        uint32_t crc_gen    = 0;

        crc_gen = CRCGenerate(unlock_begin, (unlock_end - unlock_begin));

        if (crc == crc_gen)
        {
            SERCOM3_USART_WriteByte(BL_RESP_CRC_OK);
        }

        else
        {
            SERCOM3_USART_WriteByte(BL_RESP_CRC_FAIL);
        }
    }

    else if (BL_CMD_RESET == input_command)
    {
        // Update the application start address and size in the data flash
        data_flash_task();
        
        SERCOM3_USART_WriteByte(BL_RESP_OK);

        while(SERCOM3_USART_TransmitComplete() == false)
        {
            /* Nothing to do */
        }          

        uartBLActive = false;
        updateDone = true;
    }

    else
    {
        SERCOM3_USART_WriteByte(BL_RESP_INVALID);
        
        updateDone = false;
    }

    packet_received = false;
}
/* Function to program received application firmware data into internal flash */
static void flash_task(void)
{
    uint32_t addr           = flash_addr;
    uint32_t bytes_written  = 0;
    uint32_t write_idx      = 0;

    // data_size = Actual data bytes to write + Address 4 Bytes
    uint32_t bytes_to_write = (data_size - 4U);

    FLASH_ERASE_FPTR flash_erase_fptr = (FLASH_ERASE_FPTR)NVMCTRL_RowErase;
    FLASH_WRITE_FPTR flash_write_fptr = (FLASH_WRITE_FPTR)NVMCTRL_PageWrite;

    if ((addr >= unlock_begin && addr < unlock_end))
    {
        if ((addr >= FIRMWARE_START) && (addr < (FIRMWARE_START + FIRMWARE_LENGTH)))
        {
            NVMCTRL_SecureRegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_APPLICATION);
            while(NVMCTRL_IsBusy() == true);
        }
    }
    
    // Check if the address falls in Device Configuration Space
    if (!(addr >= unlock_begin && addr < unlock_end))
    {
        if ((addr >= FIRMWARE_DATA_0_START) && (addr < (FIRMWARE_DATA_0_START + FIRMWARE_DATA_0_LENGTH)))
        {
            NVMCTRL_SecureRegionUnlock(NVMCTRL_SECURE_MEMORY_REGION_DATA);
            while(NVMCTRL_IsBusy() == true);
        }
    }
        
    /* Erase the Current sector */
    (void) flash_erase_fptr(addr);
    while(NVMCTRL_IsBusy() == true);

    for (bytes_written = 0; bytes_written < bytes_to_write; bytes_written += PAGE_SIZE)
    {
        (void) flash_write_fptr(&flash_data[write_idx], addr);
        while(NVMCTRL_IsBusy() == true);

        addr += PAGE_SIZE;
        write_idx += WORDS(PAGE_SIZE);
    }

    flash_data_ready = false;
    
    SERCOM3_USART_WriteByte(BL_RESP_OK);
}

// *****************************************************************************
// *****************************************************************************
// Section: APP NVM Global Functions
// *****************************************************************************
// *****************************************************************************

bool APP_NVM_UART_Tasks(void)
{
    do
    {
        input_task();

        if (flash_data_ready)
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
    
    if (updateDone == true)
    {
        updateDone = false;
        
        return true;
    }
    
    return false;
}

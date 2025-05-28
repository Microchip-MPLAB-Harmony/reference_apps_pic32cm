/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
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
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdint.h>
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>
#include <time.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

// EEPROM Commands
#define EEPROM_CMD_WREN                     0x06  // Write Enable
#define EEPROM_CMD_WRITE                    0x02  // Write Data
#define EEPROM_CMD_WRBP                     0x08  // Write status Register
#define EEPROM_CMD_READ                     0x03  // Read Data

#define EEPROM_DATA_LEN                     32
#define MAX_LOGS                            16
#define SWITCH_STATE_PRESSED                0

/* Global variables */
char EEPROM_DATA[EEPROM_DATA_LEN];
uint8_t  txData[(4 + EEPROM_DATA_LEN)];
uint8_t  rxData[(4 + EEPROM_DATA_LEN)];
volatile bool isTransferDone = false;
volatile bool change_detect = false;
volatile uint8_t logIndex = 0;
struct tm sys_time;

// Check the comparator output state 
void AC_CallBack(uint8_t int_flag, uintptr_t ac_context)
{    
    if(int_flag & AC_STATUSA_STATE0_Msk)
    {
        change_detect  = true;
    }
}

// This function will be called by SPI PLIB when transfer is completed 
void SPIEventHandler(uintptr_t context)
{
    EEPROM_CS_Set();
    isTransferDone = true;
}

// Initializes EEPROM control lines
void EEPROM_Initialize (void)
{
    EEPROM_HOLD_Set();
    EEPROM_CS_Set();
    LED_Clear();
}

/****************************************************************************
  Function:
    EEPROM_Write(uint32_t eepromAddr, const char* data, size_t len)

  Summary:
    Write the observed acoustic data into EEPROM storage

  Remarks:
    Handles write enable, data write, and polling for write completion.
 ***************************************************************************/
void EEPROM_Write(uint32_t eepromAddr, const char* data, size_t len)
{
    // Enable Write Operation
    txData[0] = EEPROM_CMD_WREN;
    EEPROM_CS_Clear();
    isTransferDone = false;
    SERCOM5_SPI_Write(txData, 1);
    while (!isTransferDone);

    // Write command + address + data
    txData[0] = EEPROM_CMD_WRITE;
    txData[1] = (uint8_t)(eepromAddr>>16);    
    txData[2] = (uint8_t)(eepromAddr>>8);
    txData[3] = (uint8_t)(eepromAddr);
    memcpy(&txData[4], data, len);
    EEPROM_CS_Clear();
    isTransferDone = false;
    SERCOM5_SPI_Write(txData, (4 + len));
    while (!isTransferDone);

    // Wait for write to complete 
    do {
        txData[0] = EEPROM_CMD_WRBP;
        EEPROM_CS_Clear();
        isTransferDone = false;
        SERCOM5_SPI_WriteRead(txData, 1, rxData, 2);
        while (!isTransferDone); 
    } while (rxData[1] == 0xFF); //0xFF =  EEPROM is still busy completing the write cycle (write cycle time = 5ms)
}

/****************************************************************************
  Function:
    EEPROM_Read(uint32_t eepromAddr, size_t len)

  Summary:
    Read the stored acoustic data from the EEPROM storage

  Remarks:
   Reads data from EEPROM and prints it with timestamp.
 ***************************************************************************/
void EEPROM_Read(uint32_t eepromAddr, size_t len)
{
    txData[0] = EEPROM_CMD_READ;
    txData[1] = (uint8_t)(eepromAddr >> 16);
    txData[2] = (uint8_t)(eepromAddr >> 8);
    txData[3] = (uint8_t)(eepromAddr);
    
    EEPROM_CS_Clear();
    SERCOM5_SPI_WriteRead(txData, 4, rxData, (4 + len));
    RTC_RTCCTimeGet(&sys_time);
    
    printf("  Date = %02d.%02d.%02d, Time = %02d:%02d:%02d,",
        sys_time.tm_mday,
        (sys_time.tm_mon)+1,
        (sys_time.tm_year)+1900,
        sys_time.tm_hour,
        sys_time.tm_min,
        sys_time.tm_sec);
    
    printf(" %.*s\r\n",(int)len,&rxData[4]);  
}

/****************************************************************************
  Function:
   Data_Log_Callback()

  Summary:
    Write and Read in the EEPROM and the obtained data prints on terminal 

  Remarks:
    prints the observed 16 logs of acoustic Data
 ***************************************************************************/
void Data_Log_Callback()
{
    if (change_detect && logIndex < MAX_LOGS)
    {
        change_detect = false;
        
        memcpy(EEPROM_DATA, "Noise detected", strlen("Noise detected"));

        //Write and read EEPROM
        uint32_t address = logIndex * EEPROM_DATA_LEN;
        EEPROM_Write(address, EEPROM_DATA, EEPROM_DATA_LEN);
        
        logIndex++;
        
        if (logIndex >= MAX_LOGS)
        {
            // Read and print all logs
            for (uint8_t i = 0; i < MAX_LOGS; i++)
            {
                  
                uint32_t readAddr = i * EEPROM_DATA_LEN;
                EEPROM_Read(readAddr, EEPROM_DATA_LEN); 
            }
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL);
    
    printf("\n\n\r----------------------------------------------------------------");
    printf("\n\n\r       PIC32CM GV VL Curiosity Nano + Touch Evaluation Kit      ");
    printf("\n\n\r----------------------------------------------------------------");

    printf("\n\n\r  Hold SWITCH after Reset to Data Log \n\n\r");    
    
    /* Set Time and Date: DD-MM-YYYY 30-04-2025 Wednesday */
    sys_time.tm_hour    = 11;   /* hour [0,23] */
    sys_time.tm_min     = 00;   /* minutes [0,59] */
    sys_time.tm_sec     = 00;   /* seconds [0,61] */
    sys_time.tm_mday    = 31;   /* day of month [1,31] */
    sys_time.tm_mon     = 2;   /* month of year [0,11] */
    sys_time.tm_year    = 125;  /* years since 1900 */
    
    RTC_RTCCTimeSet(&sys_time);
    AC_CallbackRegister(AC_CallBack, 0);
    SERCOM5_SPI_CallbackRegister(SPIEventHandler, (uintptr_t)0);
    EEPROM_Initialize();
    
    while(true)
    {
        if (SW_Get() == SWITCH_STATE_PRESSED)
        {
            Data_Log_Callback();
        }
        PM_StandbyModeEnter(); 
    }
    
    return ( EXIT_FAILURE );
}
/*******************************************************************************
 End of File
*/


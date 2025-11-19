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

/* EEPROM Commands */
#define EEPROM_CMD_WREN                     0x06    // Write Enable
#define EEPROM_CMD_WRITE                    0x02    // Write Data
#define EEPROM_CMD_WRBP                     0x08    // Write status Register
#define EEPROM_CMD_READ                     0x03    // Read Data

#define EEPROM_ADDRESS                      0x00000000
#define EEPROM_DATA_LEN                     64
#define MAX_LOGS                            16
#define SWITCH_STATE_PRESSED                0
#define UART_BUFFER_SIZE                    64

/* Global variables */
char EEPROM_DATA[EEPROM_DATA_LEN];
uint8_t  txData[(4 + EEPROM_DATA_LEN)];
uint8_t  rxData[(4 + EEPROM_DATA_LEN)];
volatile bool isTransferDone = false;
volatile bool change_detect = false;
volatile bool ac_comparison_done = false;
volatile bool prev_touch = false;
volatile bool standbyEntered = false;
volatile uint8_t logCount = 0;
volatile uint8_t logIndex = 0;
volatile uint8_t mode = 0;
struct tm sys_time;

// Checks the output state of the comparator.
void AC_CallBack(uint8_t int_flag, uintptr_t ac_context)
{    
    if(int_flag & AC_STATUSA_STATE0_Msk)
    {
        change_detect  = true;
    }
}

 // Callback function invoked by the SPI PLIB upon transfer completion.
void SPIEventHandler(uintptr_t context)
{
    EEPROM_CS_Set();
    isTransferDone = true;
}

// Initializes the control lines for EEPROM operation.
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
    Writes data to EEPROM storage.

  Description:
    Enables write, writes data to EEPROM via SPI, and waits for completion.
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
    Reads data from EEPROM and prints it to the terminal.

  Description:
    Retrieves data from EEPROM via SPI and displays it with a timestamp on the
    terminal.
 ***************************************************************************/
void EEPROM_Read(uint32_t eepromAddr, size_t len)
{
    txData[0] = EEPROM_CMD_READ;
    txData[1] = (uint8_t)(eepromAddr >> 16);
    txData[2] = (uint8_t)(eepromAddr >> 8);
    txData[3] = (uint8_t)(eepromAddr);
    
    EEPROM_CS_Clear();
    isTransferDone = false;
    SERCOM5_SPI_WriteRead(txData, 4, rxData, (4 + EEPROM_DATA_LEN));
    while (!isTransferDone);
    
    printf("  %s\r\n", (char*)&rxData[4]);  
    
}

/****************************************************************************
  Function:
    Data_Log_Callback()

  Summary:
    Writes data to EEPROM and prints it to the terminal.

  Description:
    Logs acoustic data in EEPROM and displays the stored data on the terminal.
 ***************************************************************************/
void Data_Log_Callback()
{
    if ((logIndex < MAX_LOGS) &&(change_detect))
        
    {
        change_detect = false;

        RTC_RTCCTimeGet(&sys_time);
    
        snprintf(EEPROM_DATA, EEPROM_DATA_LEN, "Date=%02d-%02d-%04d, Time=%02d:%02d:%02d, Noise Detected",
        sys_time.tm_mday,
        sys_time.tm_mon + 1,
        (sys_time.tm_year & 0x3F) + 2000,  // Limits from 2000 to 2063 year 
        sys_time.tm_hour,
        sys_time.tm_min,
        sys_time.tm_sec);
        
        SYSTICK_DelayUs(500); 
        
        //Write and read EEPROM
        uint32_t address = logIndex * EEPROM_DATA_LEN;
        EEPROM_Write(address, EEPROM_DATA, EEPROM_DATA_LEN); 
        EEPROM_Read(address, EEPROM_DATA_LEN);
        logIndex++;
        
        if(logIndex >= MAX_LOGS)
        {
            logIndex = 0; // Resets the log index
        }
    }
}

/****************************************************************************
  Function:
   Touch_Status()

  Summary:
    Detects a touch event to trigger the appropriate mode of operation.

  Description:
    perform the Data log and low power mode operations
 ***************************************************************************/
bool Touch_Status(void)
{
    touch_process();  
    bool touched = (0u != (get_sensor_state(0) & 0x80));

    if (touched && !prev_touch)
    {
        prev_touch = true;
        SYSTICK_DelayMs(150);   
        return true;  
    }
    else if (!touched)
    {
        prev_touch = false;
    }

    return false;  
}

/****************************************************************************
  Function:
    UART_RTC_Read()

  Summary:
    View and set the RTC date and time via the serial terminal.

  Description:
    Displays the current RTC date and time, and echoes back the user?s input 
    for RTC update.
 ***************************************************************************/

void UART_RTC_Read(char *buffer, size_t maxlen)
{
    size_t index = 0;
    char c = 0;

    while (index < maxlen - 1)
    {
        while (!SERCOM2_USART_ReceiverIsReady()); // Wait for data
        SERCOM2_USART_Read(&c, 1);

        // Echo back to terminal
        SERCOM2_USART_Write(&c, 1);

        if (c == '\r' || c == '\n')
        {
            buffer[index] = '\0';
            // Send newline for terminal formatting
            char nl[2] = "\r\n";
            SERCOM2_USART_Write(nl, 2);
            break;
        }
        else
        {
            buffer[index++] = c;
        }
    }
    buffer[index] = '\0';
}

/****************************************************************************
  Function:
    Set_RTC_Date_Time()

  Summary:
    Reads and sets the RTC date and time via the terminal.

  Description:
    Shows the current RTC date and time, prompts for new values, and updates 
    the RTC.
 ***************************************************************************/
void Set_RTC_Date_Time(void)
{
    char input[UART_BUFFER_SIZE];
    int year, month, day, hour, min, sec;

    const char *prompt = "\r\n\nEnter date and time in the format: DD-MM-YYYY HH:MM:SS\r\nExample: 07-07-2025 11:00:00\r\nInput: ";
    SERCOM2_USART_Write((void*)prompt, strlen(prompt));

    UART_RTC_Read(input, sizeof(input)); 

    // Parse input
    if (sscanf(input, "%d-%d-%d %d:%d:%d",&day, &month, &year, &hour, &min, &sec) == 6)
    {    
        if (year < 2000 || year > 2063 ||
            month < 1 || month > 12 ||
            day < 1 || day > 31 ||
            hour < 0 || hour > 23 ||
            min < 0 || min > 59 ||
            sec < 0 || sec > 59)
        {
            const char *err = "\r\n  Invalid date or time value. Please try again.\r\n";
            SERCOM2_USART_Write((void*)err, strlen(err));
            Set_RTC_Date_Time();
            return; // <-- Prevents setting RTC with invalid values
        }
        
        sys_time.tm_mday = day;
        sys_time.tm_mon  = month - 1;
        sys_time.tm_year = year - 2000; // from 2000 to 2063 year 
        sys_time.tm_hour = hour;
        sys_time.tm_min  = min;
        sys_time.tm_sec  = sec;

        RTC_RTCCTimeSet(&sys_time);

        char confirm[80];
        snprintf(confirm, sizeof(confirm), "\r\n  RTC updated to: %02d-%02d-%04d %02d:%02d:%02d\r\n",
                 day, month, year, hour, min, sec);
        SERCOM2_USART_Write((void*)confirm, strlen(confirm));
    }
    else
    {
        const char *err = "\r\n  Invalid format. Please try again.\r\n";
        SERCOM2_USART_Write((void*)err, strlen(err));
        Set_RTC_Date_Time();
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
    
    Set_RTC_Date_Time();
    AC_CallbackRegister(AC_CallBack, 0);
    EEPROM_Initialize();
    SERCOM5_SPI_CallbackRegister(SPIEventHandler, (uintptr_t)0);
    
    printf("\n\n\r  Mode 1: Press the Touch Button to start Data Log \n\n\r");

    while (true)
    {
        SYS_Tasks();

        if (Touch_Status())
        {
            mode++; 
        }

        switch (mode)
        {
            case 1:
                // Enter Acoustic data Logging
                LED_Set();
                Data_Log_Callback();
                break;

            case 2:
                // Enter low power mode
                if(!standbyEntered)
                {
 
                    standbyEntered = true;
                    printf("\n\n\r  Mode 2: Entering Low Power Standby Mode...\n");
                    
                    SYSTICK_DelayUs(1000); 
                    SYSTICK_TimerStop();  
                    
                    LED_Clear();
                    
                    PM_StandbyModeEnter();
                    
                    SYSTICK_DelayUs(500); 
                    SYSTICK_TimerStart();
                    
                    break;
                }

                
            default:
                if(change_detect)
                {
                    SYSTICK_DelayUs(1000); 
                    SYSTICK_TimerStop(); 
                    
                    PM_StandbyModeEnter();
                    
                    SYSTICK_DelayUs(500); 
                    SYSTICK_TimerStart(); 
                }
                break;
        }  
    }
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/




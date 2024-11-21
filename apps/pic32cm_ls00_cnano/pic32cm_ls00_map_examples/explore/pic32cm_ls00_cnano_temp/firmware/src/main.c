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
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes


#define TEMP_SENSOR_SLAVE_ADDR                  0x1C

#define TEMP_SENSOR_REG_ADDR                    0x05
#define TEMP_SENSOR_MANF_ID_REG                 0x06
#define TEMP_SENSOR_DEV_ID_REG                  0x07
#define TEMP_SENSOR_CONFIG_REG                  0x01
#define TEMP_SENSOR_RESOLUTION_REG              0x08

#define TEMP_SENSOR_MANF_VAL                    0x0054
#define TEMP_SENSOR_DEV_VAL                     0x0400
#define CONFIG_VAL                              0x0000
#define RESOLUTION_VAL                          0x03


/* RTC Time period match values for input clock of 1 KHz */
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
static volatile bool isRTCTimerExpired = false;
static volatile bool changeTempSamplingRate = false;
static volatile bool isTemperatureRead = false;
static volatile bool i2ctransferDone = false;
static volatile bool isUSARTTxComplete = false;
       uint16_t temperatureVal;
static uint8_t i2cWrData = TEMP_SENSOR_REG_ADDR;
       uint8_t UartTxBuffer[100] = {0};
       uint16_t TemperatureRawData;
       
typedef enum
{
    TEMP_SAMPLING_RATE_500MS = 0,
    TEMP_SAMPLING_RATE_1S = 1,
    TEMP_SAMPLING_RATE_2S = 2,
    TEMP_SAMPLING_RATE_4S = 3,
            
} TEMP_SAMPLING_RATE;

static TEMP_SAMPLING_RATE tempSampleRate = TEMP_SAMPLING_RATE_500MS;

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
/*MCP9808 I2C WriteRead function*/
static uint16_t MCP9808_WriteRead(uint8_t reg_addr, uint8_t rx_len) 
{ 
    uint8_t rxData[2];
    
    i2ctransferDone = false;
    
    SERCOM2_I2C_WriteRead(TEMP_SENSOR_SLAVE_ADDR, &reg_addr, 1, rxData, rx_len);
        
    while ( i2ctransferDone != true);
    
    return (rxData[0] << 8) | rxData[1];

}
/*MCP9808 I2C Write function*/
static bool MCP9808_Write(uint8_t reg_addr, uint16_t value, uint8_t len) 
{
    uint8_t data[3];

    data[0] = reg_addr;             // Register address
    
    if(len != 1)
    {
        data[1] = (value >> 8) & 0xFF;  // MSB of value
        data[2] = value & 0xFF;         // LSB of value
    }
    
    else
    {
        data[1] = value & 0xFF;
        len = 2;
    }
    
    i2ctransferDone = false;
    
    SERCOM2_I2C_Write(TEMP_SENSOR_SLAVE_ADDR, data, len);
    
    while ( i2ctransferDone != true);
    
    return true;  // Success
}
/*MCP9808 initialization*/
static bool MCP9808_Initialize()
{    
    /*Check Manufacture ID*/
    if(MCP9808_WriteRead(TEMP_SENSOR_MANF_ID_REG, 3) != TEMP_SENSOR_MANF_VAL)
    {
        return false;
    }
    /*Check Device ID*/
    if(MCP9808_WriteRead(TEMP_SENSOR_DEV_ID_REG , 3) != TEMP_SENSOR_DEV_VAL)
    {
        return false;
    }
    /*Set Default configuration*/
    if(MCP9808_Write(TEMP_SENSOR_CONFIG_REG, CONFIG_VAL, 3) != true)
    {
        return false;
    }
    /*Set Resolution*/
    if(MCP9808_Write(TEMP_SENSOR_RESOLUTION_REG, RESOLUTION_VAL, 1) != true)
    {
        return false;
    }
    
    return true;
}
/*EIC Interrupt handler*/
static void EIC_User_Handler(uintptr_t context)
{
    changeTempSamplingRate = true;      
}
/*RTC Interrupt handler*/
static void rtcEventHandler (RTC_TIMER32_INT_MASK intCause, uintptr_t context)
{
    if (intCause & RTC_TIMER32_INT_MASK_CMP0)
    {            
        isRTCTimerExpired = true;                              
    }
}
/*SERCOM I2C Interrupt handler*/
static void i2cEventHandler(uintptr_t contextHandle)
{
    if (SERCOM2_I2C_ErrorGet() == SERCOM_I2C_ERROR_NONE)
    {
        i2ctransferDone       = true;
    }
    
    else
    {
        i2ctransferDone = false;
    }
}
/*Get Temperature value*/
static uint16_t getTemperature(uint16_t rawTempValue)
{
    int16_t temp;
    int16_t UpperByte;
    int16_t LowerByte;
    // Convert the temperature value read from sensor to readable format (Degree Celsius)
    // For demonstration purpose, temperature value is assumed to be positive.
    // The maximum positive temperature measured by sensor is +125 C
    
    UpperByte = (rawTempValue >> 8) & 0XFF;
    LowerByte = rawTempValue & 0XFF;
    
    UpperByte = UpperByte & 0x1F;
    if ((UpperByte & 0x10) == 0x10)
    {
        UpperByte = UpperByte & 0x0F;
        temp = 256 - (UpperByte * 16 + LowerByte / 16);
    }else
    {
        temp = (UpperByte * 16 + LowerByte / 16);
    }
    temp = (temp * 9/5) + 32; /*Celsius to Fahrenheit*/
    return temp;
}
/*SERCOM USART Interrupt handler*/
static void usartTxDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUSARTTxComplete = true;
    }
}

int main ( void )
{

    /* Initialize all modules */
    SYS_Initialize ( NULL );
    /*SERCOM I2C callback register*/
    SERCOM2_I2C_CallbackRegister(i2cEventHandler, 0);
    /*Initialize MCP9808 Sensor*/
    MCP9808_Initialize();
    /*EIC callback register*/    
    EIC_CallbackRegister(EIC_PIN_2,EIC_User_Handler, 0);
    /*DMAC callback register*/
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartTxDmaChannelHandler, 0);
    /*RTC callback register*/
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Start();    

   while ( true )
    {
        if ((i2ctransferDone == true) && 
        (isRTCTimerExpired == true))  //Temperature Reading from Sensor
        {
        isRTCTimerExpired = false;
        i2ctransferDone = false;
        TemperatureRawData = MCP9808_WriteRead(i2cWrData, 2);
        isTemperatureRead = true;   
        }
        
    if (isTemperatureRead == true)
      {
        isTemperatureRead = false;
        if(changeTempSamplingRate == false)    
            {
            temperatureVal = getTemperature(TemperatureRawData);
            
            memset((char*)UartTxBuffer, 0x00, 100);
            sprintf((char*)UartTxBuffer, "Temperature = %02d F\r\n", temperatureVal);
            
            LED_Toggle();
            }
        else
          {
            changeTempSamplingRate = false;
            
            if(tempSampleRate == TEMP_SAMPLING_RATE_500MS)
            {
                tempSampleRate = TEMP_SAMPLING_RATE_1S;
                sprintf((char*)UartTxBuffer, "Sampling Temperature every 1 second \r\n");
                RTC_Timer32CompareSet(PERIOD_1S);
            }
            
            else if(tempSampleRate == TEMP_SAMPLING_RATE_1S)
            {
                tempSampleRate = TEMP_SAMPLING_RATE_2S;
                sprintf((char*)UartTxBuffer, "Sampling Temperature every 2 seconds \r\n");        
                RTC_Timer32CompareSet(PERIOD_2S);                        
            }
            
            else if(tempSampleRate == TEMP_SAMPLING_RATE_2S)
            {
                tempSampleRate = TEMP_SAMPLING_RATE_4S;
                sprintf((char*)UartTxBuffer, "Sampling Temperature every 4 seconds \r\n");        
                RTC_Timer32CompareSet(PERIOD_4S);                                        
            }    
            
            else if(tempSampleRate == TEMP_SAMPLING_RATE_4S)
            {
               tempSampleRate = TEMP_SAMPLING_RATE_500MS;
               sprintf((char*)UartTxBuffer, "Sampling Temperature every 500 ms \r\n");        
               RTC_Timer32CompareSet(PERIOD_500MS);
            }
            
            else
                {
                    ;
                }
          }
        
        DMAC_ChannelTransfer(DMAC_CHANNEL_0, UartTxBuffer, \
                    (const void *)&(SERCOM3_REGS->USART_INT.SERCOM_DATA), \
                    strlen((const char*)UartTxBuffer));
      }
    }
    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/


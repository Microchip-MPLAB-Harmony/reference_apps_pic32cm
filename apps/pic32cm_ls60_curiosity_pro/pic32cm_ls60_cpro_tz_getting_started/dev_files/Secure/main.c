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

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes

#define TEMP_SENSOR_SLAVE_ADDR                  0x004F
#define TEMP_SENSOR_REG_ADDR                    0x00
#define EEPROM_I2C_SLAVE_ADDR                   0x0057
#define EEPROM_LOG_MEMORY_ADDR                  0x00
#define EEPROM_MAX_LOG_VALUES                   5
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096

/* typedef for non-secure callback functions */
typedef void (*funcptr_void) (void) __attribute__((cmse_nonsecure_call));

static volatile bool isRTCTimerExpired = false;
static volatile bool changeSamplingRate = false;
static volatile bool isI2CFree = true;
static volatile bool isTemperatureRead = false;
static volatile bool isEEPROMReadReq = false;
static volatile bool printLedToggleRate = false;
static volatile bool printTempSampleRate = false;
static uint8_t i2cAddress;
static uint8_t i2cWrData = TEMP_SENSOR_REG_ADDR;
static uint8_t i2cRdData[2] = {0};
static uint8_t eepromTxBuffer[2] = {0};
static const char timeouts[4][20] = {"500 milliSeconds", "1 Second",  "2 Seconds",  "4 Seconds"};

volatile bool startTemperatureReading = false;
volatile bool isEEPROMRead = false;
volatile bool readUartTxStatus = false;
volatile bool eepromTemperatureDataReadStatus = false;

uint8_t wrIndex  = 0;
uint8_t temperatureVal;
uint8_t *eepromRxBuffer;
uint8_t uartTxTempBuffer[100] = {0};
bool showErrorMsg = false;

typedef enum
{
    SAMPLING_RATE_500MS = 0,
    SAMPLING_RATE_1S    = 1,
    SAMPLING_RATE_2S    = 2,
    SAMPLING_RATE_4S    = 3,
} SAMPLING_RATE;

static SAMPLING_RATE tempSampleRate = SAMPLING_RATE_500MS;

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

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


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


/* Timer Counter Time period match values for input clock of 4096 Hz */
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096

typedef enum
{
    LED_SAMPLING_RATE_500MS = 0,
    LED_SAMPLING_RATE_1S = 1,
    LED_SAMPLING_RATE_2S = 2,
    LED_SAMPLING_RATE_4S = 3,
} LED_SAMPLING_RATE;
static LED_SAMPLING_RATE ledSampleRate = LED_SAMPLING_RATE_500MS;
static const char timeouts[4][20] = {"500 milliSeconds", "1 Second",  "2 Seconds",  "4 Seconds"};

static volatile bool isTC0TimerExpired = false;
static volatile bool changeLedSamplingRate = false;
static volatile bool isUSARTTxComplete = true;
static uint8_t uartTxBuffer[100] = {0};

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
static void EIC_User_Handler(uintptr_t context)
{
    changeLedSamplingRate = true;
}

static void tc0EventHandler (TC_TIMER_STATUS status, uintptr_t context)
{
    isTC0TimerExpired = true;                              
}

static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUSARTTxComplete = true;
    }
}


int main ( void )
{
    uint8_t uartLocalTxBuffer[100] = {0};
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    EIC_CallbackRegister(EIC_PIN_0,EIC_User_Handler, 0);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    TC0_TimerCallbackRegister(tc0EventHandler, 0);
    
    sprintf((char*)uartTxBuffer, "Toggling LED at 500 milliseconds rate \r\n");
    TC0_TimerStart();
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */

        if ((isTC0TimerExpired == true)&& (true == isUSARTTxComplete))
        {
            isTC0TimerExpired = false;
            isUSARTTxComplete = false;
            LED_Toggle();
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, \
                    (const void *)&(SERCOM0_REGS->USART_INT.SERCOM_DATA), \
                    strlen((const char*)uartTxBuffer));
        }
        
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        if(changeLedSamplingRate == true)
        {
            changeLedSamplingRate = false;
            if(ledSampleRate == LED_SAMPLING_RATE_500MS)
            {
                ledSampleRate = LED_SAMPLING_RATE_1S;
                TC0_Timer32bitCounterSet(PERIOD_1S);
            }
            else if(ledSampleRate == LED_SAMPLING_RATE_1S)
            {
                ledSampleRate = LED_SAMPLING_RATE_2S;
                TC0_Timer32bitCounterSet(PERIOD_2S);                        
            }
            else if(ledSampleRate == LED_SAMPLING_RATE_2S)
            {
                ledSampleRate = LED_SAMPLING_RATE_4S;
                TC0_Timer32bitCounterSet(PERIOD_4S);                                        
            }    
            else if(ledSampleRate == LED_SAMPLING_RATE_4S)
            {
               ledSampleRate = LED_SAMPLING_RATE_500MS;
               TC0_Timer32bitCounterSet(PERIOD_500MS);
            }
            else
            {
                ;
            }
            TC0_Timer32bitCounterSet(0);
            sprintf((char*)uartLocalTxBuffer, "LED Toggling rate is changed to %s\r\n", &timeouts[(uint8_t)ledSampleRate][0]);
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartLocalTxBuffer, \
                    (const void *)&(SERCOM0_REGS->USART_INT.SERCOM_DATA), \
                    strlen((const char*)uartLocalTxBuffer));
            sprintf((char*)uartTxBuffer, "Toggling LED at %s rate \r\n", &timeouts[(uint8_t)ledSampleRate][0]);
        }        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/


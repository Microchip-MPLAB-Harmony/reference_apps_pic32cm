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
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <stdio.h>
#include "definitions.h"                // SYS function prototypes

#define LED_DATA_WIDTH 24                                   //No of bits for a single LED
#define NO_OF_LEDS 8                                        //No of LED's
#define NO_OF_BUFFER 4                                      //No of Buffers used for storing LED data
#define BUFFER_SIZE  LED_DATA_WIDTH*NO_OF_LEDS*4            //Buffer Size for DMAC transfer
#define T1H 20U                                             //To send Bit '1' 68% of the period value will be high
#define T1L 9U                                              //To send Bit '0' 32% of the period value will be high

/*Storage of LED data using 3D array*/
uint32_t DMAC_Buffer[NO_OF_BUFFER][NO_OF_LEDS][LED_DATA_WIDTH];

/*DMAC Callback function*/
void APP_DMAC_Callback(DMAC_TRANSFER_EVENT status, uintptr_t context)
{    
    if(status == DMAC_TRANSFER_EVENT_COMPLETE)  //DMAC Transfer complete check
    {
        TCC3_PWMStop(); //TCC stop condition
       
    }
}

/*Updating color in the buffer */
void APP_WS2812B_Update_Color(uint8_t buffer_index, uint8_t green, uint8_t red, uint8_t blue)
{
   for (int i = 0; i < NO_OF_LEDS; i++)  //This for loop is used to allocate 8 LED's
    {
        for (int j = 7; j >= 0; j--)    //This for loop is used to allocate the 8 LED bits from the MSB bit as 7th position
        {
            //Allocating Green LED bits
            DMAC_Buffer[buffer_index][i][j] = (green >>(7 - j) & 1);   
            
            if(DMAC_Buffer[buffer_index][i][j] == 1)
            {
                DMAC_Buffer[buffer_index][i][j] = T1H;
            }
            else
            {
                DMAC_Buffer[buffer_index][i][j] = T1L;
            }
            
            //Allocating Red LED bits
            DMAC_Buffer[buffer_index][i][8 + j] = (red >> (7 - j)) & 1;    
            
            if(DMAC_Buffer[buffer_index][i][8 + j] == 1)
            {
                DMAC_Buffer[buffer_index][i][8 + j] = T1H;
            }
            else
            {
                DMAC_Buffer[buffer_index][i][8 + j] = T1L;
            }
            
            //Allocating Blue LED bits
            DMAC_Buffer[buffer_index][i][16 + j] = (blue >> (7 - j)) & 1;  
            
            if(DMAC_Buffer[buffer_index][i][16 + j] == 1)
            {
                DMAC_Buffer[buffer_index][i][16 + j] = T1H;
            }
            else
            {
                DMAC_Buffer[buffer_index][i][16 + j] = T1L;
            }
        }
    }
}

int main ( void )
{
    /* Initialize all modules */
    
    SYS_Initialize ( NULL );
    
    /*DMAC Call back register*/
    
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, APP_DMAC_Callback, 0);
    
    /*Updating Green color in the Buffer*/
    
    APP_WS2812B_Update_Color(0, 50, 0, 0);
    
    /*Updating Red color in the Buffer*/
    
    APP_WS2812B_Update_Color(1, 0, 50, 0);
    
    
    /*Updating Blue color in the Buffer*/
    
    APP_WS2812B_Update_Color(2, 0, 0, 50);
    
    /*Updating white color in the Buffer*/
    
    APP_WS2812B_Update_Color(3, 50, 50, 50);
    
    /*Systick timer start function*/
    
    SYSTICK_TimerStart();
    
    while ( true )
    {
        SYS_Tasks( );
        for(int i = 0; i < NO_OF_BUFFER; i++)
        {
            /*DMAC transfer function*/
            DMAC_ChannelTransfer( DMAC_CHANNEL_0, (void*)&DMAC_Buffer[i][0][0], (void*)&TCC3_REGS->TCC_CCBUF[1] , BUFFER_SIZE );
            
            /*TCC3 PWM start function*/
            TCC3_PWMStart();
            
            /*One second delay using Systick*/
            SYSTICK_DelayMs(1000);
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/


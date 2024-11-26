/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pir.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
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
s
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_pir.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// I2C CLIENT 2 Address
#define PIR_SENSOR_CLIENT_ADDR                 0x4D

#define PIR_SENSOR_ADC_SCALE                   4095     // 12- bit resolution
#define PIR_SENSOR_ADC_REF_VOLT                (3.3f)   // 3.3v 
#define PIR_SENSOR_ADC_THRESHOLD               (1.45f)  // 1.45v

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_PIR_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_PIR_DATA app_pirData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void APP_PIR_I2C_Event_Handler ( 
                                        DRV_I2C_TRANSFER_EVENT event,
                                        DRV_I2C_TRANSFER_HANDLE transferHandle,
                                        uintptr_t context
                                        )
{
    APP_PIR_I2C_TRANSFER_STATUS* transferStatus = (APP_PIR_I2C_TRANSFER_STATUS*)context;

    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE)
    {
        if (transferStatus)
        {
            *transferStatus = APP_PIR_I2C_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (transferStatus)
        {
            *transferStatus = APP_PIR_I2C_TRANSFER_STATUS_ERROR;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


bool APP_PIR_TaskIsReady (void)
{
    return app_pirData.ready;
}

// MCP3221 Temperature Sensor Data
bool APP_PIR_MCP3221_Sensor_Data (void)
{
    uint8_t rxData[2];
    
    uint16_t pir_adc;
    
    float pir_output;
    
    //Read the ambient temperature 
    app_pirData.transferStatus = APP_PIR_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_ReadTransferAdd(app_pirData.drvI2CHandle, PIR_SENSOR_CLIENT_ADDR,
                                (void *)&rxData[0], 2, &app_pirData.transferHandle );
    
    while (app_pirData.transferStatus != APP_PIR_I2C_TRANSFER_STATUS_SUCCESS);
    
    pir_adc = (rxData[0] << 8 | rxData[1]) & 0xFFF;
    
    pir_output = (pir_adc * PIR_SENSOR_ADC_REF_VOLT) / PIR_SENSOR_ADC_SCALE;
    
    if(pir_output >= PIR_SENSOR_ADC_THRESHOLD)
    {
        return true;
    }
    
    return false;
}    
   


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PIR_Initialize ( void )

  Remarks:
    See prototype in app_pir.h.
 */

void APP_PIR_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_pirData.state = APP_PIR_STATE_INIT;
    app_pirData.ready = false;
}


/******************************************************************************
  Function:
    void APP_PIR_Tasks ( void )

  Remarks:
    See prototype in app_pir.h.
 */

void APP_PIR_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_pirData.state )
    {
        /* Application's initial state. */
        case APP_PIR_STATE_INIT:
        {
            /* Open the I2C Driver */
            app_pirData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            
            if(app_pirData.drvI2CHandle == DRV_HANDLE_INVALID)
            {
                app_pirData.state = APP_PIR_STATE_ERROR;
            }
            
            else
            {
                /* Register the I2C Driver event Handler */
                DRV_I2C_TransferEventHandlerSet(
                                                app_pirData.drvI2CHandle,
                                                APP_PIR_I2C_Event_Handler,
                                                (uintptr_t)&app_pirData.transferStatus
                                                );
                
                app_pirData.state = APP_PIR_STATE_IDLE;
                app_pirData.ready = true;
                
            }
            break;
        }

        case APP_PIR_STATE_IDLE:
        {            
            break;
        }
        
        case APP_PIR_STATE_ERROR:
        {
            app_pirData.ready = false;
            app_pirData.state = APP_PIR_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_PIR_TASK: Task Error ...!");
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */

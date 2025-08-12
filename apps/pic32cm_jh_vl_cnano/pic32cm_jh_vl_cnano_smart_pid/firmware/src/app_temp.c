/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_temp.c

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

#include "app_temp.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_TEMP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_TEMP_DATA app_tempData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void APP_TEMP_I2C_Event_Handler ( 
                                        DRV_I2C_TRANSFER_EVENT event,
                                        DRV_I2C_TRANSFER_HANDLE transferHandle,
                                        uintptr_t context
                                        )
{
    APP_TEMP_I2C_TRANSFER_STATUS* transferStatus = (APP_TEMP_I2C_TRANSFER_STATUS*)context;

    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE)
    {
        if (transferStatus)
        {
            *transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (transferStatus)
        {
            *transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_ERROR;
        }
    }
}

static void APP_TEMP_HTU21D_TimerCallback()
{
    app_tempData.tmrExpired = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static void APP_TEMP_DelayMS (int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;
    
    while (SYS_TIME_DelayIsComplete(timer) == false);
}

static bool APP_TEMP_HTU21D_Write(uint8_t reg_addr, uint16_t value, uint8_t len)
{
    uint8_t txData[2];
    
    txData[0] = reg_addr;
    txData[1] = value;
    
    app_tempData.transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    DRV_I2C_WriteTransferAdd(app_tempData.drvI2CHandle, TEMP_SENSOR_CLIENT_ADDR,
                            (void *)&txData, len, 
                            &app_tempData.transferHandle );
    
    while (app_tempData.transferStatus != APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS);
    
    return true;
}

static uint16_t APP_TEMP_HTU21D_Read(uint8_t reg_addr, uint8_t delay, uint8_t len)
{
    uint8_t rxData[2];
    
    APP_TEMP_HTU21D_Write(reg_addr, 0, 1);
    APP_TEMP_DelayMS(50);
    
    app_tempData.transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    DRV_I2C_ReadTransferAdd(app_tempData.drvI2CHandle, TEMP_SENSOR_CLIENT_ADDR,
                            (void *)&rxData[0], len, 
                            &app_tempData.transferHandle );
    
    while (app_tempData.transferStatus != APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS);
    
    return ((rxData[0]<<8) | rxData[1]);
}

uint16_t APP_TEMP_HTU21D_Get_Temp()
{       
    return app_tempData.temperature;
}

static bool APP_TEMP_HTU21D_Init()
{
    if(APP_TEMP_HTU21D_Write(TEMP_SENSOR_SOFT_RESET, 0, 1) != true)
    {
        return false;
    }
    APP_TEMP_DelayMS(1);
    
    if(APP_TEMP_HTU21D_Write(TEMP_SENSOR_WRITE_REG, TEMP_SENSOR_VAL, 2) != true)
    {
        return false;
    }
    APP_TEMP_DelayMS(50);
    
    return true;
}

bool APP_TEMP_TaskIsReady (void)
{
    return app_tempData.ready;
}

void APP_TEMP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_tempData.state = APP_TEMP_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_TEMP_Initialize ( void )

  Remarks:
    See prototype in app_temp.h.
 */

void APP_TEMP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_tempData.state )
    {
        /* Application's initial state. */
        case APP_TEMP_STATE_INIT:
        {
            /* Open the I2C Driver */
            app_tempData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            
            if(app_tempData.drvI2CHandle == DRV_HANDLE_INVALID)
            {
                app_tempData.state = APP_TEMP_STATE_ERROR;
            }
            
            else
            {
                /* Register the I2C Driver event Handler */
                DRV_I2C_TransferEventHandlerSet(
                                                app_tempData.drvI2CHandle,
                                                APP_TEMP_I2C_Event_Handler,
                                                (uintptr_t)&app_tempData.transferStatus
                                                );
                
                app_tempData.state = APP_TEMP_STATE_CONFIG;
            }
            
            break;
        }

        case APP_TEMP_STATE_CONFIG:
        {
            if(APP_TEMP_HTU21D_Init() == true)
            {           
                app_tempData.tmrHandle = SYS_TIME_CallbackRegisterMS(APP_TEMP_HTU21D_TimerCallback, 0,
                    100, SYS_TIME_PERIODIC);
                
                app_tempData.ready = true;
                app_tempData.state = APP_TEMP_STATE_IDLE;
            }
            
            else
            {
                app_tempData.state = APP_TEMP_STATE_ERROR;
            }
            break;
        }
        
        case APP_TEMP_STATE_IDLE:
        {           
            if(app_tempData.tmrExpired == true)
            {
                app_tempData.tmrExpired = false;
                
                app_tempData.sensor_data = APP_TEMP_HTU21D_Read
                        (TEMP_SENSOR_MEASUREMENT, TEMP_SENSOR_MEASURE_TIME, 2);
                app_tempData.sensor_data = (app_tempData.sensor_data & 0xFFFC);
    
                app_tempData.temperature = (uint16_t)(-46.85 + 175.72 * 
                                 ((float)app_tempData.sensor_data / 65536.0f));
            }
            break;
        }
        
        case APP_TEMP_STATE_ERROR:
        {
            app_tempData.ready = false;
            app_tempData.state = APP_TEMP_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_TEMP_TASK: Task Error ...!");
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

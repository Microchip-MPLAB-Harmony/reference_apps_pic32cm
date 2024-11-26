/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_light.c

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_light.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// I2C CLIENT 1 Address
#define LIGHT_SENSOR_CLIENT_ADDR                 0x51

// Light Sensor Register Address
#define LIGHT_SENSOR_CONFIG_REG                  0x00
#define LIGHT_SENSOR_DATA_REG_ADDR               0x09
#define LIGHT_SENSOR_DEV_ID_REG                  0x0E

// Light Sensor Register Value
#define LIGHT_SENSOR_CONFIG_VAL                  0x0100
#define LIGHT_SENSOR_DEV_ID_VAL                  0x1058

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_LIGHT_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_LIGHT_DATA app_lightData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void APP_LIGHT_I2C_Event_Handler ( 
                                        DRV_I2C_TRANSFER_EVENT event,
                                        DRV_I2C_TRANSFER_HANDLE transferHandle,
                                        uintptr_t context
                                        )
{
    APP_LIGHT_I2C_TRANSFER_STATUS* transferStatus = (APP_LIGHT_I2C_TRANSFER_STATUS*)context;

    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE)
    {
        if (transferStatus)
        {
            *transferStatus = APP_LIGHT_I2C_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (transferStatus)
        {
            *transferStatus = APP_LIGHT_I2C_TRANSFER_STATUS_ERROR;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


static void APP_LIGHT_DelayMS (int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;
    
    while (SYS_TIME_DelayIsComplete(timer) == false);
}


// VCNL4200 I2C Write Function
static bool APP_LIGHT_VCNL4200_Write (uint8_t reg_addr, uint16_t value, uint8_t len) 
{
    uint8_t txData[3];

    txData[0] = reg_addr;             // Register address
    txData[1] = value & 0xFF;         // LSB of value
    txData[2] = (value >> 8) & 0xFF;  // MSB of value

    app_lightData.transferStatus = APP_LIGHT_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteTransferAdd(app_lightData.drvI2CHandle, LIGHT_SENSOR_CLIENT_ADDR,
                            (void *)&txData[0], len, 
                            &app_lightData.transferHandle );
    
    while (app_lightData.transferStatus != APP_LIGHT_I2C_TRANSFER_STATUS_SUCCESS);
    
    return true;  
}


// VCNL4200 I2C Write Read Function
static uint16_t APP_LIGHT_VCNL4200_WriteRead (uint8_t reg_addr, uint8_t len) 
{ 
    uint8_t rxData[2];
    
    app_lightData.transferStatus = APP_LIGHT_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write and read the application data */
    DRV_I2C_WriteReadTransferAdd(app_lightData.drvI2CHandle, LIGHT_SENSOR_CLIENT_ADDR,
                                (void *)&reg_addr, 1, (void *)&rxData[0], len, 
                                &app_lightData.transferHandle );
    
    while (app_lightData.transferStatus != APP_LIGHT_I2C_TRANSFER_STATUS_SUCCESS);

    return (rxData[1] << 8) | rxData[0];
}


// VCNL4200 Initialization
static bool APP_LIGHT_VCNL4200_INIT (void)
{       
    // Device ID Check
    if(APP_LIGHT_VCNL4200_WriteRead(LIGHT_SENSOR_DEV_ID_REG , 2) != LIGHT_SENSOR_DEV_ID_VAL)
    {
        return false;
    }
    APP_LIGHT_DelayMS(1);
    
    // Light Sensor Configuration 
    if(APP_LIGHT_VCNL4200_Write(LIGHT_SENSOR_CONFIG_REG, LIGHT_SENSOR_CONFIG_VAL, 3) != true)
    {
        return false;
    }
    APP_LIGHT_DelayMS(1);
    
    return true;
}


bool APP_LIGHT_TaskIsReady (void)
{
    return app_lightData.ready;
}


// VCNL4200 Light Sensor Data
uint16_t APP_LIGHT_VCNL4200_Sensor_Data (void)
{    
    return APP_LIGHT_VCNL4200_WriteRead(LIGHT_SENSOR_DATA_REG_ADDR, 2);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_LIGHT_Initialize ( void )

  Remarks:
    See prototype in app_light.h.
 */

void APP_LIGHT_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_lightData.state = APP_LIGHT_STATE_INIT;
    app_lightData.ready = false;
}


/******************************************************************************
  Function:
    void APP_LIGHT_Tasks ( void )

  Remarks:
    See prototype in app_light.h.
 */

void APP_LIGHT_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_lightData.state )
    {
        /* Application's initial state. */
        case APP_LIGHT_STATE_INIT:
        {
            /* Open the I2C Driver */
            app_lightData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            
            if(app_lightData.drvI2CHandle == DRV_HANDLE_INVALID)
            {
                app_lightData.state = APP_LIGHT_STATE_ERROR;
            }
            
            else
            {
                /* Register the I2C Driver event Handler */
                DRV_I2C_TransferEventHandlerSet(
                                                app_lightData.drvI2CHandle,
                                                APP_LIGHT_I2C_Event_Handler,
                                                (uintptr_t)&app_lightData.transferStatus
                                                );
                
                app_lightData.state = APP_LIGHT_STATE_CONFIG;
            }
            
            break;
        }

        case APP_LIGHT_STATE_CONFIG:
        {
            if(APP_LIGHT_VCNL4200_INIT() == true)
            {
                app_lightData.ready = true;
                app_lightData.state = APP_LIGHT_STATE_IDLE;
            }
            
            else
            {
                app_lightData.state = APP_LIGHT_STATE_ERROR;
            }
            break;
        }

        case APP_LIGHT_STATE_IDLE:
        {            
            break;
        }
        
        case APP_LIGHT_STATE_ERROR:
        {
            app_lightData.ready = false;
            app_lightData.state = APP_LIGHT_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_LIGHT_TASK: Task Error ...!");
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

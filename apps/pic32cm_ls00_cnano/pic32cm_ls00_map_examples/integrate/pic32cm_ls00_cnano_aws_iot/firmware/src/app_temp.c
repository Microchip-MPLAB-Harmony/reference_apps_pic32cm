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

#include "app_temp.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// I2C CLIENT 0 Address
#define TEMP_SENSOR_CLIENT_ADDR                 0x1C

// Temperature Sensor Register Address
#define TEMP_SENSOR_CONFIG_REG                  0x01
#define TEMP_SENSOR_DATA_REG_ADDR               0x05
#define TEMP_SENSOR_MANF_ID_REG                 0x06
#define TEMP_SENSOR_DEV_ID_REG                  0x07
#define TEMP_SENSOR_RESOLUTION_REG              0x08

// Temperature Sensor Register Value
#define TEMP_SENSOR_CONFIG_VAL                  0x0000
#define TEMP_SENSOR_MANF_ID_VAL                 0x0054
#define TEMP_SENSOR_DEV_ID_VAL                  0x0400
#define TEMP_SENSOR_RESOLUTION_VAL              0x03

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

// MCP9808 I2C Write Function
static bool APP_TEMP_MCP9808_Write (uint8_t reg_addr, uint16_t value, uint8_t len) 
{
    uint8_t txData[3];

    txData[0] = reg_addr;             // Register address
    
    if(len == 3)
    {
        txData[1] = (value >> 8) & 0xFF;  // MSB of value
        txData[2] = value & 0xFF;         // LSB of value
    }
    
    else
    {
        txData[1] = value & 0xFF; 
    }
    
    app_tempData.transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteTransferAdd(app_tempData.drvI2CHandle, TEMP_SENSOR_CLIENT_ADDR,
                            (void *)&txData[0], len, 
                            &app_tempData.transferHandle );
    
    while (app_tempData.transferStatus != APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS);
    
    return true;  
}


// MCP9808 I2C Write Read Function
static uint16_t APP_TEMP_MCP9808_WriteRead (uint8_t reg_addr, uint8_t len) 
{ 
    uint8_t rxData[2];
    
    app_tempData.transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteReadTransferAdd(app_tempData.drvI2CHandle, TEMP_SENSOR_CLIENT_ADDR,
                                (void *)&reg_addr, 1, (void *)&rxData[0], len, 
                                &app_tempData.transferHandle );
    
    while (app_tempData.transferStatus != APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS);

    return (rxData[0] << 8) | rxData[1];
}


// MCP9808 Initialization
static bool APP_TEMP_MCP9808_INIT (void)
{    
    //Manufacture ID Check
    if(APP_TEMP_MCP9808_WriteRead(TEMP_SENSOR_MANF_ID_REG, 2) != TEMP_SENSOR_MANF_ID_VAL)
    {
        return false;
    }
    APP_TEMP_DelayMS(1);
    
    // Device ID Check
    if(APP_TEMP_MCP9808_WriteRead(TEMP_SENSOR_DEV_ID_REG , 2) != TEMP_SENSOR_DEV_ID_VAL)
    {
        return false;
    }
    APP_TEMP_DelayMS(1);
    
    // Default Configuration
    if(APP_TEMP_MCP9808_Write(TEMP_SENSOR_CONFIG_REG, TEMP_SENSOR_CONFIG_VAL, 3) != true)
    {
        return false;
    }
    APP_TEMP_DelayMS(1);
    
    // Resolution Setting
    if(APP_TEMP_MCP9808_Write(TEMP_SENSOR_RESOLUTION_REG, TEMP_SENSOR_RESOLUTION_VAL, 2) != true)
    {
        return false;
    }
    APP_TEMP_DelayMS(1);
    
    return true;
}


bool APP_TEMP_TaskIsReady (void)
{
    return app_tempData.ready;
}


// MCP9808 Temperature Sensor Data
int16_t APP_TEMP_MCP9808_Sensor_Data (void)
{
    uint8_t txdata = TEMP_SENSOR_DATA_REG_ADDR;
    uint8_t rxData[2];   
    
    //Read the ambient temperature 
    app_tempData.transferStatus = APP_TEMP_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteReadTransferAdd(app_tempData.drvI2CHandle, TEMP_SENSOR_CLIENT_ADDR,
                                (void *)&txdata, 1, (void *)&rxData[0], 2, 
                                &app_tempData.transferHandle );
    
    while (app_tempData.transferStatus != APP_TEMP_I2C_TRANSFER_STATUS_SUCCESS);
    
    //First Check flag bits
    if ((rxData[0] & 0x80) == 0x80)
    { 
        // TA >= TCRIT
    }
    
    if ((rxData[0] & 0x40) == 0x40)
    { 
        // TA > TUPPER
    }
    
    if ((rxData[0] & 0x20) == 0x20)
    { 
        // TA < TLOWER
    }
    
    //Clear flag bits
    rxData[0] = rxData[0] & 0x1F; 
    
    if ((rxData[0] & 0x10) == 0x10)
    { 
        //TA < 0�C
        rxData[0] = rxData[0] & 0x0F; 
        
        //Clear SIGN
        app_tempData.temperature = 256 - ((rxData[0] * 16) + (rxData[1] / 16));
    }
    
    else 
    {
        //TA >= 0�C
        app_tempData.temperature = ((rxData[0] * 16) + (rxData[1] / 16));
    } 
    
    return app_tempData.temperature;
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

void APP_TEMP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_tempData.state = APP_TEMP_STATE_INIT;
    app_tempData.ready = false;
}


/******************************************************************************
  Function:
    void APP_TEMP_Tasks ( void )

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
            if(APP_TEMP_MCP9808_INIT() == true)
            {
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

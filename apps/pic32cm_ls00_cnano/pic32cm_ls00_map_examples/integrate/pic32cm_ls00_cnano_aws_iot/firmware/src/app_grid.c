/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_grid.c

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

#include "app_grid.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


// I2C CLIENT 3 Address
#define GRID_SENSOR_CLIENT_ADDR                 0x68

// Grid Eye Sensor Register Address
#define GRID_SENSOR_POWER_MODE_REG              0x00
#define GRID_SENSOR_RESET_REG                   0x01
#define GRID_SENSOR_FRAME_RATE_REG              0x02
#define GRID_SENSOR_GRID_DATA_REG               0x80

// Grid Eye Sensor Register Value
#define GRID_SENSOR_POWER_MODE_VAL              0x00    // Normal Mode
#define GRID_SENSOR_RESET_VAL                   0x30    // Flag Reset
#define GRID_SENSOR_FRAME_RATE_VAL              0x00    // 10 Frames per second

#define GRID_SENSOR_GRID_PXL_SIZE               0x40    // 64 Pixels
#define GRID_SENSOR_GRID_DATA_SIZE              0x80    // 128 Bytes (each pixel 2 bytes)
#define GRID_SENSOR_GRID_TEMP_COEFF             (0.25f)
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_GRID_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_GRID_DATA app_gridData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void APP_GRID_I2C_Event_Handler ( 
                                        DRV_I2C_TRANSFER_EVENT event,
                                        DRV_I2C_TRANSFER_HANDLE transferHandle,
                                        uintptr_t context
                                        )
{
    APP_GRID_I2C_TRANSFER_STATUS* transferStatus = (APP_GRID_I2C_TRANSFER_STATUS*)context;

    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE)
    {
        if (transferStatus)
        {
            *transferStatus = APP_GRID_I2C_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (transferStatus)
        {
            *transferStatus = APP_GRID_I2C_TRANSFER_STATUS_ERROR;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

// Grid Eye Sensor Delay function 
static void APP_GRID_DelayMS (int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;
    
    while (SYS_TIME_DelayIsComplete(timer) == false);
}

// Grid Eye Sensor Status Function 
bool APP_GRID_TaskIsReady (void)
{
    return app_gridData.ready;
}

// AMG8853 I2C Write Function
static bool APP_GRID_AMG8853_Write (uint8_t reg_addr, uint16_t value, uint8_t len) 
{
    uint8_t txData[3];

    txData[0] = reg_addr;             // Register address
    
    if(len == 3)
    {
        txData[1] = (value >> 8) & 0xFF;  // MSB of value
        txData[2] = value & 0xFF;         // LSB of value
    }
    
    else if(len == 2)
    {
        txData[1] = value & 0xFF; 
    }
    
    app_gridData.transferStatus = APP_GRID_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteTransferAdd(app_gridData.drvI2CHandle, GRID_SENSOR_CLIENT_ADDR,
                            (void *)&txData[0], len, 
                            &app_gridData.transferHandle );
    
    while (app_gridData.transferStatus != APP_GRID_I2C_TRANSFER_STATUS_SUCCESS);
    
    return true;  
}

// AMG8853 Initialization Function 
static bool APP_GRID_AMG8853_INIT (void)
{    
    //Power Mode Configuration
    if(APP_GRID_AMG8853_Write(GRID_SENSOR_POWER_MODE_REG, 
            GRID_SENSOR_POWER_MODE_VAL, 2) != true)
    {
        return false;
    }
    APP_GRID_DelayMS(1);
    
    // Sensor Reset
    if(APP_GRID_AMG8853_Write(GRID_SENSOR_RESET_REG, 
            GRID_SENSOR_RESET_VAL, 2) != true)
    {
        return false;
    }
    APP_GRID_DelayMS(1);
    
    // Frame Rate Configuration
    if(APP_GRID_AMG8853_Write(GRID_SENSOR_FRAME_RATE_VAL,
            GRID_SENSOR_FRAME_RATE_VAL, 2) != true)
    {
        return false;
    }
    APP_GRID_DelayMS(1);
    
    return true;
}


// AMG8853 Sensor Data Function 
uint16_t APP_GRID_AMG8853_Sensor_Data (void)
{
    uint8_t txdata = GRID_SENSOR_GRID_DATA_REG;
    uint8_t rxData[GRID_SENSOR_GRID_DATA_SIZE];  
    
    int temp_avg = 0;
    
    //Read the Grid Eye Pixels 
    app_gridData.transferStatus = APP_GRID_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteReadTransferAdd(app_gridData.drvI2CHandle, GRID_SENSOR_CLIENT_ADDR,
                                (void *)&txdata, 1, (void *)&rxData[0], GRID_SENSOR_GRID_DATA_SIZE, 
                                &app_gridData.transferHandle );
    
    while (app_gridData.transferStatus != APP_GRID_I2C_TRANSFER_STATUS_SUCCESS);
        
    for(int i=0; i<GRID_SENSOR_GRID_DATA_SIZE; i +=2)
    {
        temp_avg += ((rxData[i+1] << 8 | rxData[i]) & 0x7FF) * GRID_SENSOR_GRID_TEMP_COEFF;
    }    
   
    return temp_avg / GRID_SENSOR_GRID_PXL_SIZE;
} 

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_GRID_Initialize ( void )

  Remarks:
    See prototype in app_grid.h.
 */

void APP_GRID_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_gridData.state = APP_GRID_STATE_INIT;
    app_gridData.ready = false;
}


/******************************************************************************
  Function:
    void APP_GRID_Tasks ( void )

  Remarks:
    See prototype in app_grid.h.
 */

void APP_GRID_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_gridData.state )
    {
        /* Application's initial state. */
        case APP_GRID_STATE_INIT:
        {
            /* Open the I2C Driver */
            app_gridData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            
            if(app_gridData.drvI2CHandle == DRV_HANDLE_INVALID)
            {
                app_gridData.state = APP_GRID_STATE_ERROR;
            }
            
            else
            {
                /* Register the I2C Driver event Handler */
                DRV_I2C_TransferEventHandlerSet(
                                                app_gridData.drvI2CHandle,
                                                APP_GRID_I2C_Event_Handler,
                                                (uintptr_t)&app_gridData.transferStatus
                                                );
                
                app_gridData.state = APP_GRID_STATE_CONFIG;
            }
            break;
        }

        case APP_GRID_STATE_CONFIG:
        {
            if(APP_GRID_AMG8853_INIT() == true)
            {
                app_gridData.ready = true;
                app_gridData.state = APP_GRID_STATE_IDLE;
            }
            
            else
            {
                app_gridData.state = APP_GRID_STATE_ERROR;
            }
            break;
        }
        
        case APP_GRID_STATE_IDLE:
        {            
            break;
        }
        
        case APP_GRID_STATE_ERROR:
        {
            app_gridData.ready = false;
            app_gridData.state = APP_GRID_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_GRID_TASK: Task Error ...!");
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

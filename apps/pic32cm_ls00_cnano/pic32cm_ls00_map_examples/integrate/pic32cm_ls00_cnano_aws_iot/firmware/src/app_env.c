/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_env.c

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

#include "app_env.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// I2C CLIENT 4 Address
#define ENV_SENSOR_CLIENT_ADDR                  0x76

// Grid Eye Sensor Register Address
#define ENV_SENSOR_RESET_REG                    0xE0
#define ENV_SENSOR_CHIP_ID_REG                  0xD0
#define ENV_SENSOR_VAR_ID_REG                   0xF0
#define ENV_SENSOR_HUMI_CTRL_REG                0x72
#define ENV_SENSOR_MEAS_CTRL_REG                0x74
#define ENV_SENSOR_TEMP0_CALIB_REG              0xE9
#define ENV_SENSOR_TEMP1_CALIB_REG              0x8A
#define ENV_SENSOR_PRES_CALIB_REG               0x8E
#define ENV_SENSOR_HUMI_CALIB_REG               0xE1
#define ENV_SENSOR_DATA_REG                     0x1F

// Grid Eye Sensor Register Value
#define ENV_SENSOR_RESET_VAL                    0xB6
#define ENV_SENSOR_CHIP_ID_VAL                  0x61    
#define ENV_SENSOR_VAR_ID_VAL                   0x01    
#define ENV_SENSOR_HUMI_CTRL_VAL                0x01    // Humi = 1 x Oversampling
#define ENV_SENSOR_MEAS_CTRL_VAL                0x25    // Temp = 1 x Oversampling, Pres = 1 x Oversampling, Mode = Forced
#define ENV_SENSOR_TEMP0_CALIB_READ_VAL         0x02
#define ENV_SENSOR_TEMP1_CALIB_READ_VAL         0x03
#define ENV_SENSOR_PRES_CALIB_READ_VAL          0x13
#define ENV_SENSOR_HUMI_CALIB_READ_VAL          0x08
#define ENV_SENSOR_DATA_SIZE                    0x08

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_ENV_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_ENV_DATA app_envData;

// Temperature Calibration Co-efficient parameters
uint16_t par_t1;
uint16_t par_t2;
uint16_t par_t3;

// Pressure Calibration Co-efficient parameters
uint16_t par_p1;
uint16_t par_p2;
uint16_t par_p3;
uint16_t par_p4;
uint16_t par_p5;
uint16_t par_p6;
uint16_t par_p7;
uint16_t par_p8;
uint16_t par_p9;
uint16_t par_p10;

// Humidity Calibration Co-efficient parameters
uint16_t par_h1;
uint16_t par_h2;
uint16_t par_h3;
uint16_t par_h4;
uint16_t par_h5;
uint16_t par_h6;
uint16_t par_h7;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void APP_ENV_I2C_Event_Handler ( 
                                        DRV_I2C_TRANSFER_EVENT event,
                                        DRV_I2C_TRANSFER_HANDLE transferHandle,
                                        uintptr_t context
                                        )
{
    APP_ENV_I2C_TRANSFER_STATUS* transferStatus = (APP_ENV_I2C_TRANSFER_STATUS*)context;

    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE)
    {
        if (transferStatus)
        {
            *transferStatus = APP_ENV_I2C_TRANSFER_STATUS_SUCCESS;
        }
    }
    else
    {
        if (transferStatus)
        {
            *transferStatus = APP_ENV_I2C_TRANSFER_STATUS_ERROR;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

// BME688 Delay Function
static void APP_ENV_DelayMS (int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;
    
    while (SYS_TIME_DelayIsComplete(timer) == false);
}

// BME688 I2C Write Function
static bool APP_ENV_BME688_Write (uint8_t reg_addr, uint16_t value, uint8_t len) 
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
    
    app_envData.transferStatus = APP_ENV_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteTransferAdd(app_envData.drvI2CHandle, ENV_SENSOR_CLIENT_ADDR,
                            (void *)&txData[0], len, 
                            &app_envData.transferHandle );
    
    while (app_envData.transferStatus != APP_ENV_I2C_TRANSFER_STATUS_SUCCESS);
    
    return true;  
}


// BME688 I2C Write Read Function
static uint16_t APP_ENV_BME688_WriteRead (uint8_t reg_addr, uint8_t len) 
{ 
    uint8_t rxData;
    
    app_envData.transferStatus = APP_ENV_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteReadTransferAdd(app_envData.drvI2CHandle, ENV_SENSOR_CLIENT_ADDR,
                                (void *)&reg_addr, 1, (void *)&rxData, len, 
                                &app_envData.transferHandle );
    
    while (app_envData.transferStatus != APP_ENV_I2C_TRANSFER_STATUS_SUCCESS);

    return rxData;
}


// BME688 I2C Data Read Function
static bool APP_ENV_BME688_Data_Read (uint8_t reg_addr, uint8_t len) 
{ 
    memset(app_envData.rxData, 0, sizeof(app_envData.rxData));
            
    app_envData.transferStatus = APP_ENV_I2C_TRANSFER_STATUS_IN_PROGRESS;
    
    /* Add a request to write the application data */
    DRV_I2C_WriteReadTransferAdd(app_envData.drvI2CHandle, ENV_SENSOR_CLIENT_ADDR,
                                (void *)&reg_addr, 1, (void *)&app_envData.rxData, 
                                len, &app_envData.transferHandle );
    
    while (app_envData.transferStatus != APP_ENV_I2C_TRANSFER_STATUS_SUCCESS);

    return true;
}

// BME688 Initialization
static bool APP_ENV_BME688_INIT (void)
{     
    //Sensor Reset
    if(APP_ENV_BME688_Write(ENV_SENSOR_RESET_REG, ENV_SENSOR_RESET_VAL, 2) != true)
    {
        return false;
    }
    APP_ENV_DelayMS(1);    
    
    //Chip ID Check
    if(APP_ENV_BME688_WriteRead(ENV_SENSOR_CHIP_ID_REG, 1) != ENV_SENSOR_CHIP_ID_VAL)
    {
        return false;
    }
    APP_ENV_DelayMS(1);
    
    // Variant ID Check
    if(APP_ENV_BME688_WriteRead(ENV_SENSOR_VAR_ID_REG , 1) != ENV_SENSOR_VAR_ID_VAL)
    {
        return false;
    }
    APP_ENV_DelayMS(1);
    
    // Humidity Configuration
    if(APP_ENV_BME688_Write(ENV_SENSOR_HUMI_CTRL_REG, ENV_SENSOR_HUMI_CTRL_VAL, 2) != true)
    {
        return false;
    }
    APP_ENV_DelayMS(1);
    
    // Temperature, Pressure and Sampling Mode Configuration
    if(APP_ENV_BME688_Write(ENV_SENSOR_MEAS_CTRL_REG, ENV_SENSOR_MEAS_CTRL_VAL, 2) != true)
    {
        return false;
    }
    APP_ENV_DelayMS(1);
    
    return true;
}

// BME688 Temperature 0 Calibration Data Conversion Function 
static void APP_ENV_BME688_TEMP0_Calibration_Data_Conversion (void)
{     
    par_t1 = (app_envData.rxData[1] << 8) | app_envData.rxData[0];
}

// BME688 Temperature 1 Calibration Data Conversion Function 
static void APP_ENV_BME688_TEMP1_Calibration_Data_Conversion (void)
{     
    par_t2 = (app_envData.rxData[1] << 8) | app_envData.rxData[0];
    par_t3 = app_envData.rxData[2];
}

// BME688 Pressure Calibration Data Conversion Function 
static void APP_ENV_BME688_PRES_Calibration_Data_Conversion (void)
{     
    par_p1 = (app_envData.rxData[1] << 8) | app_envData.rxData[0];
    par_p2 = (app_envData.rxData[3] << 8) | app_envData.rxData[2];
    par_p3 = app_envData.rxData[4];
    par_p4 = (app_envData.rxData[7] << 8) | app_envData.rxData[6];
    par_p5 = (app_envData.rxData[9] << 8) | app_envData.rxData[8];
    par_p6 = app_envData.rxData[11];
    par_p7 = app_envData.rxData[10];
    par_p8 = (app_envData.rxData[15] << 8) | app_envData.rxData[14];
    par_p9 = (app_envData.rxData[17] << 8) | app_envData.rxData[16];
    par_p10 = app_envData.rxData[18];
}

// BME688 Humidity Calibration Data Conversion Function 
static void APP_ENV_BME688_HUMI_Calibration_Data_Conversion (void)
{     
    par_h1 = (app_envData.rxData[2] << 4) | (app_envData.rxData[0] & 0x0F);
    par_h2 = (app_envData.rxData[0] << 4) | ((app_envData.rxData[1] & 0xF0) >> 4);
    par_h3 = app_envData.rxData[3];
    par_h4 = app_envData.rxData[4];
    par_h5 = app_envData.rxData[5];
    par_h6 = app_envData.rxData[6];
    par_h7 = app_envData.rxData[7];
}

// BME688 Temperature Raw Data Conversion Function 
static float APP_ENV_BME688_TEMP_Data_Conversion (uint32_t temp_adc)
{
    float var1;
    float var2;

    var1 = ((((float ) temp_adc / 16384.0) - ((float) par_t1 / 1024.0)) *
                 (( float ) par_t2));
    
    var2 = (((((float) temp_adc / 131072.0) - ((float) par_t1 / 8192.0)) *
               (((float) temp_adc / 131072.0) - ((float) par_t1 / 8192.0))) * 
                 ((float) par_t3 * 16.0));

    return var1 + var2;
}

// BME688 Pressure Raw Data Conversion Function 
static float APP_ENV_BME688_PRES_Data_Conversion (uint32_t pres_adc, float t_fine)
{
    float var1 = 0;
    float var2 = 0;
    float var3 = 0;
    float calc_pres = 0;

    var1 = (float) (t_fine) / 2.0;
    var1 -= 64000.0;

    var2 = (float) (par_p6 ) / 131072.0;
    var2 *= var1 * var1;
    var2 += (var1 * ((float) par_p5) * 2.0);
    var2 = (var2 / 4.0) + (((float) par_p4) * 65536.0);

    var1 = (((((float) par_p3 * var1 * var1) / 16384.0) + 
               ((float) par_p2 * var1)) / 524288.0);
    var1 = ((1.0  + (var1 / 32768.0)) * ((float) par_p1));
    
    calc_pres = (1048576.0 - ((float) pres_adc));

    if (0 != (int16_t) var1)
    {
        calc_pres = (((calc_pres - (var2 / 4096.0)) * 6250.0) / var1);
        
        var1 = (((float) par_p9) * calc_pres * calc_pres) / 2147483648.0;
        
        var2 = calc_pres * (((float) par_p8) / 32768.0);
        
        var3 = (calc_pres / 256.0) * (calc_pres / 256.0) * 
               (calc_pres / 256.0) * (par_p10 / 131072.0);
        
        calc_pres = calc_pres + (var1 + var2 + var3 + ((float) par_p7 * 128.0)) / 16.0;
    }
    else
    {
        calc_pres = 0;
    }

    return calc_pres;
    
}

// BME688 Humidity Raw Data Conversion Function 
static float APP_ENV_BME688_HUMI_Data_Conversion (uint32_t humi_adc, float t_fine)
{
    float var1;
    float var2;
    float var3;
    float var4;
    float temp_scaled;
    float calc_hum;

    temp_scaled = t_fine / 5120.0;
    
    var1 = (float) humi_adc - (((float) par_h1 * 16.0) + 
                               (((float) par_h3 / 2.0) * temp_scaled));
    
    var2 = var1 * ((float)(((float) par_h2 / 262144.0) * 
                              (1.0 + (((float) par_h4 / 16384.0) * temp_scaled) + 
                              (((float) par_h5 / 1048576.0) * temp_scaled * temp_scaled))));
    
    var3 = (float) par_h6 / 16384.0;
    
    var4 = (float) par_h7 / 2097152.0;
    
    calc_hum = var2 + ((var3 + (var4 * temp_scaled)) * var2 * var2);

    if (calc_hum > 100.0)
    {
        calc_hum = 100.0;
    }
    
    else if (calc_hum < 0.0)
    {
        calc_hum = 0.0;
    }
    
    return calc_hum;
}
    

// BME688 Sensor Status Function
bool APP_ENV_TaskIsReady (void)
{    
    uint32_t temp_adc;
    uint32_t pres_adc;
    uint16_t humi_adc;
    
    if(app_envData.ready == false)
    {
        return false;
    }
    
     // Temperature, Pressure and Sampling Mode Configuration
    if(APP_ENV_BME688_Write(ENV_SENSOR_MEAS_CTRL_REG, ENV_SENSOR_MEAS_CTRL_VAL, 2) != true)
    {
        return false;
    }
    APP_ENV_DelayMS(1);
    
    if(APP_ENV_BME688_Data_Read(ENV_SENSOR_DATA_REG, ENV_SENSOR_DATA_SIZE) != true)
    {
        return false;
    }
    
    pres_adc = (app_envData.rxData[0] << 12) | (app_envData.rxData[1] << 4) |
                 (app_envData.rxData[2] >> 4);
    
    temp_adc = (app_envData.rxData[3] << 12) | (app_envData.rxData[4] << 4) |
                 (app_envData.rxData[5] >> 4);
    
    humi_adc = (app_envData.rxData[6] << 8) | app_envData.rxData[7];    
    
    app_envData.temperature = APP_ENV_BME688_TEMP_Data_Conversion(temp_adc); 
    app_envData.pressure = APP_ENV_BME688_PRES_Data_Conversion(pres_adc, app_envData.temperature);
    app_envData.humidity = APP_ENV_BME688_HUMI_Data_Conversion(humi_adc, app_envData.temperature);
    
    return true;
}    

// BME688 Pressure Sensor Data Function
float APP_ENV_BME688_Pressure_Sensor_Data (void)
{
    return app_envData.pressure / 100;
}

// BME688 Humidity Sensor Data Function
float APP_ENV_BME688_Humidity_Sensor_Data (void)
{
    return app_envData.humidity;
}
    


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_ENV_Initialize ( void )

  Remarks:
    See prototype in app_env.h.
 */

void APP_ENV_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_envData.state = APP_ENV_STATE_INIT;
    app_envData.ready = false;
}

/******************************************************************************
  Function:
    void APP_ENV_Tasks ( void )

  Remarks:
    See prototype in app_env.h.
 */

void APP_ENV_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_envData.state )
    {
        /* Application's initial state. */
        case APP_ENV_STATE_INIT:
        {
            /* Open the I2C Driver */
            app_envData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            
            if(app_envData.drvI2CHandle == DRV_HANDLE_INVALID)
            {
                app_envData.state = APP_ENV_STATE_ERROR;
            }
            
            else
            {
                /* Register the I2C Driver event Handler */
                DRV_I2C_TransferEventHandlerSet(
                                                app_envData.drvI2CHandle,
                                                APP_ENV_I2C_Event_Handler,
                                                (uintptr_t)&app_envData.transferStatus
                                                );
                
                app_envData.state = APP_ENV_STATE_CONFIG;
            }
            break;
        }

        case APP_ENV_STATE_CONFIG:
        {
            if(APP_ENV_BME688_INIT() == true)
            {
                app_envData.state = APP_ENV_STATE_TEMP_CALIB;
            }
            
            else
            {
                app_envData.state = APP_ENV_STATE_ERROR;
            }
            break;
        }
        
        case APP_ENV_STATE_TEMP_CALIB:
        {
            if(APP_ENV_BME688_Data_Read(ENV_SENSOR_TEMP0_CALIB_REG, 
                    ENV_SENSOR_TEMP0_CALIB_READ_VAL) == true)
            {
                APP_ENV_BME688_TEMP0_Calibration_Data_Conversion();
                        
                if(APP_ENV_BME688_Data_Read(ENV_SENSOR_TEMP1_CALIB_REG,
                        ENV_SENSOR_TEMP1_CALIB_READ_VAL) == true)
                {
                    APP_ENV_BME688_TEMP1_Calibration_Data_Conversion();
                    
                    app_envData.state = APP_ENV_STATE_PRES_CALIB;
                }
            }
            
            else
            {
                app_envData.state = APP_ENV_STATE_ERROR;
            }
            break;
        }
                
                
        case APP_ENV_STATE_PRES_CALIB:
        {
            if(APP_ENV_BME688_Data_Read(ENV_SENSOR_PRES_CALIB_REG, 
                    ENV_SENSOR_PRES_CALIB_READ_VAL) == true)
            {
                APP_ENV_BME688_PRES_Calibration_Data_Conversion();
                        
                app_envData.state = APP_ENV_STATE_HUMI_CALIB;
            }
            
            else
            {
                app_envData.state = APP_ENV_STATE_ERROR;
            }
            break;
        }
                        
                        
        case APP_ENV_STATE_HUMI_CALIB:
        {
            if(APP_ENV_BME688_Data_Read(ENV_SENSOR_HUMI_CALIB_REG, 
                    ENV_SENSOR_HUMI_CALIB_READ_VAL) == true)
            {
                APP_ENV_BME688_HUMI_Calibration_Data_Conversion();
                
                app_envData.state = APP_ENV_STATE_IDLE;
                app_envData.ready = true;
            }
            
            else
            {
                app_envData.state = APP_ENV_STATE_ERROR;
            }
            break;
        }
        
        case APP_ENV_STATE_IDLE:
        {                
            break;
        }
        
        case APP_ENV_STATE_ERROR:
        {
            app_envData.ready = false;
            app_envData.state = APP_ENV_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_ENV_TASK: Task Error ...!");
            APP_ENV_DelayMS(500);
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

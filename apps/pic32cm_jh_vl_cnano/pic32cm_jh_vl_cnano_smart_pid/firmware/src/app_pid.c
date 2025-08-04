/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pid.c

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
#include "app_pid.h"
#include "definitions.h" 
#include "app_temp.h" 
#include "app_aws.h"
#include "app_pid.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define SET_TEMPERATURE    24.0f

// PID control variables for temperature
volatile float Error_temp = 0;
volatile float Error_temp_prev = 0;
volatile float PID_output = 0.0f;
volatile float pid_i_temp = 0;
volatile float pid_d_temp = 0;

// PID gain settings for temperature control
float pid_p_gain_temp = 40.0f;     // Proportional gain for temperature
float pid_i_gain_temp = 0.004f;     // Integral gain for temperature
float pid_d_gain_temp = 0.5f;     // Derivative gain for temperature

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_PID_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_PID_DATA app_pidData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

static void PID_TimerCallback()
{
    app_pidData.tmrExpired = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary local functions.
*/

void APP_PID_Initialize ( void )
{
    app_pidData.state = APP_PID_STATE_INIT;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PID_Initialize ( void )

  Remarks:
    See prototype in app_pid.h.
 */

void APP_PID_Tasks ( void )
{
    switch ( app_pidData.state )
    {
        case APP_PID_STATE_INIT:
        {
            TC3_CompareStart();

            app_pidData.tmrHandle = SYS_TIME_CallbackRegisterMS(PID_TimerCallback, 0,
                100, SYS_TIME_PERIODIC);

            app_pidData.state = APP_PID_STATE_SERVICE_TASKS;

            break;
        }

        case APP_PID_STATE_SERVICE_TASKS:
        {
            if(app_pidData.tmrExpired == true)
            {
                app_pidData.tmrExpired = false;                
                        
                // Set setpoint based on mode and speed
                if(APP_AWS_GetOperatingMode() == true)
                {
                    // Calculate error
                    Error_temp = APP_TEMP_HTU21D_Get_Temp() - SET_TEMPERATURE;
                
                    // Integral term calculation
                    pid_i_temp += Error_temp;

                    // Derivative term calculation
                    pid_d_temp = Error_temp - Error_temp_prev;

                    // PID output calculation
                    PID_output = (int)((pid_p_gain_temp * Error_temp) +
                                     (pid_i_gain_temp * pid_i_temp) +
                                     (pid_d_gain_temp * pid_d_temp));

                    // Clamp PID output to maximum limits
                    if (PID_output > 199)
                        PID_output = PWM_DUTY_MAX * 0.90;
                    else if (PID_output < 0)
                        PID_output = 0;
                    
                    // Store current error for next derivative calculation
                    Error_temp_prev = Error_temp;

                    TC3_Compare16bitMatch1Set(PID_output);
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

/*
End of File
*/

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sec.c

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
// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "app_sec.h"
#include "click_routines/heartrate9/heartrate9.h"
#include "click_routines/click_interface.h"
#include "click_routines/rnbd451/rnbd451.h"

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
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

const   char DevName[]          = "PIC32CM_LS00_HR";
        uint8_t service_uuid    = 0xC0;
        bool Err;
        int8_t heart_rate;
        char HR_data[2] = "";
        uint8_t resetMode;

typedef enum
{
    /* TODO: Define states used by the application state machine. */
    RNBD_INIT,
    RNBD_FACTORY_RESET,
    RNBD_CMD,
    RNBD_CMD1,
    RNBD_CMD2,
    RNBD_CMD3,
    RNBD_SET_NAME,
    RNBD_SET_PROFILE,
    RNBD_SET_APPEARANCE,
    RNBD_REBOOT,
    RNBD_REBOOT1,
    RNBD_SERVICE_UUID,
    RNBD_SERVICE_CHARACTERISTICS,
    RNBD_SERVICE_CHARACTERISTICS1,
    RNBD_SERVICE_CHARACTERISTICS2,
} STATES;

typedef enum
{
  RNBD_SEND_HR_DATA,
  RNBD_DISPLAY_HR_DATA,
  RNBD_SEND_HRBS,
  RNBD_WAIT,
} SEND_DATA;

typedef struct
{
    /* The application's current state */
    STATES state;
} RNBD_STATE;

RNBD_STATE rnbd_state;

typedef struct
{
    SEND_DATA data;
} RNBD_DATA;

RNBD_DATA rnbd_data;

APP_DATA        appData;

volatile bool button_pressed        = false;
volatile bool readHeartRateStatus   = false;
volatile bool get_rnbd_init_state   = false;
volatile bool set_name_status       = false;
         char sec_lcl_buffer[10]    = {0};

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static int8_t heartrate9_example(void)
{
    int8_t heartrate_data = -1;
    // Check the Heartrate Ready Status

    if(true == is_heartrate9_byte_ready())
    {
        // Return the Heartrate
        while(heartrate_data == -1)
        {
            heartrate_data = heartrate9_read_byte();
        }
        printf("Heartrate = %d bpm \t\r\n", (uint8_t)heartrate_data);
    }
    return heartrate_data;
}

static void EIC_Button_Handler(uintptr_t context)
{
    button_pressed = true;
}

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
void APP_Initialize ( void )
{
    readHeartRateStatus = false;
    rnbd_state.state    = RNBD_INIT;
    appData.app_state   = APP_RNBD_INIT_STATE;

    EIC_CallbackRegister(EIC_PIN_2, EIC_Button_Handler, 0);
}

void RNBD_heartrate_example_initialize(void)
{
    switch(rnbd_state.state)
    {
        case RNBD_INIT:
        {
            Err= rnbd451_init();
            if(Err)
            {
                Err=false;
                printf("RNBD451_INITIALIZING\r\n");
                rnbd_state.state=RNBD_CMD;
            }
        }
        case RNBD_CMD:
        {
            Err=rnbd451_enter_cmd_mode();
            if(Err)
            {
                Err=false;
                printf("Entered CMD Mode\r\n");
                rnbd_state.state=RNBD_FACTORY_RESET;
            }
        }
        case RNBD_FACTORY_RESET:
        {
            Err= rnbd451_factory_reset(resetMode);
            if(Err)
            {
                Err=false;
                printf("Factory Reset Done\r\n");
                rnbd_state.state=RNBD_CMD1;
            }
        }
        case RNBD_SET_NAME:
        {
            Err=rnbd451_set_name(DevName,strlen(DevName));
            if(Err)
            {
                Err=false;
                set_name_status = true;
                printf("Device Name Set\r\n");
                rnbd_state.state=RNBD_SET_PROFILE;
            }
            else
            {
                set_name_status = false;
            }
        }
        case RNBD_CMD1:
        {
            Err=rnbd451_enter_cmd_mode();
            if(Err)
            {
                Err=false;
                printf("Entered CMD Mode\r\n");
                rnbd_state.state=RNBD_SET_NAME;
            }
        }
        case RNBD_SET_PROFILE:
        {
            Err=rnbd451_set_service_bitmap(service_uuid);
            if(Err)
            {
                Err=false;
                printf("Service Bitmap Set\r\n");
                rnbd_state.state=RNBD_SET_APPEARANCE;
            }
        }
        case RNBD_SET_APPEARANCE:
        {
            char appearance[]="0340";
            Err=rnbd451_set_appearance(appearance,strlen(appearance));
            if(Err)
            {
                Err=false;
                printf("Appearance Set\r\n");
                rnbd_state.state=RNBD_REBOOT;
            }
        }
        case RNBD_REBOOT:
        {
            Err=rnbd451_reboot_cmd();
            if(Err)
            {
                Err=false;
                printf("Reboot Completed\r\n");
                rnbd_state.state=RNBD_CMD2;
            }
        }
        case RNBD_CMD2:
        {
            Err=rnbd451_enter_cmd_mode();
            if(Err)
            {
                Err=false;
                printf("Entered CMD Mode\r\n");
                rnbd_state.state=RNBD_SERVICE_UUID;
            }
        }
        case RNBD_SERVICE_UUID:
        {
            char PS[]="180D";
            Err=rnbd451_set_service_uuid(PS,strlen(PS));
            if(Err)
            {
                Err=false;
                printf("Service UUID Set\r\n");
                rnbd_state.state=RNBD_SERVICE_CHARACTERISTICS;
            }
        }
        case RNBD_SERVICE_CHARACTERISTICS:
        {
            char PC[]="2A37,10,05";
            Err=rnbd451_set_service_characteristic(PC,strlen(PC));
            if(Err)
            {
                Err=false;
                printf("Service Characteristics Set\r\n");
                rnbd_state.state=RNBD_SERVICE_CHARACTERISTICS1;
            }
        }
        case RNBD_SERVICE_CHARACTERISTICS1:
        {
            char PC1[]="2A38,02,05";
            Err=rnbd451_set_service_characteristic(PC1,strlen(PC1));
            if(Err)
            {
                Err=false;
                printf("Service Characteristics Set\r\n");
                rnbd_state.state=RNBD_SERVICE_CHARACTERISTICS2;
            }
        }
        case RNBD_SERVICE_CHARACTERISTICS2:
        {
            char PC2[]="2A39,08,05";
            Err=rnbd451_set_service_characteristic(PC2,strlen(PC2));
            if(Err)
            {
                Err=false;
                printf("Service Characteristics Set\r\n");
                rnbd_state.state=RNBD_REBOOT1;
            }
        }
        case RNBD_REBOOT1:
        {
            Err=rnbd451_reboot_cmd();
            if(Err)
            {
                Err=false;
                printf("Reboot Completed\r\n");
                rnbd_state.state=RNBD_CMD3;
            }
        }
        case RNBD_CMD3:
        {
            Err=rnbd451_enter_cmd_mode();
            if(Err)
            {
                Err=false;
                printf("Entered CMD Mode\r\n");
                // rnbd_state.state=RNBD_SEND_HRBS;
            }
        }
    }
}

void RNBD_heartrate_example(void)
{
    switch(rnbd_data.data)
    {
        case RNBD_SEND_HRBS:
            {
                char handle[]="1005,03";
                Err= rnbd451_write_local_characteristic(handle,strlen(handle),0,0);
                if(Err)
                {
                Err=false;
                printf("HRBS Set\r\n");
                printf("Press the User Button to Start Measuring the Heart Rate\r\n");
                rnbd_data.data=RNBD_WAIT;
                }
            }

        case RNBD_WAIT:
        {
            Err=false;

            if(button_pressed == true)
            {
                button_pressed = false;
                printf("!!! Measuring Heart Rate !!!\r\n");
                heart_rate = heartrate9_example();
                if(heart_rate != -1)
                {
                    rnbd_data.data= RNBD_DISPLAY_HR_DATA;
                }
                else
                {
                    rnbd_data.data=RNBD_WAIT;
                }
            }
            else
            {
               rnbd_data.data=RNBD_WAIT;
            }
        }

        case RNBD_DISPLAY_HR_DATA:
        {
            printf("Heartrate = %d bpm \t\r\n", (uint8_t)heart_rate);
            sprintf(sec_lcl_buffer, "%dbpm", (uint8_t)heart_rate);
            // Read Heart Rate info back to Non-Secure world
            readHeartRateStatus = true;
            if(true)
            {
                rnbd_state.state=RNBD_SEND_HR_DATA;
            }
        }
        
        case RNBD_SEND_HR_DATA:
        {
            char handle[] = "1002";
            sprintf(HR_data,"%x",heart_rate);
            Err=rnbd451_write_local_characteristic(handle,strlen(handle),HR_data,2);
            if(Err)
            {
            Err = false;
            rnbd_data.data=RNBD_WAIT;
            }
        }
    }
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
void secureApp ( void )
{
    /* Check the application's current state. */
    switch ( appData.app_state )
    {
        case APP_RNBD_INIT_STATE:
            if (get_rnbd_init_state ==  false)
            {
                RNBD_heartrate_example_initialize();
                appData.app_state = APP_IDLE_STATE;
            }
            else
            {

            }
            break;

        case CHKNG_HEART_RATE_DATA_RDY_STATE:
            if(true)  // Checking the Heart Rate sensor data ready Status
            {
                appData.app_state       = HEART_RATE_SENSOR_DATA_PROSS_STATE;
            }
            break;

        case HEART_RATE_SENSOR_DATA_PROSS_STATE:
            if( true )             // Checking the Heartrate sensor data process completed
            {
                rnbd_data.data = RNBD_SEND_HRBS;
                RNBD_heartrate_example();
                appData.app_state       = EINK_EPAPER_2_9_296_128_UPDATE_STATE;
            }
            break;

        case EINK_EPAPER_2_9_296_128_UPDATE_STATE:

            button_pressed              = false;

            appData.app_state           = APP_IDLE_STATE;
            break;

        case APP_IDLE_STATE:
            if (appData.queryDelay == 0)
            {
                appData.queryDelay = QUERY_DELAY;
            }
            if(button_pressed == true)
            {
                appData.app_state       = CHKNG_HEART_RATE_DATA_RDY_STATE;
            }
            break;

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

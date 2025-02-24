/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_aws.c

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

#include "app_aws.h"
#include <stdio.h>
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/sys_rnwf_system_service.h"
#include "system/mqtt/sys_rnwf_mqtt_service.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/* MQTT Connection States */
#define MQTT_DISCONNECTED       0
#define MQTT_CONNECTING         1
#define MQTT_CONNECTED          2

#define SYS_RNWF_AWS_FMT_TEMP_LIGHT      "{\\\"Temperature (C)\\\": %d,\\\"Light (lux)\\\":%d,\\\"Motion \\\":%d,\\\"Grid Eye Temp (C)\\\":%d,\\\"Pressure (hP)\\\":%.2f,\\\"Humidity (Per)\\\":%.2f}"
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_AWS_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_AWS_DATA app_awsData;

APP_AWS_CLOUD_STATES app_awsCloud_state;


uint8_t isMqttConnected = MQTT_DISCONNECTED;

static int16_t input1 = 0;
static uint16_t input2 = 0;
static uint16_t input3 = 0;
static uint16_t input4 = 0;
static float input5 = 0;
static float input6 = 0;

/* Keeps the device IP address */
static uint8_t g_devIp[64];

/* Variable to check the UART transfer */
static volatile bool isUARTTxComplete = true, isUART0TxComplete = true;

/*Application buffer to store data*/
static uint8_t appBuf[SYS_RNWF_BUF_LEN_MAX];
      
/**Aws IoT HUB subscribe list */
static const char *subscribeList[] = {SYS_RNWF_MQTT_SUB_TOPIC_0,  NULL};

/*Aws subscribe count*/
static uint8_t subCnt;

/**TLS Configuration for the aws */
const char *tlsCfgAws[] = 
                        {
                            SYS_MQTT_ENABLE_PEER_AUTH,
                            SYS_RNWF_MQTT_SERVER_CERT, 
                            SYS_RNWF_MQTT_DEVICE_CERT, 
                            SYS_RNWF_MQTT_DEVICE_KEY,                                     
                            NULL,                                    
                            SYS_RNWF_MQTT_TLS_SERVER_NAME,
                            SYS_MQTT_DOMAIN_NAME_VERIFY,
                            NULL
                        };
   

/*MQTT Configurations for aws*/
SYS_RNWF_MQTT_CFG_t mqtt_cfg = 
                        {
                            .url = SYS_RNWF_MQTT_CLOUD_URL,
                            .username = "/registrations/"SYS_RNWF_MQTT_CLIENT_ID"/api-version=2021-04-12",    
                            .clientid = SYS_RNWF_MQTT_CLIENT_ID,    
                            .password = SYS_RNWF_MQTT_PASSWORD,
                            .port = SYS_RNWF_MQTT_CLOUD_PORT,
                            .tls_conf = (uint8_t *) tlsCfgAws,
                            .tls_idx = SYS_RNWF_NET_TLS_CONFIG_2,
                            .azure_dps = SYS_RNWF_MQTT_AZURE_DPS_ENABLE,
                            .protoVer  = SYS_RNWF_MQTT_VERSION,
                            .keep_alive_time = SYS_RNWF_MQTT_KEEP_ALIVE_INT,
                        };

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/*Aws subscribe acknowledgment handler*/
static void APP_RNWF_AWS_SubackHandler()
{      
    if(subscribeList[subCnt] != NULL)
    {
        sprintf((char *)appBuf, "%s", subscribeList[subCnt++]);
        
        SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS, appBuf);            
    }   
}

/*Aws subscribe handler function*/
void SYS_RNWF_APP_AWS_SUB_Handler(char *p_str)
{    
    if(strstr(p_str, "shadow/update/delta"))
    {        
         
    }
}

/*function to get IP address*/
uint8_t *APP_GET_IP_Address(void)
{
    return g_devIp;
}

/* Application MQTT Callback Handler function */
SYS_RNWF_RESULT_t APP_MQTT_Callback(SYS_RNWF_MQTT_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        /* MQTT connected event code*/
        case SYS_RNWF_MQTT_CONNECTED:
        {                           
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: MQTT Connected");
                        
            app_awsCloud_state = APP_AWS_CLOUD_STATE_UP;
            isMqttConnected = MQTT_CONNECTED;
            
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Press touch button to start publishing");
            
            RED_LED_Clear();
            GREEN_LED_Set();
            
            break;
        }
        
        /* MQTT Subscribe acknowledge event code*/
        case SYS_RNWF_MQTT_SUBCRIBE_ACK:
        {
            APP_RNWF_AWS_SubackHandler();
            break;
        }
        
        /* MQTT Subscribe message event code*/
        case SYS_RNWF_MQTT_SUBCRIBE_MSG:
        {
            SYS_RNWF_APP_AWS_SUB_Handler((char *)p_str);
            break;
        }
        
        /*MQTT Disconnected event code*/
        case SYS_RNWF_MQTT_DISCONNECTED:
        {            
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: MQTT - Reconnecting...");
            isMqttConnected = MQTT_DISCONNECTED;
            
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL); 
            isMqttConnected = MQTT_CONNECTING;   
            
            app_awsData.touchInput = false;
            
            RED_LED_Set();
            GREEN_LED_Clear();
            
            break;
        }
               
        default:
        {
            break;
        }
    }
    
    return SYS_RNWF_PASS;
}

/* Application Wi-fi Callback Handler function */
void APP_WIFI_Callback(SYS_RNWF_WIFI_EVENT_t event, uint8_t *p_str)
{            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_RNWF_WIFI_SNTP_UP:
        {       
            
            if(isMqttConnected < MQTT_CONNECTING)
            {                        
                SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Connecting to the Cloud");
                
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SET_CALLBACK, APP_MQTT_Callback);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONFIG, (void *)&mqtt_cfg);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);
                
                isMqttConnected = MQTT_CONNECTING;
            }
                
            break;
        }
        
        /* Wi-Fi connected event code*/
        case SYS_RNWF_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Wi-Fi Connected");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_RNWF_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Wi-Fi Disconnected "
                    "\n\r APP_AWS_TASK: Reconnecting... ");
            
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV4_COMPLETE:
        {
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: DHCP IP:%s", &p_str[2]);
            
            strncpy((char *)g_devIp,(const char *) &p_str[3], 
                    strlen((const char *)(&p_str[3]))-1);
            
            break;
        }
        
        /* Wi-Fi scan indication event code*/
        case SYS_RNWF_WIFI_SCAN_INDICATION:
        {
            break;
        }
        
        /* Wi-Fi scan complete event code*/
        case SYS_RNWF_WIFI_SCAN_DONE:
        {
            break;
        }
        
        default:
        {
            break;
        }
                    
    }    
}

/* DMAC Channel Handler Function */
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUARTTxComplete = true;
    }
}

static void Timer_Callback ( uintptr_t context )
{
    touch_process();

    if (0u != (get_sensor_state(0) & 0x80))
    {
        LED_Set();
        app_awsData.touchInput = true;
    }

    else
    {
        LED_Clear();
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*MQTT data publish function*/
static SYS_RNWF_RESULT_t APP_RNWF_mqttPublish(const char *top, const char *msg)
{    
    SYS_RNWF_MQTT_FRAME_t mqtt_pub; 
    
    mqtt_pub.isNew      = SYS_RNWF_NEW_MSG;
    mqtt_pub.qos        = SYS_RNWF_MQTT_QOS0;
    mqtt_pub.isRetain   = SYS_RNWF_NO_RETAIN;
    mqtt_pub.topic      = top;
    mqtt_pub.message    = msg;
    
    return SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_PUBLISH, (void *)&mqtt_pub);              
} 

/*Function to send the button press count data*/
static void APP_RNWF_AWS_Telemetry(int16_t sensorInput1, uint16_t sensorInput2,
                                    uint16_t sensorInput3, uint16_t sensorInput4,
                                    float sensorInput5, float sensorInput6)
{            
    snprintf((char *)appBuf, sizeof(appBuf), SYS_RNWF_AWS_FMT_TEMP_LIGHT, 
            sensorInput1, sensorInput2, sensorInput3,
            sensorInput4, sensorInput5, sensorInput6);
    
    SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Sensor_1: %d, Sensor_2: %d"
                                        " Sensor_3: %d, Sensor_4: %d"
                                        " Sensor_5: %.2f, Sensor_6: %.2f",           
                                sensorInput1, sensorInput2, sensorInput3, 
                                sensorInput4, sensorInput5, sensorInput6);
    
    APP_RNWF_mqttPublish((const char *)SYS_RNWF_MQTT_PUB_TOPIC_NAME,(const char *) appBuf);
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************


/*******************************************************************************
  Function:
    void APP_AWS_Initialize ( void )

  Remarks:
    See prototype in app_aws.h.
 */

void APP_AWS_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_awsData.state       = APP_AWS_STATE_INIT;  
    app_awsData.touchInput  = false;
    app_awsData.tmrHandle   = SYS_TIME_HANDLE_INVALID;
}


/******************************************************************************
  Function:
    void APP_AWS_Tasks ( void )

  Remarks:
    See prototype in app_aws.h.
 */

void APP_AWS_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_awsData.state )
    {
        /* Application's initial state. */
        case APP_AWS_STATE_INIT:
        {
            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
            SYS_RNWF_IF_Init();
            
            char sntp_url[] =  "0.in.pool.ntp.org";    
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SET_SNTP,sntp_url);
                          
            /* RNWF Application Callback register */
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, APP_WIFI_Callback);
          
            /* Wi-Fi Connectivity */
            SYS_RNWF_WIFI_PARAM_t wifi_sta_cfg = {SYS_RNWF_WIFI_MODE_STA, 
                SYS_RNWF_WIFI_STA_SSID, SYS_RNWF_WIFI_STA_PWD, SYS_RNWF_STA_SECURITY, 
                SYS_RNWF_WIFI_STA_AUTOCONNECT};        
            
            
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Connecting to %s", SYS_RNWF_WIFI_STA_SSID);
            
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_SET_WIFI_PARAMS, &wifi_sta_cfg);
            
            RED_LED_Set();
            GREEN_LED_Clear();
          
            app_awsData.state = APP_AWS_STATE_USER_INPUT;
            break;
        }
        
        case APP_AWS_STATE_USER_INPUT:
        {            
            app_awsData.tmrHandle = SYS_TIME_CallbackRegisterMS(Timer_Callback, 0, 
                    100, SYS_TIME_PERIODIC);
            
            app_awsData.state = APP_AWS_STATE_SERVICE_TASKS;
            break;
        }

        case APP_AWS_STATE_SERVICE_TASKS:
        {           
            if ((isMqttConnected == MQTT_CONNECTED) && (app_awsData.touchInput == true))
            {
                if(APP_TEMP_TaskIsReady() == true)
                {
                    // Read the temperature sensor data 
                    input1 = APP_TEMP_MCP9808_Sensor_Data();
                }
                
                if(APP_LIGHT_TaskIsReady() == true)
                {
                    // Read the light sensor data
                    input2  = APP_LIGHT_VCNL4200_Sensor_Data();
                }
                
                if(APP_PIR_TaskIsReady() == true)
                {
                    // Read the PIR sensor data
                    input3  = APP_PIR_MCP3221_Sensor_Data();
                }
                
                if(APP_GRID_TaskIsReady() == true)
                {
                    // Read the gird eye sensor data
                    input4  = APP_GRID_AMG8853_Sensor_Data();
                }
                
                if(APP_ENV_TaskIsReady() == true)
                {
                    // Read the pressure sensor data
                    input5  = APP_ENV_BME688_Pressure_Sensor_Data();
                    
                    // Read the humidity sensor data
                    input6  = APP_ENV_BME688_Humidity_Sensor_Data();
                }
                  
                // Update it to the AWS cloud
                APP_RNWF_AWS_Telemetry(input1, input2, input3,
                                    input4, input5, input6); 
            }           
            
            SYS_RNWF_IF_EventHandler(); 
            break;
        }
        
        
        case APP_AWS_STATE_IDLE:
        {            
            break;
        }

        case APP_AWS_STATE_ERROR:
        {
            app_awsData.state = APP_AWS_STATE_IDLE;
            
            SYS_CONSOLE_PRINT("\n\r APP_AWS_TASK: Task Error ...!");
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

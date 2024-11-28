/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************


#define SYS_RNWF_NET_BIND_TYPE0					SYS_RNWF_BIND_REMOTE 
#define SYS_RNWF_NET_SOCK_TYPE0					SYS_RNWF_SOCK_TCP 
#define SYS_RNWF_NET_SOCK_TYPE_IPv4_0           4
#define SYS_RNWF_NET_SOCK_TYPE_IPv6_LOCAL0      0
#define SYS_RNWF_NET_SOCK_TYPE_IPv6_GLOBAL0     0
#define SYS_RNWF_NET_SOCK_ADDR0                 ""
#define SYS_RNWF_NET_SOCK_PORT0					80
#define SYS_RNWF_TLS_ENABLE0					0



#define SYS_RNWF_NET_SockCallbackHandler  	    APP_SOCKET_Callback

#define SYS_RNWF_MQTT_CLOUD_URL        		    "a1iq91fd6ih7tv-ats.iot.us-east-2.amazonaws.com"
#define SYS_RNWF_MQTT_CLOUD_PORT			    8883
#define SYS_RNWF_MQTT_AZURE_DPS_ENABLE			    0
#define SYS_RNWF_MQTT_CLOUD_USER_NAME			"arn:aws:iot:us-east-2:964716481709:thing/pic32c_aws_iot"
#define SYS_RNWF_MQTT_PASSWORD				    ""
#define SYS_RNWF_MQTT_CLIENT_ID       			"pic32c_aws_iot"
#define SYS_RNWF_MQTT_VERSION                   3
#define SYS_RNWF_MQTT_TOPIC_NAME        	    "/aws_iot_pic32c/"
#define SYS_RNWF_MQTT_QOS_TYPE					SYS_RNWF_MQTT_QOS0
#define SYS_RNWF_MQTT_RETAIN_TYPE				SYS_RNWF_NO_RETAIN
#define SYS_RNWF_MQTT_KEEP_ALIVE_INT            60
#define SYS_RNWF_MQTT_SUB_TOPIC_0            ""
#define SYS_RNWF_MQTT_SUB_QOS_TYPE           "QOS0"

#define SYS_MQTT_AZURE_ENABLE_PEER_AUTH       "1"
#define SYS_RNWF_MQTT_SERVER_CERT            "AmazonRootCA1"

#define SYS_RNWF_MQTT_DEVICE_CERT            "aws_iot-certificate"
#define SYS_RNWF_MQTT_DEVICE_KEY             "aws_iot-private"
#define SYS_RNWF_MQTT_DEVICE_KEY_PSK         NULL

#define SYS_RNWF_MQTT_TLS_SERVER_NAME        "a1iq91fd6ih7tv-ats.iot.us-east-2.amazonaws.com"
#define SYS_MQTT_AZURE_DOMAIN_NAME_VERIFY    NULL
#define SYS_RNWF_MQTT_CallbackHandler           APP_MQTT_Callback
/* TIME System Service Configuration Options */
#define SYS_TIME_INDEX_0                            (0)
#define SYS_TIME_MAX_TIMERS                         (10)
#define SYS_TIME_HW_COUNTER_WIDTH                   (24)
#define SYS_TIME_TICK_FREQ_IN_HZ                    (1000)


#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL       SYS_ERROR_DEBUG
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE


#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			(1U)
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			(1U)
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		(0U)
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		(200U)


#define SYS_CONSOLE_INDEX_0                       0






// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/* I2C Driver Instance 0 Configuration Options */
#define DRV_I2C_INDEX_0                       0
#define DRV_I2C_CLIENTS_NUMBER_IDX0           5
#define DRV_I2C_QUEUE_SIZE_IDX0               10
#define DRV_I2C_CLOCK_SPEED_IDX0              100

/* I2C Driver Common Configuration Options */
#define DRV_I2C_INSTANCES_NUMBER              (1U)




// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************
/* RN WIFI System Service Configuration Options */

#define RNWF_WIFI_DEVMODE        			SYS_RNWF_WIFI_MODE_STA

#define SYS_RNWF_WIFI_STA_SSID				"DEMO_AP"
#define SYS_RNWF_WIFI_STA_PWD        		"password"
#define SYS_RNWF_STA_SECURITY				SYS_RNWF_WPA2 
#define SYS_RNWF_WIFI_STA_AUTOCONNECT   		true


#define SYS_RNWF_COUNTRYCODE                "USA"
#define SYS_RNWF_SNTP_ADDRESS               "0.in.pool.ntp.org"
#define SYS_RNWF_WIFI_CallbackHandler	    APP_WIFI_Callback


// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/

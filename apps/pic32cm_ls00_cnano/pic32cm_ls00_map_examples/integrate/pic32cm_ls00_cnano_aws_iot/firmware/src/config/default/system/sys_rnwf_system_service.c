/*******************************************************************************
  RNWF Host Assisted System Service Implementation

  File Name:
    sys_rnwf_system_service.c

  Summary:
    Source code for the RNWF Host Assisted System Service implementation.

  Description:
    This file contains the source code for the RNWF Host Assisted System Service
    implementation.
 *******************************************************************************/

/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

 
Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "system/inf/sys_rnwf_interface.h"
#include "system/sys_rnwf_system_service.h"
#include "system/console/sys_console.h"

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* RNWF System Service Conttrol Function */
SYS_RNWF_RESULT_t SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SERVICE_t request, void *input)
{
    SYS_RNWF_RESULT_t result = SYS_RNWF_FAIL;
    
    switch(request)
    {
        /* RNWF Reset */
        case SYS_RNWF_SYSTEM_RESET:
        {
            uint8_t init_msg[128];
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, init_msg, SYS_RNWF_RESET_TARGET);
            break;
        }
        
        /* RNWF Echo Off  */
        case SYS_RNWF_SYSTEM_ECHO_OFF:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_ECHO_OFF);
            break;
        }
        
        /* RNWF Manufacturer ID */
        case SYS_RNWF_SYSTEM_GET_MAN_ID:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+GMI:", input, SYS_RNWF_GET_MAN_ID);
            break;
        }
        
        /* RNWF Software Revision */
        case SYS_RNWF_SYSTEM_SW_REV:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+GMR:", input, SYS_RNWF_GET_REV_ID);
            break;
        }
        
        /* RNWF Device Info  */
        case SYS_RNWF_SYSTEM_DEV_INFO:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+DI:", input, SYS_RNWF_GET_DEV_INFO);
            break;
        }
        
        /* RNWF Get Certificates List */
        case SYS_RNWF_SYSTEM_GET_CERT_LIST:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+FS:2,1,", input, SYS_RNWF_GET_CERT_LIST);
            break;
        }
        
        /* RNWF Get Key List*/
        case SYS_RNWF_SYSTEM_GET_KEY_LIST:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+FS:2,2,", input, SYS_RNWF_GET_KEY_LIST);
            break;
        }
        
        /* RNWF Get Time */
        case SYS_RNWF_SYSTEM_GET_TIME:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_GET_SYS_TIME);
            break;
        }
        
        /* RNWF Get Time Unix*/
        case SYS_RNWF_SYSTEM_SET_TIME_UNIX:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SYS_TIME_UNIX, *(uint32_t*)input);
            break;
        }
        
        /* RNWF Set time ntp */
        case SYS_RNWF_SYSTEM_SET_TIME_NTP:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SYS_TIME_NTP, *(uint32_t*)input);
            break;
        }
        
        /* RNWF Set time string */
        case SYS_RNWF_SYSTEM_SET_TIME_STRING:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SYS_TIME_STRING, (uint8_t*)input);
            break;
        }
        
        /* RNWF Get Wifi Info */
        case SYS_RWWF_SYSTEM_GET_WIFI_INFO:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+NETIFC:", input, SYS_RNWF_GET_WIFI_IFNO);
            break;
        }
        
        /* RNWF Set SNTP */
        case SYS_RNWF_SYSTEM_SET_SNTP:
        {            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SNTP_STATIC, input);
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SNTP_URL, input);
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SET_SNTP_ENBL, input);
            break;
        }
        
        /* RNWF GET Mqtt info */
        case SYS_RNWF_SYSTEM_GET_MQTT_INFO:
        {            
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+MQTTC:", input, SYS_RNWF_GET_MQTT_IFNO);
            break;
        }
        
        default:
        {
            break;
        }
    }
    
    return result;
}

/* *****************************************************************************
 End of File
 */

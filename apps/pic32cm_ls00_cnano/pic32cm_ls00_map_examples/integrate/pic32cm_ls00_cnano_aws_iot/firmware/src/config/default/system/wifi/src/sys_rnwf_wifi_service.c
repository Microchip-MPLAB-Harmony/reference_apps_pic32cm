/*******************************************************************************
  RNWF Host Assisted Wifi Service Implementation

  File Name:
    sys_rnwf_wifi_service.c

  Summary:
    Source code for the RNWF Host Assisted wifi Service implementation.

  Description:
    This file contains the source code for the RNWF Host Assisted wifiService
    implementation.
 ******************************************************************************/

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
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "peripheral/sercom/usart/plib_sercom0_usart.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
SYS_RNWF_WIFI_CALLBACK_t g_wifiCallBackHandler[SYS_RNWF_WIFI_SERVICE_CB_MAX];

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*Wi-Fi Service Control Function*/
SYS_RNWF_RESULT_t SYS_RNWF_WIFI_SrvCtrl( SYS_RNWF_WIFI_SERVICE_t request, void *input)  {

    SYS_RNWF_RESULT_t result = SYS_RNWF_PASS;
    
    switch (request)
    {
        /**<Request/Trigger Wi-Fi connect */
        case SYS_RNWF_STA_CONNECT:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_CONNECT);
            break;
        }

         /**<Request/Trigger Wi-Fi disconnect */    
        case SYS_RNWF_STA_DISCONNECT:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_DISCONNECT);
            break;
        }

        /**<Request/Trigger SoftAP disable */       
        case SYS_RNWF_AP_DISABLE:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SOFTAP_DISABLE); 
            break;
        }

         /**<Configure the Wi-Fi channel */    
        case SYS_RNWF_SET_WIFI_AP_CHANNEL:
        {            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_CHANNEL, *(uint8_t *)input);
            break;            
        }   
        
        /**<Configure the Access point's BSSID */ 
        case SYS_RNWF_SET_WIFI_BSSID:
        {            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_CHANNEL, (uint8_t *)input);            
            break;
        } 
        
        /**<Configure Wi-Fi connection timeout */ 
        case SYS_RNWF_SET_WIFI_TIMEOUT:
        {
            break;
        }

        /**<Configure Hidden mode SSID in SoftAP mode*/     
        case SYS_RNWF_SET_WIFI_HIDDEN:
        {            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_HIDDEN, *(uint8_t *)input);            
            break;
        }  

        /**<Configure the Wi-Fi parameters */ 
        case SYS_RNWF_SET_WIFI_PARAMS:  
        {
            SYS_RNWF_WIFI_PARAM_t *wifi_config = (SYS_RNWF_WIFI_PARAM_t *)input;
            
            if(wifi_config->mode == SYS_RNWF_WIFI_MODE_STA)
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SOFTAP_DISABLE);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_DISCONNECT);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_STA_SSID, wifi_config->ssid);            
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_STA_PWD, wifi_config->passphrase);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_STA_SEC, wifi_config->security);
                if(wifi_config->autoconnect)
                {
                    result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL,SYS_RNWF_WIFI_SOFTAP_DISABLE );
                    
                    result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_CONNECT);
                }
            }
            else if(wifi_config->mode == SYS_RNWF_WIFI_MODE_AP)                
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_DISCONNECT);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SOFTAP_DISABLE);       
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_SSID, wifi_config->ssid);     
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_PWD, wifi_config->passphrase);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_SEC, wifi_config->security);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SET_AP_CHANNEL, wifi_config->channel);
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_SOFTAP_ENABLE);
            }
            break;            
        }



        case SYS_RNWF_WIFI_SET_REGULATORY_DOMAIN:
        {
            const char *reg_domain = (const char *)input;
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL,NULL, SYS_RNWF_WIFI_SET_REG_DONAIN, reg_domain);
            
            break;
        }

        /**<Request/Trigger Wi-Fi passive scan */ 
        case SYS_RNWF_WIFI_PASSIVE_SCAN:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_PSV_SCAN);
            break;
        }

        /**<Request/Trigger Wi-Fi active scan */    
        case SYS_RNWF_WIFI_ACTIVE_SCAN:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_WIFI_ACT_SCAN);
            break;
        }

        /**<Regester the call back for async events */    
        case SYS_RNWF_WIFI_SET_CALLBACK:  
        {
            g_wifiCallBackHandler[1] = (SYS_RNWF_WIFI_CALLBACK_t)input;
            break;
        }

        /**<Regester the call back for async events */
        case SYS_RNWF_WIFI_SET_SRVC_CALLBACK:                        
        {
            g_wifiCallBackHandler[0] = (SYS_RNWF_WIFI_CALLBACK_t)input;  
            break;
        }
        case SYS_RNWF_WIFI_GET_CALLBACK:
        {
            SYS_RNWF_WIFI_CALLBACK_t *callBackHandler;
            callBackHandler = (SYS_RNWF_WIFI_CALLBACK_t *)input;
            
            callBackHandler[0] = g_wifiCallBackHandler[0];
            callBackHandler[1] = g_wifiCallBackHandler[1];
            break;
        }
        
        /* RNWF Get Wifi Config Info */
        case SYS_RNWF_GET_WIFI_CONF_INFO:
        {
            *(uint8_t*)input = '\0';
            result = SYS_RNWF_CMD_SEND_OK_WAIT("+WIFIC:", input, SYS_RNWF_WIFI_CONF_INFO);
            break;
        }
        default:
        {
            result = SYS_RNWF_FAIL;
            break;
        }
            
    }  
    return result;
}


/* *****************************************************************************
 End of File
 */

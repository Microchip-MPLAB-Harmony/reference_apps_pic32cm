/*******************************************************************************
  RNWF Host Assisted System service Header file

  File Name:
    sys_rnwf_system_service.h

  Summary:
    Header file for the RNWF Host Assisted System service implementation.

  Description:
    This header file provides a simple APIs to enable System service with RNWF device 
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.
 
 * Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SYS_RNWF_SYSTEM_SERVICE_H
#define	SYS_RNWF_SYSTEM_SERVICE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "inf/sys_rnwf_interface.h"

/* RNWF Basic Commands */
#define SYS_RNWF_SET_ECHO_OFF       "ATE0\r\n"

#define SYS_RNWF_RESET_TARGET       "AT+RST\r\n"
#define SYS_RNWF_GET_MAN_ID         "AT+GMI\r\n"
#define SYS_RNWF_GET_MOD_ID         "AT+GMI\r\n"
#define SYS_RNWF_GET_REV_ID         "AT+GMR\r\n"
#define SYS_RNWF_GET_DEV_INFO       "AT+DI\r\n"

/* RNWF SNTP Commands */
#define SYS_RNWF_SET_SNTP_ENBL      "AT+SNTPC=1,1\r\n"
#define SYS_RNWF_SET_SNTP_STATIC    "AT+SNTPC=2,1\r\n"
#define SYS_RNWF_SET_SNTP_DHCP      "AT+SNTPC=2,1\r\n"
#define SYS_RNWF_SET_SNTP_URL       "AT+SNTPC=3,\"%s\"\r\n"

/* RNWF TIME Commands */
#define SYS_RNWF_GET_SYS_TIME       "AT+TIME,3\r\n"

#define SYS_RNWF_SET_SYS_TIME_UNIX  "AT+TIME=1,%lu\r\n"
#define SYS_RNWF_SET_SYS_TIME_NTP   "AT+TIME=2,%lu\r\n"

/* Date/time in format YYYY-MM-DDTHH:MM:SS.00Z */ 
#define SYS_RNWF_SET_SYS_TIME_STRING    "AT+TIME=3,\"%s\"\r\n"

/* RNWF Certificate Commands */
#define SYS_RNWF_GET_CERT_LIST      "AT+FS=2,1\r\n"
#define SYS_RNWF_GET_KEY_LIST       "AT+FS=2,2\r\n"


#define SYS_RNWF_GET_WIFI_IFNO     "AT+NETIFC=0\r\n"

#define SYS_RNWF_GET_MQTT_IFNO     "AT+MQTTC\r\n"


/**
 @brief System Service List
 
 */
typedef enum
{
    /**<Request/Trigger reset the system */
    SYS_RNWF_SYSTEM_RESET,             
            
    /**<Request Software Revision */
    SYS_RNWF_SYSTEM_SW_REV,            
        
    /**<Request Software Revision */            
    SYS_RNWF_SYSTEM_DEV_INFO,         
            
     /**<Request/Trigger reset the system */
    SYS_RNWF_SYSTEM_ECHO_OFF,          
            
    /**<Get the manufacturing ID */
    SYS_RNWF_SYSTEM_GET_MAN_ID,       
            
     /**<Enable SNTP with given server URL */
    SYS_RNWF_SYSTEM_SET_SNTP,         
            
    /**<Set the sytem time in UNIX format */            
    SYS_RNWF_SYSTEM_SET_TIME_UNIX,     
            
    /**<Set the system time in NTP format */            
    SYS_RNWF_SYSTEM_SET_TIME_NTP,      
            
    /**<Set the system time in string(YYYY-MM-DDTHH:MM:SS.00Z) format */                        
    SYS_RNWF_SYSTEM_SET_TIME_STRING, 
            
     /**<Get the system time */
    SYS_RNWF_SYSTEM_GET_TIME,          
            
    /**<Get the available certificate list */
    SYS_RNWF_SYSTEM_GET_CERT_LIST,     
            
    /**<Get the available private key list */
    SYS_RNWF_SYSTEM_GET_KEY_LIST,      
            
    /**<Get the Wifi Info */
    SYS_RWWF_SYSTEM_GET_WIFI_INFO,
            
    /**<Get the MQTT info */
    SYS_RNWF_SYSTEM_GET_MQTT_INFO,  
            
}SYS_RNWF_SYSTEM_SERVICE_t;


/**
 * @brief System Service Layer API to handle system operations.
 * 
 *
 * @param[in] request   Service request ::SYS_RNWF_SYSTEM_SERVICE_t
 * @param[in] input     Parameters for the requested service
 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
SYS_RNWF_RESULT_t SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SERVICE_t request, void *input);

#endif	/* SYS_RNWF_SYSTEM_SERVICE_H */

/** @}*/

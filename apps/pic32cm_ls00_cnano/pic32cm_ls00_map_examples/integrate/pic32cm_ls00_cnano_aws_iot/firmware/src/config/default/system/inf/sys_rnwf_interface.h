/*******************************************************************************
  RNWF Host Assisted Interface Header file

  File Name:
    sys_rnwf_interface.h

  Summary:
    Header file for the RNWF Host Assisted Interface Service implementation.

  Description:
    This header file provides a simple APIs to enable communication with RNWF device 
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
#ifndef SYS_RNWF_INTERFACE_H
#define	SYS_RNWF_INTERFACE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <xc.h> // include processor files - each processor file is guarded.  
#include "system/console/sys_console.h"

/* Interface debug Print */
#define SYS_RNWF_IF_DBG_MSG(args, ...)    SYS_CONSOLE_PRINT("[IF]:"args, ##__VA_ARGS__)

/* Interface buffer maximum size */
#define SYS_RNWF_BUF_LEN_MAX        4096

/* Interface maximum length */
#define SYS_RNWF_IF_LEN_MAX         1024 

/* Interface Async msg maximum size */
#define SYS_RNWF_IF_ASYNC_MSG_MAX   (512+256) 

/* Interface Async msg count maximum size */
#define SYS_RNWF_IF_ASYNC_MSG_CNT   2

/* Interface Async buffer maximum msg size */
#define SYS_RNWF_IF_ASYNC_BUF_MAX   (SYS_RNWF_IF_ASYNC_MSG_MAX*SYS_RNWF_IF_ASYNC_MSG_CNT)
/* Interface buffer maximum size */
#define SYS_RNWF_IF_BUF_MAX     (SYS_RNWF_IF_ASYNC_BUF_MAX/SYS_RNWF_IF_ASYNC_MSG_MAX)

/* Interface Timeout */
#define SYS_RNWF_IF_TIMEOUT      0x7FFFF

/* Enable to get Debug prints of Interface */
//#define SYS_RNWF_INTERFACE_DEBUG        1

/* Interface status macros */
#define SYS_RNWF_IS_INTERFACE_BUSY()      if(g_interfaceState == SYS_RNWF_INTERFACE_BUSY){SYS_CONSOLE_PRINT("IF Busy\n");return SYS_RNWF_INTERFACE_BUSY;}
#define SYS_RNWF_SET_INTERFACE_BUSY()     (g_interfaceState = SYS_RNWF_INTERFACE_BUSY)             
#define SYS_RNWF_SET_INTERFACE_FREE()     (g_interfaceState = SYS_RNWF_INTERFACE_FREE)         

/* Macros related to RNWF Commands */
#define SYS_RNWF_AT_CMD     "AT"
#define SYS_RNWF_AT_EOL     "\r\n>"
#define SYS_RNWF_AT_DONE    "OK"
#define SYS_RNWF_AT_ERROR   "ERROR"

#define SYS_RNWF_ARG_DELIMETER        ":"

/*  Wi-Fi Event Code*/
#define SYS_RNWF_EVENT_STA_AUTO_IP    "WSTAAIP:"
#define SYS_RNWF_EVENT_AP_AUTO_IP     "WAPAIP:"

/*  Wi-Fi Ping Event Code*/
#define SYS_RNWF_EVENT_PING           "PING:"

/*IPv6 local link IP prefix*/
#define SYS_RNWF_WIFI_IPv6_LOCAL_PREFIX      "FE80"

#define SYS_RNWF_EVENT_LINK_UP        "WSTALU:"
#define SYS_RNWF_EVENT_LINK_LOSS      "WSTALD:"
#define SYS_RNWF_EVENT_ERROR          "WSTAERR:"

/*DNS Resolve Event Code*/
#define SYS_RNWF_EVENT_DNS_RES       "DNSRESOLV"

/*  SCAN Event Code*/
#define SYS_RNWF_EVENT_SCAN_IND       "WSCNIND:"
#define SYS_RNWF_EVENT_SCAN_DONE      "WSCNDONE:"

/*  DHCP Event Code*/
#define SYS_RNWF_EVENT_DHCP_CFG       "DHCPSC:"

/*  DNS Event Code */
#define SYS_RNWF_EVENT_DNS_RESOLVE    "DNSRESOLV:"
#define SYS_RNWF_EVENT_DNS_ERROR      "DNSERR:"

/*  TIME Event Code */
#define SYS_RNWF_EVENT_TIME           "TIME:"

/*  INFO Event Code */
#define SYS_RNWF_EVENT_INFO           "INFO:"

/* SOCKET Event Code */
#define SYS_RNWF_EVENT_SOCK_CONNECTED   "SOCKIND:"
#define SYS_RNWF_EVENT_SOCK_TLS_DONE    "SOCKTLS:"
#define SYS_RNWF_EVENT_SOCK_TCP_RECV    "SOCKRXT:"
#define SYS_RNWF_EVENT_SOCK_UDP_RECV    "SOCKRXU:"
#define SYS_RNWF_EVENT_SOCK_CLOSE       "SOCKCL:"
#define SYS_RNWF_EVENT_SOCK_TLS_SUCCESS "SOCKTLS:"
#define SYS_RNWF_EVENT_SOCK_ERROR       "SOCKERR:"

#define SYS_RNWF_EVENT_MQTT_CONNECTED   "MQTTCONN:"
#define SYS_RNWF_EVENT_MQTT_CONN_ACKED  "MQTTCONNACK:"
#define SYS_RNWF_EVENT_MQTT_PUB_ACKED   "MQTTPUBACK:"
#define SYS_RNWF_EVENT_MQTT_PUB_ERR     "MQTTPUBERR:"
#define SYS_RNWF_EVENT_MQTT_PUB_COMPLT  "MQTTPUBCOMP:"
#define SYS_RNWF_EVENT_MQTT_SUB_RESP    "MQTTSUB:"
#define SYS_RNWF_EVENT_MQTT_UNSUB_RESP  "MQTTUNSUB:"
#define SYS_RNWF_EVENT_MQTT_PROP_RX     "MQTTPROPRX:"
#define SYS_RNWF_EVENT_MQTT_SUB_MSG     "MQTTSUBRX:"
// *****************************************************************************

/* RNWF Result Event values

  Summary:
    Identifies the result type of the RNWF command sent 

  Remarks:
    None.
 */
typedef enum
{      
    /**<Success*/
    SYS_RNWF_PASS =  0x0000, 
            
    /**<Failure*/
    SYS_RNWF_FAIL =  -1, 
            
    /**<RAW mode*/
    SYS_RNWF_RAW = -2,  
            
    /**<Retry*/ 
    SYS_RNWF_COTN =  -3, 
            
    /**<Busy*/
    SYS_RNWF_BUSY = -4, 
            
    /**<Timeout*/
    SYS_RNWF_TIMEOUT = -5,
            
}SYS_RNWF_RESULT_t;


// *****************************************************************************

/* RNWF Interface state values

  Summary:
    Identifies the RNWF interface state 

  Remarks:
    None.
 */
typedef enum
{          
    /* Interface is free */
    SYS_RNWF_INTERFACE_FREE,
            
    /* Interface is busy */
    SYS_RNWF_INTERFACE_BUSY
            
}SYS_RNWF_INTERFACE_STATE_t;

// *****************************************************************************

/* RNWF Interface Queue structure

  Summary:
    Queue structure

  Remarks:
    None.
 */

typedef struct 
{
    /* Head data */
    int8_t    head; 
    
    /* Tail data */
    int8_t    tail;
    
    /* Size of Queue */
    int8_t    size;
    
    /* Queue array */
    uint32_t  queue[SYS_RNWF_IF_BUF_MAX];
    
}SYS_RNWF_IF_QUEUE_t;

// *****************************************************************************
/*  Function:
        SYS_RNWF_IF_Read(uint8_t *buffer, uint16_t len)

    Summary:
        Reads response from the RNWF device

    Description:
        This  function reads the response from RNWF 
 
    Remarks:
        None
 */
size_t SYS_RNWF_IF_Read(uint8_t *buffer, uint16_t len);

// *****************************************************************************
/*  Function:
        SYS_RNWF_IF_RawWrite(uint8_t *, size_t);

    Summary:
        Sends data to RNWF device

    Description:
        This function sends data to RNWF
 
    Remarks:
        None
 */
SYS_RNWF_RESULT_t SYS_RNWF_IF_RawWrite(uint8_t *, size_t);

// *****************************************************************************
/*  Function:
        SYS_RNWF_IF_Init (void);

    Summary:
        Initialization of RNWF device

    Description:
        This function is used to do Initialization of the RNWF device  
 
    Remarks:
        None
 */
SYS_RNWF_RESULT_t SYS_RNWF_IF_Init(void);

// *****************************************************************************
/*  Function:
        SYS_RNWF_RESULT_t SYS_RNWF_IF_EventHandler(void);

    Summary:
        Interface Event handler

    Description:
        This function handles the Interface events
 
    Remarks:
        None
 */
SYS_RNWF_RESULT_t SYS_RNWF_IF_EventHandler(void);

// *****************************************************************************
/*  Function:
        int16_t SYS_RNWF_IF_RawRead(uint8_t *buffer, uint16_t len);

    Summary:
        Read Raw response from the RNWF device

    Description:
        This function Reads response from RNWF in Raw mode
 
    Remarks:
        None
 */
int16_t SYS_RNWF_IF_RawRead(uint8_t *buffer, uint16_t len);

// *****************************************************************************
/*  Function:
        uint16_t SYS_RNWF_IF_Write(uint8_t *buffer, size_t len);

    Summary:
        Write to RNWF device

    Description:
        This function writes/sends command to RNWF  
 
    Remarks:
        None
 */
uint16_t SYS_RNWF_IF_Write(uint8_t *buffer, size_t len);

// *****************************************************************************
/*  Function:
        SYS_RNWF_IF_CmdRspSend(const char * cmd_complete,const char * delimeter, uint8_t * response,const char * format, ...) ;

    Summary:
        Send command and wait for response from the RNWF device

    Description:
        This function executes commands and accumulates responses from the RNWF device.
 
    Remarks:
        None
 */
int16_t SYS_RNWF_IF_CmdRspSend(const char * cmd_complete,const char * delimeter, uint8_t * response,const char * format, ...) ;
 
 
#define SYS_RNWF_IF_BUF_Q_ENQUEUE(frameIdx)   SYS_RNWF_IF_QEnqueue(&g_interfaceFreeQ,frameIdx) 
#define SYS_RNWF_IF_BUF_Q_DEQUEUE(frameIdx)   SYS_RNWF_IF_QDequeue(&g_interfaceFreeQ, frameIdx) 

#define SYS_RNWF_IF_RX_Q_ENQUEUE(frameIdx)   SYS_RNWF_IF_QEnqueue(&g_interfaceRxQ, frameIdx) 
#define SYS_RNWF_IF_RX_Q_DEQUEUE(frameIdx)   SYS_RNWF_IF_QDequeue(&g_interfaceRxQ, frameIdx) 

#define SYS_RNWF_CMD_SEND_OK_WAIT(delimeter, response, format, ...) SYS_RNWF_IF_CmdRspSend(SYS_RNWF_AT_DONE, delimeter, response, format, ##__VA_ARGS__)
#define SYS_RNWF_CMD_SEND_RESP_WAIT(cmd_complete, delimeter, response, format, ...) SYS_RNWF_IF_CmdRspSend(cmd_complete, delimeter, response, format, __VA_ARGS__)


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* SYS_RNWF_INTERFACE_H */

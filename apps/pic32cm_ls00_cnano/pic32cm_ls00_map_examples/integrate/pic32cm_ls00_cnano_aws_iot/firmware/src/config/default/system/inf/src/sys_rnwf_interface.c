/*******************************************************************************
  RNWF Host Assisted Interface Implementation

  File Name:
    sys_rnwf_interface.c

  Summary:
    Source code for the RNWF Host Assisted Interface implementation.

  Description:
    This file contains the source code for the RNWF Host Assisted Interface 
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* This section lists the other files that are included in this file.
 */
#include "system/debug/sys_debug.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "peripheral/dmac/plib_dmac.h"
#include "system/sys_rnwf_system_service.h"
#include "peripheral/sercom/usart/plib_sercom0_usart.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/mqtt/sys_rnwf_mqtt_service.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* Queue used by the interface bus to full the incoming frames*/
static SYS_RNWF_IF_QUEUE_t g_interfaceFreeQ;

/* Queue used push the read bytes to process incoming frames*/
static SYS_RNWF_IF_QUEUE_t g_interfaceRxQ;

/* To check UART transmission completion*/
static volatile bool g_isUART0TxComplete = true;

/* Interface Time out*/
static uint32_t g_interfaceTimeout = SYS_RNWF_IF_TIMEOUT;

/* Asynchronous buffer for Queue implementation*/
static uint8_t g_asyncBuffer[SYS_RNWF_IF_ASYNC_BUF_MAX] = {'\0'};

/* Asynchronous buffer for Event handler*/
static char g_asyncBuf[SYS_RNWF_IF_ASYNC_MSG_MAX] = {'\0'};

/* To check UART RX ready*/
static volatile bool g_IsRxReady = true;

/* Initialize interface state with interface free */
static SYS_RNWF_INTERFACE_STATE_t g_interfaceState = SYS_RNWF_INTERFACE_FREE;

 
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* Circular Queue implementation */

/*
 * 
 * @param if_q -> queue pointer
 * 
 */
static void SYS_RNWF_IF_QInit(SYS_RNWF_IF_QUEUE_t * if_q) 
{
	if_q -> tail = -1;
	if_q -> head = 0;
	if_q -> size = 0;
}

/*
 * SYS_RNWF_IF_QFull() -> Queue full API
 * @param if_q -> queue pointer
 * 
 * @return true/false
 */
static bool SYS_RNWF_IF_QFull(SYS_RNWF_IF_QUEUE_t * if_q) 
{
	if (if_q -> size >= SYS_RNWF_IF_BUF_MAX)
        return true;

	return false;
}

/*
 * SYS_RNWF_IF_QEmpty() -> Queue empty API
 * @param if_q -> queue pointer
 * 
 * @return true/false
 */
static bool SYS_RNWF_IF_QEmpty(SYS_RNWF_IF_QUEUE_t * if_q) 
{
	if (if_q -> size)
        return false;

	return true;
}

/*
 * SYS_RNWF_IF_QEnqueue() -> Queue Enqueue API
 * @param if_q -> queue pointer
 * 
 * @return true/false
 */
static bool SYS_RNWF_IF_QEnqueue(SYS_RNWF_IF_QUEUE_t * if_q, uint8_t * mem_idx) 
{
	if (SYS_RNWF_IF_QFull(if_q)) 
	{
        return false;
	}

	if_q -> tail = ((if_q -> tail + 1) % SYS_RNWF_IF_BUF_MAX);
	if_q -> queue[if_q -> tail] = (uint32_t)mem_idx;
	if_q -> size++;

	return true;
}

/*
 * SYS_RNWF_IF_QDequeue() -> Queue Dequeue API
 * @param if_q -> queue pointer
 * 
 * @return true/false
 */
static bool SYS_RNWF_IF_QDequeue(SYS_RNWF_IF_QUEUE_t * if_q, uint8_t ** mem_idx) 
{
	if (SYS_RNWF_IF_QEmpty(if_q)) 
	{
        return false;
	}

	* mem_idx = (uint8_t *)if_q -> queue[if_q -> head];
	if_q -> head = ((if_q -> head + 1) % SYS_RNWF_IF_BUF_MAX);
	if_q -> size--;

	return true;
}

/* To do Software reset of the RNWF device */
static SYS_RNWF_RESULT_t SYS_RNWF_IF_SwReset(void) 
{
	SYS_RNWF_IF_CmdRspSend(NULL, NULL, NULL, SYS_RNWF_RESET_TARGET, NULL);
    
    return SYS_RNWF_PASS;
}

/* Function to return the status of RX*/
static inline bool SYS_RNWF_IF_IsRxReady()
{
    return g_IsRxReady;
}

/* To Read Command Response from RNWF*/
static inline size_t SYS_RNWF_IF_CommandRespRead(uint8_t* pRdBuffer, const size_t size)
{
    return SERCOM0_USART_Read(pRdBuffer, size);
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/* To Get the Socket ID from the string */
static uint32_t SYS_RNWF_IF_GetSocketID(unsigned char * main_string) 
{
    unsigned char * temp;
    uint32_t socket_id = 0;
    for (int i = 0; main_string[i]; i++) 
    {
        if (main_string[i] == ' ' || main_string[i] == '=' || main_string[i] == ':') 
        {
            temp = main_string + i + 1;
            for (int j = 0; temp[j]; j++) 
            {
                if (temp[j] == ',' || temp[j] == ' ') 
                {
                    return socket_id;
                }
                socket_id = (socket_id * 10) + (temp[j] - 48);
            }
	    return socket_id;
        }
    }
    return 0;
}

/*Check whether the address is IPv4 or IPv6*/
bool SYS_RNWF_IpAddress(unsigned char * main_string)
{
    int count=0;
    for(int i=0; main_string[i]; i++)
    {
        if(main_string[i] == '.')
            count++;
    }
    if(count >= 3)
        return true;
    else
        return false;
}

/* This function effectively replaces the '\r' character with '\0' */
static SYS_RNWF_RESULT_t SYS_RNWF_IF_ResponseTrim(uint8_t *buffer)
{
    int16_t idx = strlen((char *)buffer);

    while(idx-- >= 0)
    {
        if(buffer[idx] == '\r')
        {
            buffer[idx] = '\0';
            return SYS_RNWF_PASS;
        }
    }
    return SYS_RNWF_FAIL;
}

/* To segregate the incoming messages and initiates the required callback functions. */
static SYS_RNWF_RESULT_t SYS_RNWF_IF_AsyncHandler(uint8_t * p_msg) 
{
    uint16_t arg_len = 0;
    SYS_RNWF_RESULT_t result = SYS_RNWF_COTN;
    uint8_t * p_arg = (uint8_t * )(strstr((char * ) p_msg, SYS_RNWF_ARG_DELIMETER) + 1);
    
    while (p_arg[arg_len] != '\0') 
    {
        (p_arg[arg_len] == ',') ? p_arg[arg_len++] = ' ': arg_len++;
    }
    
    SYS_RNWF_IF_ResponseTrim(p_arg);
    
    #ifdef SYS_RNWF_INTERFACE_DEBUG
        SYS_RNWF_IF_DBG_MSG("Async Message -> %s\r\n", p_msg);
        SYS_RNWF_IF_DBG_MSG("Async Arguments-> %s\r\n", p_arg);
    #endif
    switch (p_msg[0]) 
    {

        /** Wi-Fi DHCP/DNS or Net based Async Events*/
        case 'W':
        case 'D':
        case 'N': 
        case 'T':
        case 'P':
        {
            SYS_RNWF_WIFI_CALLBACK_t wifi_CallBackHandler[SYS_RNWF_WIFI_SERVICE_CB_MAX];
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_GET_CALLBACK, wifi_CallBackHandler);
            
            for (int i = 0; i < SYS_RNWF_WIFI_SERVICE_CB_MAX; i++) 
            {
                if (NULL == wifi_CallBackHandler[i])
                    continue;

                SYS_RNWF_WIFI_CALLBACK_t wifi_cb_func = wifi_CallBackHandler[i];

                if ((strstr((char * ) p_msg, SYS_RNWF_EVENT_STA_AUTO_IP) != NULL) || (strstr((char * ) p_msg, SYS_RNWF_EVENT_AP_AUTO_IP) != NULL))
                {
                    bool IPv4_address = false;
                    IPv4_address = SYS_RNWF_IpAddress(p_msg);
                    if(IPv4_address == true)
                    {
                        wifi_cb_func(SYS_RNWF_IPv4_DHCP_DONE, p_arg);
                    }
                    else
                    {
                        wifi_cb_func(SYS_RNWF_IPv6_DHCP_DONE, p_arg);
                    }
                }

                else if ((strstr((char * ) p_msg, SYS_RNWF_EVENT_LINK_LOSS) != NULL) || (strstr((char * ) p_msg, SYS_RNWF_EVENT_ERROR) != NULL))
                {
                    wifi_cb_func(SYS_RNWF_DISCONNECTED, p_arg);
                } 

                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_LINK_UP)) 
                {
                    wifi_cb_func(SYS_RNWF_CONNECTED, p_arg);
                }

                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_ERROR)) 
                {
                    wifi_cb_func(SYS_RNWF_CONNECT_FAILED, p_arg);
                }

                if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SCAN_IND)) 
                {
                    wifi_cb_func(SYS_RNWF_SCAN_INDICATION, p_arg);
                }

                if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SCAN_DONE)) 
                {
                    wifi_cb_func(SYS_RNWF_SCAN_DONE, p_arg);
                    result = SYS_RNWF_PASS;
                }
                
                if(strstr((char *)p_msg, SYS_RNWF_EVENT_TIME))
                {
                    wifi_cb_func(SYS_RNWF_SNTP_UP, p_arg);   
                }

                if(strstr((char *)p_msg, SYS_RNWF_EVENT_PING))
                {
                    wifi_cb_func(SYS_RNWF_WIFI_PING_RESP, p_arg);   
                }

            }
        }
        break;
        /** Socket based Async Events*/
        case 'S': 
        {
            uint32_t socket_id = 0;
            SYS_RNWF_NET_SOCK_EVENT_t event = SYS_RNWF_NET_SOCK_EVENT_UNDEFINED;
            
            SYS_RNWF_NET_SOCK_CALLBACK_t socketCallBackHandler[SYS_RNWF_NET_SOCK_SERVICE_CB_MAX];
            SYS_RNWF_NET_SockSrvCtrl(SYS_RNWF_NET_SOCK_GET_CALLBACK, socketCallBackHandler);
            
            /** Service Layer Handler */
            for (uint8_t i = 0; i < SYS_RNWF_NET_SOCK_SERVICE_CB_MAX; i++) 
            {
                SYS_RNWF_NET_SOCK_CALLBACK_t netSock_cb_func = socketCallBackHandler[i];

                if (NULL == netSock_cb_func)
                    continue;
                
                if (strstr((char * )p_msg, "SOCK")) 
                {
                    unsigned char *msg = (unsigned char *)strstr((char * )p_msg, "SOCK");
                    socket_id = SYS_RNWF_IF_GetSocketID(msg);
                }

                if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SOCK_TLS_DONE)) 
                {
                    event = SYS_RNWF_NET_SOCK_EVENT_TLS_DONE;
                }
                
                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SOCK_CONNECTED)) 
                {
                    event = SYS_RNWF_NET_SOCK_EVENT_CONNECTED;
                }
                
                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SOCK_CLOSE)) 
                {
                    event = SYS_RNWF_NET_SOCK_EVENT_DISCONNECTED;
                }
                
                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SOCK_TCP_RECV)) 
                {
                    uint16_t rx_len = 0;
                    rx_len= atoi(strstr((char * ) p_arg," "));
                    p_arg = (uint8_t * ) & rx_len;
					SYS_CONSOLE_PRINT(" ");
                    event = SYS_RNWF_NET_SOCK_EVENT_READ;
                }
                
                else if (strstr((char * ) p_msg, SYS_RNWF_EVENT_SOCK_ERROR)) 
                {
                    event = SYS_RNWF_NET_SOCK_EVENT_ERROR;
                }
                
                netSock_cb_func(socket_id, event, p_arg);
            }
        }
        break;
	    /** MQTT based Async Events*/
        case 'M': 
        {
            uint32_t status = 0;
            
            SYS_RNWF_MQTT_CALLBACK_t mqttCallBackHandler[SYS_RNWF_MQTT_SERVICE_CB_MAX];
            SYS_RNWF_MQTT_SrvCtrl (SYS_RNWF_MQTT_GET_CALLBACK, mqttCallBackHandler);
            
            for(uint8_t i = 0; i < SYS_RNWF_MQTT_SERVICE_CB_MAX; i++)
            {
                SYS_RNWF_MQTT_CALLBACK_t mqtt_cb_func = mqttCallBackHandler[i];
                /**No call back then just return */
                if(mqtt_cb_func == NULL)
                    continue;
            
                if(strstr((char *)p_msg, SYS_RNWF_EVENT_MQTT_CONNECTED))
                {                
                    status = atoi((const char *)p_arg);
                    if(status)
                        result = mqtt_cb_func(SYS_RNWF_MQTT_CONNECTED, p_arg);
                    else
                        result = mqtt_cb_func(SYS_RNWF_MQTT_DISCONNECTED, p_arg);                
                }   
                
                if(strstr((char *)p_msg, SYS_RNWF_EVENT_MQTT_SUB_RESP))
                {
                    result = mqtt_cb_func(SYS_RNWF_MQTT_SUBCRIBE_ACK, p_arg);
                }
                
                if(strstr((char *)p_msg, SYS_RNWF_EVENT_MQTT_SUB_MSG))
                {
                    result = mqtt_cb_func(SYS_RNWF_MQTT_SUBCRIBE_MSG, p_arg);
                }
                
                if(result == SYS_RNWF_COTN)
                    break;
            }
        }
        break;


        default:
        {
            break;
        }
    }
    
    return result; 
}

/* To read the response from RNWF */
size_t SYS_RNWF_IF_Read(uint8_t *buffer, uint16_t len)
{
    size_t read_cnt = 0;
    g_interfaceTimeout = SYS_RNWF_IF_TIMEOUT;
    while((read_cnt < len) && (g_interfaceTimeout != 0))
    {
        g_interfaceTimeout--;
        //later make it timeout        
        if(SYS_RNWF_IF_IsRxReady())
        {      
            read_cnt += SYS_RNWF_IF_CommandRespRead(&buffer[read_cnt],len);
            if(read_cnt != len)
            {
                continue;
            }
        }
    }  
    return read_cnt;
}

/* To send data to RNWF */
SYS_RNWF_RESULT_t SYS_RNWF_IF_RawWrite(uint8_t *buffer, size_t len)
{
    /* Write the bytes to Interface */
    SYS_RNWF_IF_Write(buffer, len);
    
    /* check the response */
    SYS_RNWF_IF_CmdRspSend(NULL, NULL, NULL, NULL);
    
    return SYS_RNWF_PASS;
}

/* To exit RNWF from RAW mode transfer */
static void SYS_RNWF_IF_ExitRawMode(void) 
{
    uint8_t rawExitCmd[] = "+++";
    SYS_RNWF_IF_RawWrite((uint8_t *)rawExitCmd,(size_t) 3);
}

/* To send command to RNWF using DMAC */
static size_t SYS_RNWF_IF_CommandSend(uint8_t *p_frame, size_t cmd_len)
{
    size_t ret = 0;
    if ( * p_frame != '\0') 
    {
        if(true == DMAC_ChannelTransfer(DMAC_CHANNEL_0, p_frame, \
                (const void * ) & (SERCOM0_REGS -> USART_INT.SERCOM_DATA), \
                cmd_len))
        {
            ret = cmd_len;
        }

        while (!SERCOM0_USART_TransmitComplete());
    }
    return ret;
}


/* To Read response from RNWF in Raw mode*/
int16_t SYS_RNWF_IF_RawRead(uint8_t *buffer, uint16_t len)
{
    uint8_t tempBuf[4] = {0, 0, 0, 0};
    int16_t result = SYS_RNWF_TIMEOUT;

    if((result = SYS_RNWF_IF_Read(buffer, len)) == len) // Complete response is copied
    {
        if(SYS_RNWF_IF_Read(tempBuf, 4) == 4)
        {
            if(memcmp(tempBuf, "OK\r\n", 4) == 0)
            {
                return result;
            }                       
        }
    }
    SYS_RNWF_IF_ExitRawMode();
    return result;
}

/* To write/send command to RNWF */
uint16_t SYS_RNWF_IF_Write(uint8_t *buffer, size_t len)
{
    if(len == SYS_RNWF_IF_CommandSend(buffer,len))
    {
        return SYS_RNWF_PASS;
    }
    return SYS_RNWF_FAIL;
}


/* To handle the Interface events*/
SYS_RNWF_RESULT_t SYS_RNWF_IF_EventHandler(void)
{   
    uint8_t *ptr_async;            
    while(SYS_RNWF_IF_RX_Q_DEQUEUE(&ptr_async) != false)
    {                   
        strcpy((char *)g_asyncBuf, (const char *)ptr_async);        
        SYS_RNWF_IF_BUF_Q_ENQUEUE(ptr_async);        
        
        char *token = strtok((char *)g_asyncBuf, "\r+");        
        while(token != NULL)
        {            
            if(strlen(token) > 1)
            {
                SYS_RNWF_IF_AsyncHandler((uint8_t *)token);            
            }
            token = strtok(NULL, "\r+");            
        }
    }
    
    if(SYS_RNWF_IF_IsRxReady())
        SYS_RNWF_IF_CmdRspSend(NULL, NULL, NULL, NULL);
    
    return SYS_RNWF_PASS;
}

/* To execute commands and accumulates responses from the RNWF device.*/
int16_t SYS_RNWF_IF_CmdRspSend(const char * cmd_complete,const char * delimeter, uint8_t * response,const char * format, ...) 
{
    static uint8_t g_ifBuffer[SYS_RNWF_IF_LEN_MAX];
    uint8_t *p_frame = g_ifBuffer;
    uint16_t result = SYS_RNWF_PASS;
    size_t cmd_len;
    volatile int16_t rsp_len = 0;        
    uint16_t offset =  0;
    va_list args;   

    if (format != NULL) 
    {  
        va_start(args, format);
        cmd_len = vsnprintf((char * ) g_ifBuffer, SYS_RNWF_IF_LEN_MAX, format, args);
        va_end(args);

        #ifdef SYS_RNWF_INTERFACE_DEBUG
            SYS_RNWF_IF_DBG_MSG("cmd[%d] -> %s\r\n", cmd_len, p_frame);
        #endif /* SYS_RNWF_INTERFACE_DEBUG */

        SYS_RNWF_IF_CommandSend(p_frame,cmd_len);
        
        memset(g_ifBuffer, 0, cmd_len);
        if(response != NULL)
            response[0] = '\0';
    }

    g_interfaceTimeout = SYS_RNWF_IF_TIMEOUT;
    /* later make it timeout*/  
    while(g_interfaceTimeout--) 
    {
        if(SYS_RNWF_IF_IsRxReady())
        {
            if(rsp_len < SYS_RNWF_IF_LEN_MAX)
            {
                if(1 == (SYS_RNWF_IF_CommandRespRead(&g_ifBuffer[rsp_len],1)))
                {
                    rsp_len++;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                /* truncate the buffer till '\r\n' */            
                g_ifBuffer[rsp_len-3] = g_ifBuffer[rsp_len-2];
                g_ifBuffer[rsp_len-2] = g_ifBuffer[rsp_len-1];
                rsp_len = rsp_len-1;    
            }
            
            if(g_ifBuffer[rsp_len-1] == '#')
            {    
                #ifdef SYS_RNWF_INTERFACE_DEBUG       
                    SYS_RNWF_IF_DBG_MSG("RAW Mode!\r\n");
                #endif /* SYS_RNWF_INTERFACE_DEBUG */                                     
                result = SYS_RNWF_RAW;
                break;
            }  
            
            if((rsp_len > 1) && (g_ifBuffer[rsp_len - 1] == '\n') && (g_ifBuffer[rsp_len - 2] == '\r'))
            {   
                if((rsp_len > 3) && ((g_ifBuffer[rsp_len - 3] == 'K') && (g_ifBuffer[rsp_len - 4] == 'O')))
                {
                    g_ifBuffer[rsp_len-1] = '\0';
                    g_ifBuffer[rsp_len-2] = '\0';

					#ifdef SYS_RNWF_INTERFACE_DEBUG
							SYS_RNWF_IF_DBG_MSG("rsp[%d] -> %.*s\n", rsp_len, rsp_len, g_ifBuffer);
					#endif /* SYS_RNWF_INTERFACE_DEBUG */

                    if(response != NULL)
                    {
                        if(delimeter != NULL)
                        {
                            uint8_t *token, *rsp_ptr;

                            rsp_ptr = (uint8_t *)strstr((char *)g_ifBuffer, delimeter);
                            offset =  strlen(delimeter);
                            token = (uint8_t *)strtok((char *)rsp_ptr, "\r\n");

                            /* walk through other tokens */
                            while( token != NULL ) {
                                if(strstr((char *)token, delimeter))
                                {
                                    strcat((char *)response, " \0");
                                    strcat((char *)response, (char *)token+offset);
                                }
                                token = (uint8_t *)strtok(NULL, "\r\n");
                            }
                        }
                        else
                        {
                            g_ifBuffer[rsp_len-5] = '\0';
                            memcpy((char *)response, (char *)g_ifBuffer, rsp_len-5);
                            result = rsp_len-5;
                        }
                    }
                    break;
                }
                else
                {
                    if((g_ifBuffer[0] == 'E') && (g_ifBuffer[1] == 'R' && (g_ifBuffer[2] == 'R')))
                    {
                        g_ifBuffer[rsp_len-1] = '\0';
                        g_ifBuffer[rsp_len-2] = '\0';
						#ifdef SYS_RNWF_INTERFACE_DEBUG
							SYS_RNWF_IF_DBG_MSG("%s\r\n", g_ifBuffer);
						#endif
                        if(response != NULL)
                        {
                            strcpy((char *)response, (char *)g_ifBuffer+sizeof(SYS_RNWF_AT_ERROR));
                        }
                        result = SYS_RNWF_FAIL;
                        break;
                    }

                }
                
                if((g_ifBuffer[0] == '\r') && (g_ifBuffer[1] == '+'))
                {
                    uint8_t *async_buf ;
                    g_ifBuffer[rsp_len] = '\0';
                    if(SYS_RNWF_IF_BUF_Q_DEQUEUE(&async_buf))
                    {
                        memset(async_buf, 0, SYS_RNWF_IF_ASYNC_MSG_MAX);
                        strncpy((char *)async_buf, (char *)g_ifBuffer, SYS_RNWF_IF_ASYNC_MSG_MAX);
						#ifdef SYS_RNWF_INTERFACE_DEBUG
							SYS_RNWF_IF_DBG_MSG("AEC[%d] -> %s\n", rsp_len, async_buf);
						#endif /* SYS_RNWF_INTERFACE_DEBUG */
                        
                        SYS_RNWF_IF_RX_Q_ENQUEUE(async_buf);
                    }
                    else
                    {
					#ifdef SYS_RNWF_INTERFACE_DEBUG
                        SYS_RNWF_IF_DBG_MSG("No Free-Q\n");
					#endif /* SYS_RNWF_INTERFACE_DEBUG */
                    }
                    if(response != NULL || format != NULL)
                    {
                        rsp_len = 0;
                        continue;
                    }
                    break;
                }
            }
        }
    }
    
    if (g_interfaceTimeout == -1)
    {
        result = SYS_RNWF_TIMEOUT;
    }
    
    SYS_RNWF_SET_INTERFACE_FREE();
    return result;
}

/* To check if the DMAC transfer is complete*/
static void SYS_RNWF_IF_Usart0txDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) 
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE) 
    {
        g_isUART0TxComplete = true;
    }
}

/* This function is used to do Initialization of the RNWF device */
SYS_RNWF_RESULT_t SYS_RNWF_IF_Init(void) 
{
	SYS_RNWF_IF_QInit( & g_interfaceFreeQ);
	SYS_RNWF_IF_QInit( & g_interfaceRxQ);

	/* Fill up the free queue will all the buffer entries */
	for (uint8_t * pMem_addr = g_asyncBuffer; pMem_addr < (g_asyncBuffer + SYS_RNWF_IF_ASYNC_BUF_MAX); pMem_addr += SYS_RNWF_IF_ASYNC_MSG_MAX) 
	{
		SYS_RNWF_IF_BUF_Q_ENQUEUE(pMem_addr);
	}

	DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, SYS_RNWF_IF_Usart0txDmaChannelHandler, 0);

    /* Software Reset of RNWF device */
	SYS_RNWF_IF_SwReset(); 

    while(SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_ECHO_OFF, NULL) != SYS_RNWF_PASS)
    {
        SYS_CONSOLE_PRINT("Waiting for RNWF...\r\n");
    } 
    
    /* Echo Off of RNWF device */
	SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_ECHO_OFF, NULL);

	return SYS_RNWF_PASS;
}


/* *****************************************************************************
 End of File
 */

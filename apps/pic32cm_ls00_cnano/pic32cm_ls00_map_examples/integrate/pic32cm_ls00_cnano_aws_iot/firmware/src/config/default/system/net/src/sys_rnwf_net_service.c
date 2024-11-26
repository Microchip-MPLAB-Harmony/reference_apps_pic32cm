/*******************************************************************************
  RNWF Host Assisted Net Service Implementation

  File Name:
    sys_rnwf_net_service.c

  Summary:
    Source code for the RNWF Host Assisted Net Service implementation.

  Description:
    This file contains the source code for the RNWF Host Assisted Net Service
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


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "system/inf/sys_rnwf_interface.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/console/sys_console.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
SYS_RNWF_NET_SOCK_CALLBACK_t g_SocketCallBackHandler[SYS_RNWF_NET_SOCK_SERVICE_CB_MAX];

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* This function is used for Network and Socket service Control*/
SYS_RNWF_RESULT_t SYS_RNWF_NET_SockSrvCtrl( SYS_RNWF_NET_SOCK_SERVICE_t request, void *input)
{
    SYS_RNWF_RESULT_t result = SYS_RNWF_PASS; 

    switch(request)
    {
        /**<Enable the DHCP server */
        case SYS_RNWF_NET_DHCP_SERVER_ENABLE:
        {
            if(input == NULL)
                break;
            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_DHCPS_DISABLE);
            const char **dhcps_cfg_list = input;                        
            
            if(dhcps_cfg_list[0] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_NETIF_SET_IP, dhcps_cfg_list[0]);            
            
            if(dhcps_cfg_list[1] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_DHCPS_POOL_START, dhcps_cfg_list[1]);
            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_DHCPS_ENABLE);
            
            break;
        }
	
        /**<Disable the DHCP server */ 
        case SYS_RNWF_NET_DHCP_SERVER_DISABLE:
        {
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_DHCPS_DISABLE);   
            break;
        }
	
        /**<Open TCP Socket*/ 
        case SYS_RNWF_NET_SOCK_TCP_OPEN: 
        {   
            SYS_RNWF_NET_SOCKET_t *socket = (SYS_RNWF_NET_SOCKET_t*)(input); 
            uint8_t socket_id[32];
            
            if(SYS_RNWF_CMD_SEND_OK_WAIT(SYS_RNWF_SOCK_OPEN_RESP, (uint8_t *)socket_id, SYS_RNWF_SOCK_OPEN_TCP,socket->IP) == SYS_RNWF_PASS)
            {
                socket->sock_master = atoi((char *)socket_id);
                switch(socket->bind_type)
                {
                    case SYS_RNWF_BIND_LOCAL:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_LOCAL, socket->sock_master, socket->sock_port);                        
                        break;
                    }

                    case SYS_RNWF_BIND_REMOTE:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_REMOTE, socket->sock_master, socket->sock_addr, socket->sock_port);
                        break;
                    }

                    case SYS_RNWF_BIND_MCAST:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_MCAST, socket->sock_master, socket->sock_addr, socket->sock_port);
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                
                if(socket->tls_conf && result == SYS_RNWF_PASS)
                    result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_CONFIG_TLS, socket->sock_master, socket->tls_conf);
            }                                    
            break;
        }
            
        /**<Open UDP Socket*/  
        case SYS_RNWF_NET_SOCK_UDP_OPEN:   
        {
            SYS_RNWF_NET_SOCKET_t *socket = (SYS_RNWF_NET_SOCKET_t*)(input);
            int8_t socket_id[32];
            
            if(SYS_RNWF_CMD_SEND_OK_WAIT(SYS_RNWF_SOCK_OPEN_RESP, (uint8_t *)socket_id, SYS_RNWF_SOCK_OPEN_UDP) == SYS_RNWF_PASS)
            {
                sscanf((char *)socket_id, "%lu", &socket->sock_master);
                switch(socket->bind_type)
                {
                    case SYS_RNWF_BIND_LOCAL:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_LOCAL, socket->sock_master, socket->sock_port);                        
                        break;
                    }

                    case SYS_RNWF_BIND_REMOTE:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_REMOTE, socket->sock_master, socket->sock_addr, socket->sock_port);
                        break;
                    }

                    case SYS_RNWF_BIND_MCAST:
                    {
                        result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BIND_MCAST, socket->sock_master, socket->sock_addr, socket->sock_port);
                        break; 
                    }

                    default:
                    {
                        break;
                    }
                }               
            }             
            break;
        }
            
        /**<Close the socket*/    
        case SYS_RNWF_NET_SOCK_CLOSE:
        {
            uint32_t socket = *((uint32_t *)input);
            if(socket)
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_CLOSE, socket); 
            }           
            
            break;
        }
	
        /**<Configure the socket settings*/
        case SYS_RNWF_NET_SOCK_CONFIG:
        {
            SYS_RNWF_NET_SOCKET_CONFIG_t *sock_cfg = (SYS_RNWF_NET_SOCKET_CONFIG_t *)input;
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_CONFIG_NODELAY, sock_cfg->sock_id, sock_cfg->sock_nodelay);            
            result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_CONFIG_KEEPALIVE, sock_cfg->sock_id, sock_cfg->sock_keepalive);              
            
            break;
        }
	
        /**<Use the TLS configurations */
        case SYS_RNWF_NET_TLS_CONFIG_1:
        case SYS_RNWF_NET_TLS_CONFIG_2:
        {
            const char **tls_cfg_list = input;            
            
            if(tls_cfg_list[SYS_RNWF_NET_PEER_AUTH] != NULL)
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_NET_PEER_AUTHENTICATION, request, 1);
                
                if(tls_cfg_list[SYS_RNWF_NET_TLS_CA_CERT] != NULL)
                    result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_SET_CA_NAME, request, tls_cfg_list[SYS_RNWF_NET_TLS_CA_CERT]);     
            }
            else
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_NET_PEER_AUTHENTICATION, request, 0);
            }
                 
            
            if(tls_cfg_list[SYS_RNWF_NET_TLS_CERT_NAME] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_SET_CERT_NAME, request, tls_cfg_list[SYS_RNWF_NET_TLS_CERT_NAME]);     
            
            if(tls_cfg_list[SYS_RNWF_NET_TLS_KEY_NAME] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_SET_KEY_NAME, request, tls_cfg_list[SYS_RNWF_NET_TLS_KEY_NAME]);     
            
            if(tls_cfg_list[SYS_RNWF_NET_TLS_KEY_PWD] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_SET_KEY_PWD, request, tls_cfg_list[SYS_RNWF_NET_TLS_KEY_PWD]);     
            
            if(tls_cfg_list[SYS_RNWF_NET_TLS_SERVER_NAME] != NULL)
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_SERVER_NAME, request, tls_cfg_list[SYS_RNWF_NET_TLS_SERVER_NAME]); 

            if(tls_cfg_list[SYS_RNWF_NET_TLS_DOMAIN_NAME] != NULL)
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL,SYS_RNWF_SOCK_TLS_DOMAIN_NAME_VERIFY, request, 1);
                if(tls_cfg_list[SYS_RNWF_NET_TLS_DOMAIN_NAME_VERIFY] != NULL)
                    result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_TLS_DOMAIN_NAME, request, tls_cfg_list[SYS_RNWF_NET_TLS_DOMAIN_NAME]);
            }
            else
            {
                result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL,SYS_RNWF_SOCK_TLS_DOMAIN_NAME_VERIFY, request, 0);
            }
            
            break;
        }

        /**<Register application callback for sockets*/
        case SYS_RNWF_NET_SOCK_SET_CALLBACK:
	    {
            g_SocketCallBackHandler[1] = (SYS_RNWF_NET_SOCK_CALLBACK_t)(input);                        
            break;
	    }    
        
        /**<Register application callback for sockets*/
        case SYS_RNWF_NET_SOCK_SET_SRVC_CALLBACK:
	    {
            g_SocketCallBackHandler[0] = (SYS_RNWF_NET_SOCK_CALLBACK_t)(input);                        
            break;
	    }   
	    
        /*<Get Function callback data*/        
        case SYS_RNWF_NET_SOCK_GET_CALLBACK:
        {
            SYS_RNWF_NET_SOCK_CALLBACK_t *socketCallBackHandler;
            socketCallBackHandler = (SYS_RNWF_NET_SOCK_CALLBACK_t *) input;
            
            socketCallBackHandler[0] = g_SocketCallBackHandler[0];
            socketCallBackHandler[1] = g_SocketCallBackHandler[1];
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

/*This function is used to write into TCP Socket*/
SYS_RNWF_RESULT_t SYS_RNWF_NET_TcpSockWrite( uint32_t socket, uint16_t length, uint8_t *input)
{
    SYS_RNWF_RESULT_t result = SYS_RNWF_FAIL;

    if((result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_BINARY_WRITE_TCP, socket, length)) == SYS_RNWF_RAW) 
    {
        result = SYS_RNWF_IF_RawWrite(input, length);     
    }    
    return result;
}

/*This function is used to write into UDP Socket*/
SYS_RNWF_RESULT_t SYS_RNWF_NET_UdpSockWrite( uint32_t socket, uint8_t *addr, uint32_t port, uint16_t length, uint8_t *input)
{
    SYS_RNWF_RESULT_t result = SYS_RNWF_FAIL;
    if((result = SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_ASCII_WRITE_UDP, socket, addr, port, length, input)) == SYS_RNWF_RAW)     
    {                                                            
        result = SYS_RNWF_IF_RawWrite(input, length);         
    }
    
    return result;
}

/*This function is used to read from the socket*/
int16_t SYS_RNWF_NET_SockRead( uint32_t socket, uint16_t length, uint8_t *buffer) 
{                
    int16_t result = SYS_RNWF_FAIL;
    if(SYS_RNWF_CMD_SEND_OK_WAIT(NULL, NULL, SYS_RNWF_SOCK_READ, socket, SYS_RNWF_BINARY_MODE, length) == SYS_RNWF_RAW)
    {  
        result = SYS_RNWF_IF_RawRead(buffer, length);
    }    
    return result;
}

/*This function is used to read from the TCP socket*/
int16_t SYS_RNWF_NET_TcpSockRead( uint32_t socket, uint16_t  length, uint8_t *buffer)  
{
    return SYS_RNWF_NET_SockRead(socket, length, buffer);
}

/*This function is used to read from the UDP socket*/
int16_t SYS_RNWF_NET_UdpSockRead( uint32_t socket, uint16_t length, uint8_t *buffer)  
{                   
    return SYS_RNWF_NET_SockRead(socket, length, buffer);
}


/* *****************************************************************************
 End of File
 */

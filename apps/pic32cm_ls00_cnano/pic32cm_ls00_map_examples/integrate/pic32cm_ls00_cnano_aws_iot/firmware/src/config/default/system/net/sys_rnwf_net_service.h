/*******************************************************************************
  RNWF Host Assisted Net Service Header file 

  File Name:
    sys_rnwf_net_service.h

  Summary:
    Header file for the RNWF Host Assisted Net Service implementation.

  Description:
    This file contains the header file for the RNWF Host Assisted Net Service
    implementation.
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
#ifndef SYS_RNWF_NET_SERVICE_H
#define	SYS_RNWF_NET_SERVICE_H

#include <xc.h> // include processor files - each processor file is guarded.  


/*RNWF Network Socket max callback service */
#define SYS_RNWF_NET_SOCK_SERVICE_CB_MAX    2

#define SYS_RNWF_SOCK_ID_LEN_MAX            8
#define SYS_RNWF_SOCK_ADDR_LEN_MAX          32
#define SYS_RNWF_SOCK_TLS_CFG_LEN_MAX       64

/*RNWF NET Service commands */
#define SYS_RNWF_DHCPS_ENABLE               "AT+DHCPSC=0,1,1\r\n"
#define SYS_RNWF_DHCPS_DISABLE              "AT+DHCPSC=0,1,0\r\n"
#define SYS_RNWF_DHCPS_POOL_START           "AT+DHCPSC=0,2,\"%s\"\r\n"

#define SYS_RNWF_DHCPS_SET_GW               "AT+DHCPSC=0,10,\"%s\"\r\n"

#define SYS_RNWF_NETIF_SET_IP               "AT+NETIFC=0,40,\"%s\"\r\n"

#define SYS_RNWF_SOCK_GET_LIST              "AT+SOCKLST\r\n"

#define SYS_RNWF_SOCK_OPEN_UDP              "AT+SOCKO=1,4\r\n"
#define SYS_RNWF_SOCK_OPEN_TCP              "AT+SOCKO=2,%d\r\n"
#define SYS_RNWF_SOCK_OPEN_RESP             "+SOCKO:"

#define SYS_RNWF_SOCK_BIND_LOCAL            "AT+SOCKBL=%lu,%d\r\n"
#define SYS_RNWF_SOCK_BIND_REMOTE           "AT+SOCKBR=%lu,\"%s\",%d\r\n"
#define SYS_RNWF_SOCK_BIND_MCAST            "AT+SOCKBM=%lu,%s,%d\r\n"


#define SYS_RNWF_SOCK_BINARY_WRITE_TCP      "AT+SOCKWR=%lu,%u\r\n"
#define SYS_RNWF_SOCK_BINARY_WRITE_UDP      "AT+SOCKWRTO=%lu,%s,%d,%d\r\n"

#define SYS_RNWF_SOCK_ASCII_WRITE_TCP       "AT+SOCKWR=%d,%d,\"%.*s\"\r\n"
#define SYS_RNWF_SOCK_ASCII_WRITE_UDP       "AT+SOCKWRTO=%d,%s,%d,%d,\"%.*s\"\r\n"

#define SYS_RNWF_SOCK_READ_BUF              "AT+SOCKRDBUF=%lu,%d,%d\r\n"
#define SYS_RNWF_SOCK_READ                  "AT+SOCKRD=%lu,%d,%d\r\n"
#define SYS_RNWF_SOCK_READ_RESP             "+SOCKRD:"

#define SYS_RNWF_SOCK_CLOSE                 "AT+SOCKCL=%lu\r\n"

#define SYS_RNWF_SOCK_CONFIG_TLS            "AT+SOCKTLS=%lu,%d\r\n"
#define SYS_RNWF_SOCK_CONFIG_KEEPALIVE      "AT+SOCKC=%lu,1,%d\r\n"
#define SYS_RNWF_SOCK_CONFIG_NODELAY        "AT+SOCKC=%lu,2,%d\r\n"


#define SYS_RNWF_SOCK_TLS_GET_CONFIG        "AT+TLSC\r\n"
#define SYS_RNWF_SOCK_TLS_SET_CA_NAME       "AT+TLSC=%d,1,\"%s\"\r\n"
#define SYS_RNWF_SOCK_TLS_SET_CERT_NAME     "AT+TLSC=%d,2,\"%s\"\r\n"
#define SYS_RNWF_SOCK_TLS_SET_KEY_NAME      "AT+TLSC=%d,3,\"%s\"\r\n"
#define SYS_RNWF_SOCK_TLS_SET_KEY_PWD       "AT+TLSC=%d,4,\"%s\"\r\n"
#define SYS_RNWF_SOCK_TLS_SERVER_NAME       "AT+TLSC=%d,5,\"%s\"\r\n"
#define SYS_RNWF_SOCK_TLS_DOMAIN_NAME       "AT+TLSC=%d,6,\"%s\"\r\n"
#define SYS_RNWF_NET_PEER_AUTHENTICATION    "AT+TLSC=%d,40,%d\r\n"
#define SYS_RNWF_SOCK_TLS_DOMAIN_NAME_VERIFY  "AT+TLSC=%d,41,%d\r\n"



/**
 @brief Network Interfaces List
 
 */
typedef enum 
{
    /* RNWF Wifi Init */
    SYS_RNWF_IF_WIFI,

}SYS_RNWF_NET_IFS_t;

/**
 @brief Network and Socket service List
 */
typedef enum 
{
    /**<Use the TLS configuration 1*/
    SYS_RNWF_NET_TLS_CONFIG_1 = 1, 
    
    /**<Use the TLS configuration 2*/
    SYS_RNWF_NET_TLS_CONFIG_2,       
            
    /**<Network Interface configuration*/        
    SYS_RNWF_NET_IF_CONFIG,                 
     
    /**<Enable the DHCP server */
    SYS_RNWF_NET_DHCP_SERVER_ENABLE,
     
    /**<Disable the DHCP server */        
    SYS_RNWF_NET_DHCP_SERVER_DISABLE,  
            
    /**<Open TCP Socket*/        
    SYS_RNWF_NET_SOCK_TCP_OPEN,     
    
    /**<Open UDP Socket*/        
    SYS_RNWF_NET_SOCK_UDP_OPEN,     
            
    /**<Close the socket*/        
    SYS_RNWF_NET_SOCK_CLOSE,     
            
    /**<Configurs the socket settings*/        
    SYS_RNWF_NET_SOCK_CONFIG,               
    
    /**<Register application callback for sockets*/
    SYS_RNWF_NET_SOCK_SET_CALLBACK,         
    SYS_RNWF_NET_SOCK_SET_SRVC_CALLBACK,
            
    /*<Get Function callback data*/        
    SYS_RNWF_NET_SOCK_GET_CALLBACK,

}SYS_RNWF_NET_SOCK_SERVICE_t;



/**
 @brief Socket Type
 
 */
typedef enum   
{    
    /**<UDP Socket type*/
    SYS_RNWF_SOCK_UDP = 1,                  
            
    /**<TCP Socket type*/
    SYS_RNWF_SOCK_TCP,  

}SYS_RNWF_SOCK_TYPE_t;

/**
 @brief Socket Bind Type
 
 */
typedef enum    
{ 
    /**<Bind as a server Socket*/
    SYS_RNWF_BIND_LOCAL,         
            
    /**<Bind as a client Socket*/        
    SYS_RNWF_BIND_REMOTE,          
     
     /**<Bind as a multicast Socket*/
    SYS_RNWF_BIND_MCAST,             
           
    /**Bind None**/        
    SYS_RNWF_BIND_NONE,

}SYS_RNWF_BIND_TYPE_t;

/**
 @brief Socket Type
 
 */
typedef enum    
{ 
    /**<ASCII Socket mode*/
    SYS_RNWF_ASCII_MODE = 1,      
            
    /**<Binary Socket mode*/        
    SYS_RNWF_BINARY_MODE,  

}SYS_RNWF_SOCK_RW_MODE_t;



/**
 @brief Network socket events
 */
typedef enum 
{
    /**<Socket connected event*/
    SYS_RNWF_NET_SOCK_EVENT_CONNECTED,  
            
    /**<TLS handshake done event*/
    SYS_RNWF_NET_SOCK_EVENT_TLS_DONE,
            
    /**<Socket disonnected event*/
    SYS_RNWF_NET_SOCK_EVENT_DISCONNECTED,   
            
    /**<Socket data read event*/        
    SYS_RNWF_NET_SOCK_EVENT_READ,          
        
    /**<Socket error event*/        
    SYS_RNWF_NET_SOCK_EVENT_ERROR,
            
    /**<Socket event undefined*/
    SYS_RNWF_NET_SOCK_EVENT_UNDEFINED,

}SYS_RNWF_NET_SOCK_EVENT_t;


/**
 @brief Network socket TLS Configurations
 */
typedef enum 
{
    /* TLS Peer authentication */
    SYS_RNWF_NET_PEER_AUTH = 0,

    /* TLS CA Certificate */
    SYS_RNWF_NET_TLS_CA_CERT, 

    /* TLS Certificate Name */         
    SYS_RNWF_NET_TLS_CERT_NAME,

    /* TLS Key name  */            
    SYS_RNWF_NET_TLS_KEY_NAME,

    /* TLS Key password  */
    SYS_RNWF_NET_TLS_KEY_PWD,

    /* TLS Server name  */
    SYS_RNWF_NET_TLS_SERVER_NAME,

    /* TLS Domain Name */
    SYS_RNWF_NET_TLS_DOMAIN_NAME,
     
    /* TLS Domain Name Verify */
    SYS_RNWF_NET_TLS_DOMAIN_NAME_VERIFY

}SYS_RNWF_NET_TLS_CONFIG_ID_t;


/**
 @brief Network socket IP Configurations
 */
typedef enum 
{
    /**<Open socket with IPv4 address */
        SYS_RNWF_NET_IPV4 = 4,

    /**<Open socket with IPv6 locak address */
        SYS_RNWF_NET_IPV6_LOCAL = 6,
    
    /**<Open socket with IPv6 global address */
        SYS_RNWF_NET_IPV6_GLOBAL = 6,
    
}SYS_RNWF_NET_IP_TYPE_t;


/**
 @brief Socket data type
 
 */
typedef struct 
{    
    /**<Bind type of socket*/
    SYS_RNWF_BIND_TYPE_t    bind_type;  
    
    /**<UDP or TCP socket type*/
    SYS_RNWF_SOCK_TYPE_t    sock_type;        
    
    /**<Server or Client port number*/
    uint16_t            sock_port;          
    
    /**<Socket Address (IPv4 or IPv6   Address)*/
    const char          *sock_addr;         
    
    /**<Server Socket ID*/
    uint32_t            sock_master;       
    
    /**<TLS configuration */
    uint8_t             tls_conf; 

    /**<Open socket with IPv4/IPv6 address */
    SYS_RNWF_NET_IP_TYPE_t  IP;

}SYS_RNWF_NET_SOCKET_t;


/**
 @brief Advanced Socket settings
 
 */
typedef struct 
{  
    /**<Socket ID */
    uint32_t    sock_id;                
    
    /**<Keep-Alive option*/
    uint8_t     sock_keepalive;         
    
    /**<Socket NAGLE/NoDelay*/
    uint8_t     sock_nodelay;  

}SYS_RNWF_NET_SOCKET_CONFIG_t;


/**
 @brief Network socket events callback function type 
 */
typedef SYS_RNWF_RESULT_t (*SYS_RNWF_NET_SOCK_CALLBACK_t)(uint32_t sock, SYS_RNWF_NET_SOCK_EVENT_t, uint8_t *);

/**
 * @brief NET Sock Service Layer API to handle system operations.
 * 
 *
 * @param[in] request       Requested service ::SYS_RNWF_NET_SOCK_SERVICE_t
 * @param[in] input         Input/Output data for the requested service 
 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
SYS_RNWF_RESULT_t SYS_RNWF_NET_SockSrvCtrl( SYS_RNWF_NET_SOCK_SERVICE_t request, void *input);


/**
 * @brief NET Socket Write API to send data over TCP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * @param[in] mode          Write mode for the socket ::SYS_RNWF_SOCK_RW_MODE_t
 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
SYS_RNWF_RESULT_t SYS_RNWF_NET_TcpSockWrite( uint32_t socket, uint16_t length, uint8_t *input);

/**
 * @brief NET Socket Write API to send data UDP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] addr          IP address of the UDP peer
 * @param[in] port          Port address of the UDP peer
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     

 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
SYS_RNWF_RESULT_t SYS_RNWF_NET_UdpSockWrite( uint32_t socket, uint8_t *addr, uint32_t port, uint16_t length, uint8_t *input);

/**
 * @brief NET Socket Read API to read data from TCP Socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
int16_t SYS_RNWF_NET_TcpSockRead( uint32_t socket, uint16_t length, uint8_t *input);


/**
 * @brief NET Socket Read API to read data from UDP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * 
 * @return ::SYS_RNWF_PASS Requested service is handled successfully
 * @return ::SYS_RNWF_FAIL Requested service has failed
 */
int16_t SYS_RNWF_NET_UdpSockRead( uint32_t socket, uint16_t length, uint8_t *input);

#endif	/* SYS_RNWF_NET_SERVICE_H */

/** @}*/

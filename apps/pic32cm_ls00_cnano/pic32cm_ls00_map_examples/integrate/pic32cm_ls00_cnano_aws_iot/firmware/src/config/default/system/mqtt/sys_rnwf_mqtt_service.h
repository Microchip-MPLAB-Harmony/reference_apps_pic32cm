/*******************************************************************************
  RNWF Host Assisted MQTT Service Header file 

  File Name:
    sys_rnwf_mqtt_service.h

  Summary:
    Header file for the RNWF Host Assisted MQTT Service implementation.

  Description:
    This file contains the header file for the RNWF Host Assisted MQTT Service
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
#ifndef SYS_RNWF_MQTT_SERVICE_H
#define	SYS_RNWF_MQTT_SERVICE_H

#include <xc.h> // include processor files - each processor file is guarded.  


#define SYS_RNWF_MQTT_DBG_MSG(args, ...)      SYS_CONSOLE_PRINT("[MQTT]:"args, ##__VA_ARGS__)

/*MQTT buffer max length*/
#define SYS_RNWF_MQTT_BUF_LEN_MAX	4096

/* MQTT Configuration Commands */
#define SYS_RNWF_MQTT_SET_BROKER_URL    "AT+MQTTC=1,\"%s\"\r\n"
#define SYS_RNWF_MQTT_SET_BROKER_PORT   "AT+MQTTC=2,%d\r\n"
#define SYS_RNWF_MQTT_SET_CLIENT_ID     "AT+MQTTC=3,\"%s\"\r\n"
#define SYS_RNWF_MQTT_SET_USERNAME      "AT+MQTTC=4,\"%s\"\r\n"
#define SYS_RNWF_MQTT_SET_PASSWORD      "AT+MQTTC=5,\"%s\"\r\n"
#define SYS_RNWF_MQTT_SET_KEEPALIVE     "AT+MQTTC=6,%d\r\n"
#define SYS_RNWF_MQTT_SET_TLS_CONF      "AT+MQTTC=7,%d\r\n"
#define SYS_RNWF_MQTT_SET_PROTO_VER     "AT+MQTTC=8,%d\r\n"

/* MQTT Connection Commands */
#define SYS_RNWF_MQTT_CMD_CONNECT      "AT+MQTTCONN=1\r\n"
#define SYS_RNWF_MQTT_CMD_RECONNECT    "AT+MQTTCONN=0\r\n"

/* MQTT Disconnection Commands */
#define SYS_RNWF_MQTT_CMD_DISCONNECT        "AT+MQTTDISCONN=0\r\n"

/* MQTT Subscribe Commands */
#define SYS_RNWF_MQTT_CMD_SUBSCRIBE_QOS0    "AT+MQTTSUB=\"%s\",0\r\n"
#define SYS_RNWF_MQTT_CMD_SUBSCRIBE_QOS1    "AT+MQTTSUB=\"%s\",1\r\n"
#define SYS_RNWF_MQTT_CMD_SUBSCRIBE_QOS2    "AT+MQTTSUB=\"%s\",2\r\n"
#define SYS_RNWF_MQTT_CMD_UNSUBSCRIBE       "AT+MQTTUNSUB=%s\r\n"

/* MQTT Publish Commands */
#define SYS_RNWF_MQTT_CMD_PUBLISH           "AT+MQTTPUB=%d,%d,%d,\"%s\",\"%s\"\r\n"

/* MQTT Transmit Properties  Commands */
#define SYS_RNWF_MQTT_SET_TX_SESSION_EXPIRY     "AT+MQTTPROPTX=17,%d\r\n"
#define SYS_RNWF_MQTT_SET_TX_USER_PROP          "AT+MQTTPROPTX=38,%s\r\n"

/* MQTT Receive Properties  Commands */
#define SYS_RNWF_MQTT_SET_RX_SESSION_EXPIRY     "AT+MQTTPROPRX=17,%d\r\n"
#define SYS_RNWF_MQTT_SET_RX_TOP_ALIAS_MAX      "AT+MQTTPROPRX=34,%s\r\n"
#define SYS_RNWF_MQTT_SET_RX_TOP_USER_PROP      "AT+MQTTPROPRX=38,%s\r\n"


#define SYS_RNWF_MQTT_CLR_TX_PROP_ID            "AT+MQTTPROPTXS=%d,0"
#define SYS_RNWF_MQTT_SET_TX_PROP_ID            "AT+MQTTPROPTXS=%d,1"


#define SYS_RNWF_MQTT_PORT_NO_ECN_NO_AUTH        1883
#define SYS_RNWF_MQTT_PORT_NO_ECN_AUTH           1884
#define SYS_RNWF_MQTT_PORT_ECN_NO_AUTH           8883

/*MQTT Service max callback service*/
#define SYS_RNWF_MQTT_SERVICE_CB_MAX        2

//#define SYS_RNWF_MQTT_DPS_SUBSCRIBE_TOPIC    "$MCHP/Sample"
#define SYS_RNWF_MQTT_DPS_SUBSCRIBE_TOPIC    "$dps/registrations/res/#"
/**
 @defgroup MQTT_GRP MQTT Cloud API
 @{
 */

/**
 @brief Network and Socket service List
 
 */
typedef enum 
{
    /**<Configure the MQTT Broker parameters*/
    SYS_RNWF_MQTT_CONFIG,               
            
    /**<Connect to the MQTT Broker */        
    SYS_RNWF_MQTT_CONNECT,              
            
    /**<Request reconnect to the MQTT Cloud*/        
    SYS_RNWF_MQTT_RECONNECT,        
            
    /**<Trigger Disconnect from MQTT Broker*/        
    SYS_RNWF_MQTT_DISCONNECT,      
            
    /**<Subscribe to QoS0 Topics */        
    SYS_RNWF_MQTT_SUBSCRIBE_QOS0,  
            
    /**<Subscribe to QoS1 Topics */        
    SYS_RNWF_MQTT_SUBSCRIBE_QOS1,      
            
    /**<Subscribe to QoS2 Topics */        
    SYS_RNWF_MQTT_SUBSCRIBE_QOS2,       
            
    /**<Publis to MQTT Broker*/
    SYS_RNWF_MQTT_PUBLISH,              
            
    /**<Configure the MQTT Application Callback*/            
    SYS_RNWF_MQTT_SET_CALLBACK,                
            
    /**<Configure the MQTT Application Callback*/
    SYS_RNWF_MQTT_SET_SRVC_CALLBACK,   
            
    /*< Get Callback Function data*/
    SYS_RNWF_MQTT_GET_CALLBACK,
            
}SYS_RNWF_MQTT_SERVICE_t;


/**
 @brief MQTT Application callback events
 
 */
typedef enum
{
    /**<Connected to MQTT broker event */
    SYS_RNWF_MQTT_CONNECTED,    
            
    /**<Disconnected from MQTT broker event*/   
    SYS_RNWF_MQTT_DISCONNECTED, 
            
    /**<Event to report received MQTT message*/   
    SYS_RNWF_MQTT_SUBCRIBE_MSG,  
            
    /*Subscribe MQTT ACK*/        
    SYS_RNWF_MQTT_SUBCRIBE_ACK,
            
    /*MQTT Public ACK*/
    SYS_RNWF_MQTT_PUBLIC_ACK,     
            
    /*MQTT DPS Status*/
    SYS_RNWF_MQTT_DPS_STATUS,    
	   
}SYS_RNWF_MQTT_EVENT_t;

/**
 @brief Network and Socket service List
 
 */
typedef struct 
{    
    /**<MQTT Broker/Server URL */    
    const char *url;          
    
    /**<MQTT Service client ID*/
    const char *clientid;
    
    /**<MQTT User Name Credential */
    const char *username;       
    
    /**<MQTT Password Credential */ 
    const char *password;       
    
    /**<MQTT Broker/Server Port */
    uint16_t port;          
    
    /*MQTT TLS Index*/
    uint8_t     tls_idx;
    
    /*Azure DPS*/
    uint8_t     azure_dps;
    
    /*TLS Configuration*/
    uint8_t     *tls_conf;    

    /*MQTT Protocol Version*/
    uint8_t     protoVer;

    /*MQTT Keep Alive time */
    uint16_t     keep_alive_time;


}SYS_RNWF_MQTT_CFG_t;

/**
 @brief Network and Socket service List
 
 */
typedef enum
{
    /**New message*/
    SYS_RNWF_NEW_MSG,         
            
    /**Duplicate message*/         
    SYS_RNWF_DUP_MSG      

}SYS_RNWF_MQTT_MSG_t;

/**
 @brief MQTT Message QoS Type
 
 */
typedef enum
{
    /**<No-Ack, Best effort delivery(No Guarantee)*/          
    SYS_RNWF_MQTT_QOS0,      
            
    /**<Pub-Ack, sent untill PUBACK from broker(possible duplicates) */
    SYS_RNWF_MQTT_QOS1,      
            
    /**<Highest service, no duplicate with guarantee */            
    SYS_RNWF_MQTT_QOS2,   

}SYS_RNWF_MQTT_QOS_t;

/**
 @brief MQTT Message Retain flag
 */
typedef enum
{
    /**<Publish message is not saved at broker */
    SYS_RNWF_NO_RETAIN,          
            
    /**<Publish message is saved at broker */        
    SYS_RNWF_RETAIN,  

}SYS_RNWF_MQTT_RETAIN_t;

/**
 @brief MQTT Publish Frame format
 
 */
typedef struct
{
    /**<Indicates message is new or duplicate */
    SYS_RNWF_MQTT_MSG_t isNew;          
    
    /**<QoS type for the message ::SYS_RNWF_MQTT_QOS_t */
    SYS_RNWF_MQTT_QOS_t qos;         
    
    /**<Retain flag for the publish message */
    SYS_RNWF_MQTT_RETAIN_t isRetain;    
    
    /**<Publish topic for the message */
    const char *topic;           
    
    /**<Indicates message is new or duplicate */
    const char *message; 
                       
}SYS_RNWF_MQTT_FRAME_t;


/**
 @brief MQTT Callback Function definition
 
 */
typedef SYS_RNWF_RESULT_t (*SYS_RNWF_MQTT_CALLBACK_t)(SYS_RNWF_MQTT_EVENT_t, uint8_t *);


/**
 * @brief MQTT Service Layer API to handle system operations.
 * 
 *
 * @param[in] request       Requested service ::SYS_RNWF_MQTT_SERVICE_t
 * @param[in] input         Input/Output data for the requested service 
 * 
 * @return SYS_RNWF_PASS Requested service is handled successfully
 * @return SYS_RNWF_PASS Requested service has failed
 */
SYS_RNWF_RESULT_t SYS_RNWF_MQTT_SrvCtrl( SYS_RNWF_MQTT_SERVICE_t request, void *input);

#endif	/* XC_HEADER_TEMPLATE_H */

/** @}*/
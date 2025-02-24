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
#define SYS_RNWF_MQTT_CMD_SUBSCRIBE_QOS    "AT+MQTTSUB=\"%s\",%d\r\n"
#define SYS_RNWF_MQTT_CMD_UNSUBSCRIBE       "AT+MQTTUNSUB=%s\r\n"

/* MQTT Publish Commands */
#define SYS_RNWF_MQTT_CMD_PUBLISH           "AT+MQTTPUB=%d,%d,%d,\"%s\",\"%s\"\r\n"

/* MQTT LWT Commands */
#define SYS_RNWF_MQTT_LWT_CMD               "AT+MQTTLWT=%d,%d,\"%s\",\"%s\"\r\n"

/* MQTT Transmit Properties  Commands */
#define SYS_RNWF_MQTT_SET_TX_PAYLOD_FORMAT_IND      "AT+MQTTPROPTX=1,%d\r\n"
#define SYS_RNWF_MQTT_SET_TX_MSG_EXPIRY            "AT+MQTTPROPTX=2,%d\r\n"
#define SYS_RNWF_MQTT_SET_TX_CONTENT_TYPE           "AT+MQTTPROPTX=3,\"%s\"\r\n"         
#define SYS_RNWF_MQTT_SET_TX_SESSION_EXPIRY         "AT+MQTTPROPTX=17,%d\r\n"
#define SYS_RNWF_MQTT_SET_TX_USER_PROP              "AT+MQTTPROPTX=38,\"%s\",\"%s\"\r\n"

/* MQTT Transmit Properties Select Commands */
#define SYS_RNWF_MQTT_SET_TX_PAYLOD_FORMAT_IND_SEC      "AT+MQTTPROPTXS=1,1\r\n"
#define SYS_RNWF_MQTT_SET_TX_MSG_EXPIRY_SEC             "AT+MQTTPROPTXS=2,1\r\n"
#define SYS_RNWF_MQTT_SET_TX_CONTENT_TYPE_SEC           "AT+MQTTPROPTXS=3,1\r\n"         
#define SYS_RNWF_MQTT_SET_TX_SESSION_EXPIRY_SEC         "AT+MQTTPROPTXS=17,1\r\n"
#define SYS_RNWF_MQTT_SET_TX_USER_PROP_SEC              "AT+MQTTPROPTXS=38,1\r\n"

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


/* Handle for MQTT Configurations */
typedef void* SYS_RNWF_MQTT_HANDLE_t;

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

    /**<Configure the MQTT transmit parameters*/        
    SYS_RNWF_MQTT_TX_CONFIG,

    /**<Configure the MQTT LWT parameters*/
    SYS_RNWF_MQTT_LWT_CONFIG,

    /**<Connect to the MQTT Broker */        
    SYS_RNWF_MQTT_CONNECT,              
            
    /**<Request reconnect to the MQTT Cloud*/        
    SYS_RNWF_MQTT_RECONNECT,        
            
    /**<Trigger Disconnect from MQTT Broker*/        
    SYS_RNWF_MQTT_DISCONNECT,      

    /**<Subscribe to QoS Topics */        
    SYS_RNWF_MQTT_SUBSCRIBE_QOS,
            
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
 @brief MQTT Payload Format Indicator
 
 */
typedef enum
{
    /**<unspecified byte stream*/          
    unspecified,
            
    /**<UTF-8 encoded payload*/
    UTF8_encoded,

}SYS_RNWF_MQTT_PAYLOD_FORMAT_INDI;


/**
 @brief MQTT User property Structure
 
 */
typedef struct
{
    uint8_t *key;
    
    uint8_t *value;
    
}SYS_RNWF_MQTT_USER_PROP;


/**
 @brief MQTT Configuration parameters
 
 */
typedef struct 
{   
    /**<MQTT Payload format indicati */    
    SYS_RNWF_MQTT_PAYLOD_FORMAT_INDI paylod;
    
    /**<MQTT Service message expiry interval time*/
    uint16_t    msgExpInt;
    
    /**<MQTT content type */
    const char *contentType;       
    
    /**<MQTT response Topic */ 
    const char *responseTopic;       

    /**<MQTT correlation data */ 
    const char *correlationData;
    
    /**<MQTT subscription ID */
    uint32_t    subscriptionId;          
    
    /*MQTT session expiry interval time*/
    uint32_t     sessionExpInt;
    
    /*MQTT Will Delay Interval*/
    uint16_t     willDelayInt;
    
    /*TLS Configuration*/
    uint8_t     *tlsConf;    

    /*MQTT  Receive Maximum*/
    uint32_t     receiveMax;

    /*MQTT topic alias max */
    uint32_t     tpoicAliasMax;

    /*MQTT topic alias */
    uint32_t     tpoicAlias;
    
    /*MQTT user property */
    SYS_RNWF_MQTT_USER_PROP userProp;
    
}SYS_RNWF_MQTT_TX_CFG_t;


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
 @brief MQTT Subscribe Frame format
 
 */
typedef struct
{
    /**<QoS type for the message ::SYS_RNWF_MQTT_QOS_t */
    SYS_RNWF_MQTT_QOS_t qos;         
    
    /**<Publish topic for the message */
    const char *topic;           
                       
}SYS_RNWF_MQTT_SUB_FRAME_t;

/**
 @brief MQTT LWT config format

 */
typedef struct
{
    /**<QoS type for the LWT message ::SYS_RNWF_MQTT_QOS_t */
    SYS_RNWF_MQTT_QOS_t qos;
    
    /**<Retain flag for the LWT message */
    SYS_RNWF_MQTT_RETAIN_t isRetain; 
    
    /**<Topic name of the LWT message */
    const char *topic_name; 
    
    /**<LWT message */
    const char *message;
    
}SYS_RNWF_MQTT_LWT_CFG_t;



/**
 @brief MQTT Callback Function definition
 
 */
typedef SYS_RNWF_RESULT_t (*SYS_RNWF_MQTT_CALLBACK_t)(SYS_RNWF_MQTT_EVENT_t, SYS_RNWF_MQTT_HANDLE_t);


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
SYS_RNWF_RESULT_t SYS_RNWF_MQTT_SrvCtrl( SYS_RNWF_MQTT_SERVICE_t request, SYS_RNWF_MQTT_HANDLE_t );

#endif	/* XC_HEADER_TEMPLATE_H */

/** @}*/
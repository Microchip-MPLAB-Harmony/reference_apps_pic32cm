/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#define VCNL4200_SLAVE_ADDR       0x51

#define VCNL4200_Device_ID        0x0E
#define VCNL4200_Device_ID_value  0x58

#define VCNL4200_PS_CONF1_REG     0x03
#define VCNL4200_PS_CONF3_REG     0x04
#define VCNL4200_ALS_CONF_REG     0x00

#define VCNL4200_PS_DATA_REG      0x08
#define VCNL4200_ALS_DATA_REG     0x09

/* PS_CONF1 and PS_CONF2 Bits */
#define PS_ON					  (0 << 0) /* PS power on */
#define PS_IT_8T				  (0 << 1)|(0 << 2)|(1 << 3) /* PS integration time 8T */
#define PS_HD_16				  (1 << 11) /* PS output is 16 bits */

/* PS_CONF3 and PS_MS Bits */
#define PS_MPS_8				  (1 << 5)|(1 << 6) /* Proximity multi pulse number 8 */
#define PS_MS_OUTPUT			  (1 << 13) /* Proximity detection logic output mode enable */

/* ALS_CONF Bits*/
#define ALS_ON					  (0 << 0) /* ALS power on */
#define ALS_IT_50				  (0 << 6)|(0 << 7) /*ALS integration time setting 50 ms*/

/*Configuration for PS_CONF1_REG*/
#define VCNL4200_PS_CONF1_VAL     (PS_ON |PS_IT_8T| PS_HD_16)
/*Configuration for PS_CONF3_REG*/
#define VCNL4200_PS_CONF3_VAL     (PS_MPS_8 | PS_MS_OUTPUT)
/*Configuration for ALS_CONF_REG*/
#define VCNL4200_ALS_CONF_VAL     (ALS_ON | ALS_IT_50)

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

static volatile bool transferDone = false;

uint16_t PS_Data; //Proximity Sensor Data
uint16_t ALS_Data; //Ambient Light Sensor Data

static uint16_t VCNL4200_WriteRead(uint8_t reg_addr)
{
    uint8_t rxData[2];
    transferDone = false;
    
    SERCOM2_I2C_WriteRead(VCNL4200_SLAVE_ADDR, &reg_addr, 1, rxData, 2);
    
    while ( transferDone != true);
    
    return (rxData[1] << 8) | rxData[0]; 
}

static bool VCNL4200_Write(uint8_t reg_addr, uint16_t value)
{
    uint8_t data[3];
    
    data[0] = reg_addr;
    data[1] = value & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    
    transferDone = false;
    
    SERCOM2_I2C_Write(VCNL4200_SLAVE_ADDR,data,3);
    
    while ( transferDone != true);
    
    return true;
}

static bool VCNL4200_Initialize()
{
    uint16_t read_reg;
    read_reg = VCNL4200_WriteRead(VCNL4200_Device_ID);
    //Device ID
    if((read_reg & 0x00FF) != VCNL4200_Device_ID_value)
    {
        return false;
    }
    //Proximity Sensor (PS) Configuration 1
    if(VCNL4200_Write(VCNL4200_PS_CONF1_REG,  VCNL4200_PS_CONF1_VAL) != true)
    {
        return false;
    }
    //Proximity Sensor (PS) Configuration 3
    if(VCNL4200_Write(VCNL4200_PS_CONF3_REG, VCNL4200_PS_CONF3_VAL) != true)
    {
        return false;
    }
    //Ambient Light Sensor (ALS) Configuration
    if(VCNL4200_Write(VCNL4200_ALS_CONF_REG, VCNL4200_ALS_CONF_VAL) != true)
    {
        return false;
    }
    return true;
}

static void i2cEventHandler(uintptr_t contextHandle)
{
    //Transfer completed
    if (SERCOM2_I2C_ErrorGet() == SERCOM_I2C_ERROR_NONE)
    {
        transferDone = true;
    }
    
    //Transfer not completed
    else
    {
        transferDone = false;
    }
}

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    SYSTICK_TimerInitialize();
    
    SERCOM2_I2C_CallbackRegister(i2cEventHandler, 0);
    
    if(VCNL4200_Initialize()== true)
    {
        printf("\n\rVCNL4200 Initialization successful\n\r");
    }
    else
    {
        printf("\n\rVCNL4200 Initialization failed\n\r");
    }
    
    SYSTICK_TimerStart();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        if(transferDone == true)
        {
            transferDone = false;
            
            PS_Data = VCNL4200_WriteRead(VCNL4200_PS_DATA_REG);
            ALS_Data = VCNL4200_WriteRead(VCNL4200_ALS_DATA_REG);
            /*Print values for Proximity Sensor and Ambient Light Sensor*/
            printf("\rProximity Sensor: %d, Ambient Light Sensor: %d \t",PS_Data,ALS_Data);
            
            SYSTICK_DelayMs(100U);
        }
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/
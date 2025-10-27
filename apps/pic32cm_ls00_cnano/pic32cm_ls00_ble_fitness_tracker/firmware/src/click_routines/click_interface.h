/*******************************************************************************
  Click interface header file

  Company:
    Microchip Technology Inc.

  File Name:
    click_interface.h

  Summary:

  Description:
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CLICK_INTERFACES_H
#define CLICK_INTERFACES_H

/**
  Section: Included Files
 */

#include <xc.h>
#include <stdint.h>
#include <rnbd/rnbd.h>
#include "rnbd/rnbd_interface.h"
#include "../config/default/definitions.h"

/** Click routine interfaces/resources Macro Declarations
 *
 *  1. SPI
 *  The EINK_EPAPER_2_9_296_128  click example on SAM E51 Curiosity Nano uses
 *  mikroBUS socket #1. Connect EINK_EPAPER_2_9_296_128 to eINK click then mount eINK click on
 *  mikroBUS socket #1.
 *  The SPI lines from MCU coming to this socket are from
 *  SERCOM1 peripheral. The SPI is configured to use manual chip select.
 *
 *
 * 2. Timer
 *  The EINK_EPAPER_2_9_296_128  click  example on SAM E51 Curiosity Nano uses
 *  Systick timer module on the MCU to implement the time
 *  requirement weather click routines.
 *
 * 3. PORTs
 *  The EINK_EPAPER_2_9_296_128  click uses the following ports pins.
 *  EPAPER_2_9_296_128_DC
 *  EPAPER_2_9_296_128_RST
 *  EPAPER_2_9_296_128_CS
 *  EPAPER_2_9_296_128_BSY
 *  The pins are configured using the MCC Pin configurator.
 */

// SPI Definitions
#define EPAPER_2_9_296_128_SPI_Write                 SERCOM2_SPI_Write
#define EPAPER_2_9_296_128_SPI_Read                  SERCOM2_SPI_Read
#define EPAPER_2_9_296_128_SPI_WriteRead             SERCOM2_SPI_WriteRead

// Timer Definitions
#define EPAPER_2_9_296_128_TimerStart                SYSTICK_TimerStart
#define EPAPER_2_9_296_128_DelayMs                   SYSTICK_DelayMs
/** Click routine interfaces/resources Macro Declarations
 *
 *  1. USART
 *  The heartrate9 click example on SAM E51 Curiosity Nano uses
 *  mikroBUS socket #3 on the Curiosity Nano Base for Click boards
 *  to mount heartrate9 click board. The USART lines from MCU coming to
 *  this socket are from SERCOM4 peripheral on the MCU.
 *
 * 2. Timer
 *  The heartrate9 click example on SAM E51 Curiosity Nano uses
 *  Systick timer module on the MCU to implement the time
 *  requirement of heartrate9 click routines.
 *
 */

// SERCOM USART Definitions
#define CLICK_HEARTRATE9_USART_Read                          SERCOM0_USART_Read
#define CLICK_HEARTRATE9_USART_ReadThresholdSet              SERCOM0_USART_ReadThresholdSet
#define CLICK_HEARTRATE9_USART_ReadNotificationEnable        SERCOM0_USART_ReadNotificationEnable
#define CLICK_HEARTRATE9_USART_ReadCallbackRegister          SERCOM0_USART_ReadCallbackRegister
#define CLICK_HEARTRATE9_USART_ReadCountGet                  SERCOM0_USART_ReadCountGet
#define CLICK_HEARTRATE9_USART_EVENT_READ_THRESHOLD_REACHED  SERCOM_USART_EVENT_READ_THRESHOLD_REACHED
#define CLICK_HEARTRATE9_USART_READ_BUFFER_SIZE              128                 //Define value same as SERCOM4_USART_READ_BUFFER_SIZE defined in the respective USART PLIB '.c' file

// Timer Definitions
#define CLICK_HEARTRATE9_TimerStart                         SYSTICK_TimerStart
#define CLICK_HEARTRATE9_DelayMs                            SYSTICK_DelayMs

#define CLICK_RNBD451_Init                                  RNBD_Init
#define CLICK_RNBD_EnterCmdMode                             RNBD_EnterCmdMode
#define CLICK_RNBD_FactoryReset                             RNBD_FactoryReset
#define CLICK_RNBD_SetName                                  RNBD_SetName
#define CLICK_RNBD_SetServiceBitmap                         RNBD_SetServiceBitmap
#define CLICK_RNBD_SetAppearance                            RNBD_SetAppearance
#define CLICK_RNBD_RebootCmd                                RNBD_RebootCmd
#define CLICK_RNBD_SetServiceUUID                           RNBD_SetServiceUUID
#define CLICK_RNBD_SetServiceCharacteristic                 RNBD_SetServiceCharacteristic
#define CLICK_RNBD_WriteLocalCharacteristic                 RNBD_WriteLocalCharacteristic

#endif // CLICK_INTERFACES_H

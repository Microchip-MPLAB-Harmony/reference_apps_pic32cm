/**
 * \file rnbd.h
 * \brief This file contains APIs to access features support by RNBD series devices.
 */
/*
    (c) 2023 Microchip Technology Inc. and its subsidiaries.
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/


#include <stdbool.h>
#include <stdint.h>
#include "../click_interface.h"

bool rnbd451_init(void);

bool rnbd451_enter_cmd_mode(void);

bool rnbd451_factory_reset(uint8_t resetMode);

bool rnbd451_set_name(const char *name, uint8_t nameLen);

bool rnbd451_set_service_bitmap(uint8_t serviceBitmap);

bool rnbd451_set_appearance(const char *appearance, uint8_t appearanceLen);

bool rnbd451_reboot_cmd(void);

bool rnbd451_set_service_uuid(const char *uuid, uint8_t uuidLen);

bool rnbd451_set_service_characteristic(const char *service, uint8_t serviceLen);

bool rnbd451_write_local_characteristic(const char *handle, uint8_t handleLen, const char *data, uint8_t dataLen);


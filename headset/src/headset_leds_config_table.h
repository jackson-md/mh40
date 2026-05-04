/*!
\copyright  Copyright (c) 2019 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file       headset_leds_config_table.h
\brief      Header file for the Headset led config table.
*/

#ifndef HEADSET_LEDS_CONFIG_TABLE_H
#define HEADSET_LEDS_CONFIG_TABLE_H

#include <csrtypes.h>
#include <ui_indicator_leds.h>

extern const ui_event_indicator_table_t headset_ui_leds_table[];
extern const ui_provider_context_consumer_indicator_table_t headset_ui_leds_context_indications_table[];
uint8 HeadsetLedsConfigTable_GetSize(void);
uint8 HeadsetLedsConfigTable_ContextsTableGetSize(void);

#ifdef ENABLE_APP_BATTERY_CHECK_LED
extern uint8 get_LED_Battery_High_LEDIndex(void);
extern uint8 get_LED_Battery_Medium_LEDIndex(void);
extern uint8 get_LED_Battery_Low_LEDIndex(void);
#endif

#ifdef ENABLE_APP_BATTERY_LOW_WARNING
extern uint8 get_LED_Battery_Low_Warning_LEDIndex(void);
#endif

#ifdef ENABLE_APP_OTA_LED
extern uint8 get_LED_APP_OTA_LEDIndex(void);
extern uint8 get_LED_App_Idle_Connected_LEDIndex(void);
#endif

#ifdef ENABLE_APP_ENTER_DUT_MODE
extern uint8 get_LED_App_Enter_DUT_LEDIndex(void);
#endif

#endif // HEADSET_LEDS_CONFIG_TABLE_H

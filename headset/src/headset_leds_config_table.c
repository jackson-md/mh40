/*!
\copyright  Copyright (c) 2020 - 2021 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    %%version
\file       headset_leds_config_table.c
\brief      Headset led configuration table module
*/
#include "headset_leds_config_table.h"
#include "headset_led.h"
#include "headset_sm.h"

#include <domain_message.h>
#include <ui_indicator_leds.h>
#include <pairing.h>
#include <charger_monitor.h>
#include <power_manager.h>
#include <telephony_messages.h>
#include <hfp_profile.h>
#include <av.h>
#include <media_player.h>
#include <voice_sources.h>

const ui_provider_context_consumer_indicator_table_t headset_ui_leds_context_indications_table[] =
{
    {.provider=ui_provider_handset_pairing,
     .context=context_handset_pairing_active,           { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_pairing,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_ringing_incoming,           { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_call_incoming,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_ringing_outgoing,           { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_sco,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_in_call,                    { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_sco,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_in_call_with_incoming,      { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_call_incoming,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_in_call_with_outgoing,      { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_sco,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_in_call_with_held,          { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_sco,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_telephony,
     .context=context_voice_in_multiparty_call,         { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_sco,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_media_player,
     .context=context_media_player_streaming,           { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_streaming,
                                                          .led.priority = LED_PRI_LOW}},
    {.provider=ui_provider_app_sm,
     .context=context_app_sm_idle,                      { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_idle,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},

    {.provider=ui_provider_app_sm,
     .context=context_app_sm_idle_connected,            { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_idle_connected,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
    {.provider=ui_provider_app_sm,
     .context=context_app_sm_exit_idle,                 { .led.action = LED_STOP_PATTERN,
                                                          .led.data.pattern = NULL,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE }},
};

const ui_event_indicator_table_t headset_ui_leds_table[] =
{
#if 0
    {.sys_event=POWER_ON,                               { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_power_on,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#endif

    {.sys_event=POWER_OFF,                              { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_power_off,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },

#ifdef ENABLE_APP_BATTERY_CHECK_LED
    {.sys_event=INN_BATTERY_LEVEL_HIGH,                 { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_high,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },

    {.sys_event=INN_BATTERY_LEVEL_MEDIUM,               { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_medium,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },

    {.sys_event=INN_BATTERY_LEVEL_LOW,                  { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_low,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },
#endif

#ifdef ENABLE_APP_BATTERY_LOW_WARNING
    {.sys_event=INN_BATTERY_LOW_WARNING_LED,            { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_low_warning,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },
#endif
#if 0
    {.sys_event=TELEPHONY_CALL_CONNECTION_FAILURE,      { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_error,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE}},

    {.sys_event=TELEPHONY_ERROR,                        { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_error,
                                                          .led.priority = LED_PRI_LOW,
                                                          .led.local_only = TRUE}},

    {.sys_event=AV_ERROR,                               { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_error,
                                                          .led.priority = LED_PRI_LOW}},
#endif

#if 0
    {.sys_event=CHARGER_MESSAGE_DETACHED,               { .led.action = LED_CANCEL_FILTER,
                                                          .led.data.filter = NULL,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE}},

    {.sys_event=CHARGER_MESSAGE_DISABLED,               { .led.action = LED_CANCEL_FILTER,
                                                          .led.data.filter = NULL,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE}},

    {.sys_event=CHARGER_MESSAGE_COMPLETED,              { .led.action = LED_SET_FILTER,
                                                          .led.data.filter = app_led_filter_charging_complete,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE}},

    {.sys_event=CHARGER_MESSAGE_CHARGING_OK,            { .led.action = LED_SET_FILTER,
                                                          .led.data.filter = app_led_filter_charging_ok,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE}},

    {.sys_event=CHARGER_MESSAGE_CHARGING_LOW,           { .led.action = LED_SET_FILTER,
                                                          .led.data.filter = app_led_filter_charging_low,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE}},
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
    {.sys_event=CHARGER_MESSAGE_ATTACHED,               { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charging_led,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },

    {.sys_event=CHARGER_MESSAGE_DETACHED,               { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charge_detached,
                                                          .led.priority = LED_PRI_MEDIUM,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },

    {.sys_event=INN_APP_BATTERY_CHARGE_COMPLETE,        { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charge_completed,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#if 0
    {.sys_event=CHARGER_MESSAGE_COMPLETED,              { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charge_completed,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#endif
#endif
    #if 0
    {.sys_event=CHARGER_MESSAGE_CHARGING_OK,            { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charging_led,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },

    {.sys_event=CHARGER_MESSAGE_CHARGING_LOW,           { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charging_led,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#endif

#ifdef ENABLE_APP_ENTER_DUT_MODE
    {.sys_event=INN_ENTER_DUT_LED,                      { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_DUT,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },
#endif

#ifdef ENABLE_APP_OTA_LED
    {.sys_event=INN_OTA_PROCESSING_LED,                            { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_OTA,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },

    {.sys_event=INN_OTA_COMPLETE_LED,                   { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_battery_charge_detached,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE},
                                                          .await_indication_completion = TRUE },
#endif

#ifdef ENABLE_APP_FACTORY_RESET
    {.sys_event=INN_APP_FACTORY_RESET,                  { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_factory_reset,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#endif

#if 0
    {.sys_event=TELEPHONY_CONNECTED,                    { .led.action = LED_START_PATTERN,
                                                          .led.data.pattern = app_led_pattern_idle_connected,
                                                          .led.priority = LED_PRI_EVENT,
                                                          .led.local_only = TRUE },
                                                          .await_indication_completion = TRUE },
#endif
};

#ifdef ENABLE_APP_BATTERY_CHECK_LED
uint8 get_LED_Battery_High_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_BATTERY_LEVEL_HIGH)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_Battery_High_LEDIndex : %d", index);
    return index;
}

uint8 get_LED_Battery_Medium_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_BATTERY_LEVEL_MEDIUM)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_Battery_Medium_LEDIndex : %d", index);
    return index;
}

uint8 get_LED_Battery_Low_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_BATTERY_LEVEL_LOW)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_Battery_Low_LEDIndex : %d", index);
    return index;
}
#endif

#ifdef ENABLE_APP_BATTERY_LOW_WARNING
uint8 get_LED_Battery_Low_Warning_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_BATTERY_LOW_WARNING_LED)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_Battery_Low_Warning_LEDIndex : %d", index);
    return index;
}
#endif

#ifdef ENABLE_APP_OTA_LED
uint8 get_LED_APP_OTA_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_OTA_PROCESSING_LED)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_APP_OTA_LEDIndex : %d", index);
    return index;
}

uint8 get_LED_App_Idle_Connected_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_OTA_COMPLETE_LED)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_App_Idle_Connected_LEDIndex : %d", index);
    return index;
}
#endif

#ifdef ENABLE_APP_ENTER_DUT_MODE
uint8 get_LED_App_Enter_DUT_LEDIndex(void)
{
    uint8 index = 0;
    uint8 i;

    for(i=0; i< sizeof(headset_ui_leds_table)/sizeof(ui_event_indicator_table_t); i++)
    {
        if(headset_ui_leds_table[i].sys_event == INN_ENTER_DUT_LED)
        {
            index = i;
            break;
        }
    }

    DEBUG_LOG_INFO("get_LED_blink_4times_LEDIndex : %d", index);
    return index;
}
#endif

uint8 HeadsetLedsConfigTable_GetSize(void)
{
    return ARRAY_DIM(headset_ui_leds_table);
}

uint8 HeadsetLedsConfigTable_ContextsTableGetSize(void)
{

    return ARRAY_DIM(headset_ui_leds_context_indications_table);
}

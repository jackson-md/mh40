/*!
\copyright  Copyright (c) 2020 - 2021 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    %%version
\file       headset_prompts_config_table.c
\brief      Headset prompts configuration table module
*/
#include "headset_prompts_config_table.h"

#include <domain_message.h>
#include <ui_indicator_prompts.h>
#include <av.h>
#include <pairing.h>
#include <handset_service_protected.h>
#include <power_manager.h>
#include <voice_ui.h>

#ifdef ENABLE_APP_INCOMMING_RINGTONE
#include "telephony_messages.h"
#endif

#ifdef INCLUDE_PROMPTS
const ui_event_indicator_table_t headset_ui_prompts_table[] =
{
#ifndef EXCLUDE_POWER_PROMPTS
    {.sys_event=POWER_ON,                {.prompt.filename = PROMPT_FILENAME("MD_Power_On_48K"),
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = FALSE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE },
                                          .await_indication_completion = TRUE },
    {.sys_event=POWER_OFF,              { .prompt.filename = PROMPT_FILENAME("MD_Power_Off_48K"),
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = FALSE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE },
                                          .await_indication_completion = TRUE },
#endif
    {.sys_event=PAIRING_ACTIVE,         { .prompt.filename = PROMPT_FILENAME("MD_Pairing_48K"),
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = TRUE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE }},
#if 0
    {.sys_event=PAIRING_COMPLETE,       { .prompt.filename = "MD_052318_Connect_48K.sbc",
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = FALSE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE }},

    {.sys_event=PAIRING_FAILED,         { .prompt.filename = "MD_052318_Disconnect_48K.sbc",
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = FALSE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE }},
#endif

#if 1
    {.sys_event=TELEPHONY_CONNECTED,                            { .prompt.filename = PROMPT_FILENAME("MD_Connect_48K"),
                                                                  .prompt.rate = 48000,
                                                                  .prompt.format = PROMPT_FORMAT,
                                                                  .prompt.interruptible = FALSE,
                                                                  .prompt.queueable = TRUE,
                                                                  .prompt.requires_repeat_delay = FALSE }},
    {.sys_event=TELEPHONY_DISCONNECTED,                         { .prompt.filename = PROMPT_FILENAME("MD_Disconnect_48K"),
                                                                  .prompt.rate = 48000,
                                                                  .prompt.format = PROMPT_FORMAT,
                                                                  .prompt.interruptible = FALSE,
                                                                  .prompt.queueable = TRUE,
                                                                  .prompt.requires_repeat_delay = FALSE }},

#else
#ifndef EXCLUDE_CONN_PROMPTS
    {.sys_event=HANDSET_SERVICE_FIRST_PROFILE_CONNECTED_IND,    { .prompt.filename = PROMPT_FILENAME("MD_Connect_48K"),
                                                                  .prompt.rate = 48000,
                                                                  .prompt.format = PROMPT_FORMAT,
                                                                  .prompt.interruptible = FALSE,
                                                                  .prompt.queueable = TRUE,
                                                                  .prompt.requires_repeat_delay = FALSE }},

    {.sys_event=HANDSET_SERVICE_DISCONNECTED_IND,               { .prompt.filename = PROMPT_FILENAME("MD_Disconnect_48K"),
                                                                  .prompt.rate = 48000,
                                                                  .prompt.format = PROMPT_FORMAT,
                                                                  .prompt.interruptible = FALSE,
                                                                  .prompt.queueable = TRUE,
                                                                  .prompt.requires_repeat_delay = FALSE }},
#endif
#endif

#ifdef INCLUDE_GAA
    {.sys_event=VOICE_UI_MIC_OPEN,      { .prompt.filename = PROMPT_FILENAME("mic_open"),
                                          .prompt.rate = 16000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = TRUE,
                                          .prompt.queueable = FALSE,
                                          .prompt.requires_repeat_delay = FALSE }},
    {.sys_event=VOICE_UI_MIC_CLOSE,     { .prompt.filename = PROMPT_FILENAME("mic_close"),
                                          .prompt.rate = 16000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = TRUE,
                                          .prompt.queueable = FALSE,
                                          .prompt.requires_repeat_delay = FALSE }},
    {.sys_event=VOICE_UI_DISCONNECTED,  { .prompt.filename = PROMPT_FILENAME("bt_va_not_connected"),
                                          .prompt.rate = 48000,
                                          .prompt.format = PROMPT_FORMAT,
                                          .prompt.interruptible = FALSE,
                                          .prompt.queueable = TRUE,
                                          .prompt.requires_repeat_delay = TRUE }}
#endif /* INCLUDE_GAA */


#ifdef ENABLE_APP_BATTERY_LOW_WARNING
    {.sys_event=INN_BATTERY_LOW_WARNING_PROMPT,     { .prompt.filename = PROMPT_FILENAME("MD_LowBat_48K"),
                                                      .prompt.rate = 48000,
                                                      .prompt.format = PROMPT_FORMAT,
                                                      .prompt.interruptible = FALSE,
                                                      .prompt.queueable = TRUE,
                                                      .prompt.requires_repeat_delay = TRUE }},
#endif

#ifdef ENABLE_APP_INCOMMING_RINGTONE
    {.sys_event=INN_TELEPHONY_INCOMING_RINGTONE,    { .prompt.filename = PROMPT_FILENAME("MD_Ring_48K"),
                                                      .prompt.rate = 48000,
                                                      .prompt.format = PROMPT_FORMAT,
                                                      .prompt.interruptible = FALSE,
                                                      .prompt.queueable = TRUE,
                                                      .prompt.requires_repeat_delay=FALSE }},
#endif

#ifdef ENABLE_APP_FACTORY_RESET
    {.sys_event=INN_APP_FACTORY_RESET,              {.prompt.filename = PROMPT_FILENAME("MD_FactoryReset_48K"),
                                                      .prompt.rate = 48000,
                                                      .prompt.format = PROMPT_FORMAT,
                                                      .prompt.interruptible = FALSE,
                                                      .prompt.queueable = TRUE,
                                                      .prompt.requires_repeat_delay = TRUE },
                                                      .await_indication_completion = TRUE },
#endif

#ifdef ENABLE_APP_MIC_MUTE
    {.sys_event=INN_APP_MIC_MUTE_PROMPT,            { .prompt.filename = PROMPT_FILENAME("MD_MicMute_48K"),
                                                      .prompt.rate = 48000,
                                                      .prompt.format = PROMPT_FORMAT,
                                                      .prompt.interruptible = FALSE,
                                                      .prompt.queueable = TRUE,
                                                      .prompt.requires_repeat_delay = FALSE }},

    {.sys_event=INN_APP_MIC_UNMUTE_PROMPT,          { .prompt.filename = PROMPT_FILENAME("MD_MicUnmute_48K"),
                                                      .prompt.rate = 48000,
                                                      .prompt.format = PROMPT_FORMAT,
                                                      .prompt.interruptible = FALSE,
                                                      .prompt.queueable = TRUE,
                                                      .prompt.requires_repeat_delay = FALSE }},
#endif
};

#endif
uint8 HeadsetPromptsConfigTable_GetSize(void)
{
        #ifdef INCLUDE_PROMPTS
            return ARRAY_DIM(headset_ui_prompts_table);
        #else
            return 0;
        #endif
}


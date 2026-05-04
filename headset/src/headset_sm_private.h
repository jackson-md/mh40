/*!
\copyright  Copyright (c) 2019 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    %%version
\file       headset_sm_private.h
\brief      Internal interface for SM components.
*/

#ifndef HEADSET_SM_PRIVATE_H_
#define HEADSET_SM_PRIVATE_H_

/*! \brief Application state machine internal message IDs */
enum sm_internal_message_ids
{
    SM_INTERNAL_PAIR_HANDSET,                       /*!< Start pairing with a handset. */
    SM_INTERNAL_DELETE_HANDSETS,                    /*!< Delete all paired handsets. */
    SM_INTERNAL_FACTORY_RESET,                      /*!< Reset device to factory defaults. */
    SM_INTERNAL_LINK_DISCONNECTION_COMPLETE,        /*!< All links are now disconnected */
    SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION,         /*!< Timeout if link disconnection takes too long. */
    SM_INTERNAL_POWER_OFF,                          /*!< power OFF headset */
    SM_INTERNAL_TIMEOUT_IDLE,                       /*!< Timeout when idle and no bt connection */
    SM_INTERNAL_TIMEOUT_LIMBO,                      /*!< Timeout when in limbo state and no charger connected*/

#ifdef ENABLE_APP_LINE_IN_AUDIO
    SM_INTERNAL_APP_LINE_IN_DETECH,
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
    SM_INTERNAL_NTC_CHARGER_COMPLETED_HANDLE,
    SM_INTERNAL_CHARGER_COMPLETED_HANDLE,
#endif

#ifdef ENABLE_APP_MD_GAIA
    SM_INTERNAL_REBOOT,
#endif

#ifdef ENABLE_APP_HID_COMMAND
    SM_INTERNAL_APP_DISABLE_USB_AUDIO,
#endif

#ifdef ENABLE_APP_POWERON_ENTER_PAIRING
    SM_INTERNAL_POWER_ON_300MS_TIMER,
#endif

#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
    SM_INTERNAL_PAIRING_PROMPTS_CYCLE,
#ifdef ENABLE_APP_FACTORY_RESET
    SM_INTERNAL_APP_FACTORY_RESET_HANDLE,
#endif
#endif

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
    SM_INTERNAL_POWER_ON_3S_TIMER,
    SM_INTERNAL_FORCE_POWER_OFF,
#endif

#ifdef ENABLE_APP_FIX_PO_NOISE
    SM_INTERNAL_APP_MUTE_MAIN_VOLUME,
    SM_INTERNAL_APP_UNMUTE_MAIN_VOLUME,
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
    SM_INTERNAL_APP_GAIA_CMD_CONNECT_DEVICE_TIMER,
#endif
};

#endif /* HEADSET_SM_PRIVATE_H_ */

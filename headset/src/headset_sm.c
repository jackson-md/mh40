/*!
\copyright  Copyright (c) 2019 - 2022 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.

\version    %%version
\file       headset_sm.c
\brief     headset application SM .

*/

/* local includes */
#include "headset_sm.h"
#include "headset_sm_private.h"
#include "adk_log.h"
#include "headset_led.h"
#include "wired_audio_source.h"
#include "headset_test.h"
#include "headset_ui_config.h"
#include "headset_usb.h"
#include "headset_wired_audio_controller.h"

/* framework includes */
#include <ui.h>
#include <unexpected_message.h>
#include <connection_manager.h>
#include <connection_manager_config.h>
#include <hfp_profile.h>
#include <av.h>
#include <power_manager.h>
#include <power_manager_conditions.h>
#include <charger_monitor.h>
#include <stereo_topology.h>
#include <pairing.h>
#include <device.h>
#include <device_properties.h>
#include <dfu.h>
#include <gaia_framework.h>
#include <handset_service.h>
#include <device_list.h>
#include <system_state.h>
#include <hfp_profile_config.h>
#include <usb_device.h>
#include <gatt_server_gatt.h>
#include <anc_state_manager.h>
#include <aec_leakthrough.h>
#include <system_reboot.h>
#include <ps_key_map.h>

#ifdef INCLUDE_FAST_PAIR
#include <fast_pair.h>
#endif

/* system includes */
#include <vm.h>
#include <vmtypes.h>
#include <panic.h>
#include <message.h>
#include <ps.h>

#include <telephony_messages.h>

#ifdef ENABLE_APP_LINE_IN_AUDIO
#include "wired_audio_private.h"
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
#include "headset_init.h"
#include "battery_region.h"
#include "state_of_charge.h"
#include "battery_monitor_config.h"
#endif

#ifdef ENABLE_APP_MD_GAIA
#include "local_name.h"
#include "tym_ps.h"
#endif

#ifdef ENABLE_APP_BATTERY_CHECK_LED
#include "battery_monitor.h"
#include "ui_indicator_leds.h"
#include "headset_leds_config_table.h"
#endif

#ifdef ENABLE_APP_BATTERY_LOW_WARNING
#include "battery_monitor.h"
#endif

#ifdef ENABLE_APP_POWERON_ENTER_PAIRING
#include "pio_common.h"
#endif

#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
#include "ui_indicator_prompts.h"
#include "kymera_tones_prompts.h"
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
#include "voice_ui_gaia_plugin.h"
#endif

/* Make the type used for message IDs available in debug tools */
LOGGING_PRESERVE_MESSAGE_ENUM(sm_internal_message_ids)

/*! \name Connection library factory reset keys

    These keys should be deleted during a factory reset.
*/
/*! @{ */
#define ATTRIBUTE_BASE_PSKEY_INDEX  100
#define TDL_BASE_PSKEY_INDEX        142
#define TDL_INDEX_PSKEY             141
#define TDL_SIZE                    DeviceList_GetMaxTrustedDevices()
/*! @} */

/*!< headset state machine. */
smTaskData headset_sm;

const message_group_t sm_ui_inputs[] =
{
    UI_INPUTS_HANDSET_MESSAGE_GROUP,
    UI_INPUTS_DEVICE_STATE_MESSAGE_GROUP,
    UI_INPUTS_APP_MESSAGE_GROUP
};

/*! \brief Get the state machine disconnect lock. */
#define headsetSmGetDisconnectLock() (SmGetTaskData()->disconnect_lock)
/*! \brief Clear the disconnect lock */
#define headsetSmClearDisconnectLock() (headsetSmGetDisconnectLock() = FALSE)
/*! \brief Set the disconnect lock */
#define headsetSmSetDisconnectLock() (headsetSmGetDisconnectLock() = TRUE)
/*! @brief Query if pairing has been initiated by the user. */
#define headsetSmIsUserPairing() (SmGetTaskData()->user_pairing)
/*! @brief Set user initiated pairing flag. */
#define headsetSmSetUserPairing()  (SmGetTaskData()->user_pairing = TRUE)
/*! @brief Clear user initiated pairing flag. */
#define headsetSmClearUserPairing()  (SmGetTaskData()->user_pairing = FALSE)
/*! @brief Query auto poweron */
#define headsetSmGetAutoPowerOn() (SmGetTaskData()->auto_poweron)
/*! @brief Set headset auto poweron flag. */
#define headsetSmSetAutoPowerOn()  (SmGetTaskData()->auto_poweron = TRUE)
/*! @brief Clear auto poweron flag. */
#define headsetSmClearAutoPowerOn()  (SmGetTaskData()->auto_poweron = FALSE)

/*! \brief Return TRUE if the state is idle */
#define headsetSmStateIsIdle(state) (state == HEADSET_STATE_IDLE)

#ifdef ALLOW_USB_BT_COEXISTENCE
/*! \brief Stay in Idle state, as Host can anytime start voice/music. 
  Headset shouldn't be in limbo state and requires user to power-on again */
#define IsUsbNotConnected() (!HeadsetUsb_IsAudioConnected())
#else
#define IsUsbNotConnected() TRUE
#endif

/*! \brief Return TRUE if there is no handset connection */
#define IsBtConnectionIdle() (headsetSmStateIsIdle(headsetGetState()) && (!appDeviceIsHandsetConnected()) && \
                                                                          headsetConfigIdleTimeoutMs())
/*! \brief Return TRUE if the idle timer needs to start */
#define headsetSmIsIdleTimerNeedToStart()  (IsUsbNotConnected() && IsBtConnectionIdle() && (!IsWiredAudioActive()) && \
                           (!AncStateManager_IsEnabled()) && (!AecLeakthrough_IsLeakthroughEnabled()))

/*! \brief Return TRUE if A2DP state is 'connected media streaming' and avrcp status is paused  */
#define IsA2dpStreamingAndAvrcpPaused() ( Av_IsA2dpSinkStreaming() && Av_IsPaused())

/*! \brief Return TRUE if the state is pairing*/
#define headsetSmStateIsPairing(state) (state == HEADSET_STATE_PAIRING)

/*! \brief Check if the headset is active.
*/
#define headsetSmIsActiveState(state) ((state == HEADSET_STATE_IDLE) || \
                                                      (state == HEADSET_STATE_BUSY))

/*! \brief Return TRUE if Auto power on panic is enabled and last reset happened due to panic */
#define headsetSmIsLastResetDueToPanic()  ( appConfigEnableAutoPowerOnAfterPanic() && isHeadsetResetSourcePanic() )


/*! \brief Check if the Bt audio is active.
*/                                                      
#define IsBtAudioActive() (Av_IsA2dpSinkStreaming() || HfpProfile_IsScoActive())

#ifdef ALLOW_USB_BT_COEXISTENCE
/*! \brief Check if the wired audio is active.
*/
#define IsWiredAudioActive() WiredAudioSource_IsAudioAvailable(audio_source_line_in)
#define IsUsbAudioRouted()  (AudioSources_IsAudioRouted(audio_source_usb) || VoiceSources_IsVoiceChannelAvailable(voice_source_usb))

#else
/*! \brief Check if the wired audio is active.
*/
#define IsWiredAudioActive() (WiredAudioSource_IsAudioAvailable(audio_source_line_in) || HeadsetUsb_IsAudioConnected())
#define IsUsbAudioRouted()  FALSE
#endif

/*! \brief Check if the headset audio is active.
*/
#define IsHeadsetAudioActive() (IsUsbAudioRouted() || IsBtAudioActive() || (IsWiredAudioActive()))

#ifdef INCLUDE_DFU
/*! \brief Check if the headset upgrade is active
*/
#define headsetSmIsDfuActive(reboot_reason) (reboot_reason == REBOOT_REASON_DFU_RESET)
/*! \brief Check if the upgrade was reverted post reboot
*/
#define headsetSmIsDfuRevertReset(reboot_reason) (reboot_reason == REBOOT_REASON_REVERT_RESET)
#endif /* INCLUDE_DFU */

#ifdef ENABLE_APP_LINE_IN_AUDIO
static bool g_Record_Line_In_State = TRUE;
#endif

#ifdef ENABLE_APP_MD_GAIA
typedef struct
{
    uint8 eq_mode;
    uint8 sidetone_status;
    uint8 mic_mute_control_status;
    uint8 idle_auto_poweroff_time;
}AppPskey_T;

AppPskey_T g_AppPoweronGetPskeyData = {0x00};
AppPskey_T g_AppGaiaChangePskeyData = {0x00};

static void appPowerOnGetPsSetting(void);
#endif

#ifdef ENABLE_APP_HID_COMMAND
bool g_IsAppDisableUsbAudio = FALSE;
#endif

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
static bool g_EnablePowerOff = FALSE;
#endif

#ifdef ENABLE_APP_MIC_MUTE
static bool g_MuteFlag = FALSE;
#endif

#ifdef ENABLE_APP_POWEROFF_DISPLAY_DISCONNECTED_PROMPT
static bool g_EnablePlayDisconnectPrompt = TRUE;
#endif

static void headsetSmHandleAncUpdateStateEnableInd(void);
static void headsetSmHandleAncUpdateStateDisableInd(void);
static void headsetSetState(headsetState new_state);
static headsetState headsetGetState(void);
static void headsetSMStopIdleTimer(void);
static void headsetSMStartIdleTimer(void);
static void headsetSmPowerOff(void);

static headsetState headsetSmDetermineCoreState(void);

#ifdef ALLOW_USB_BT_COEXISTENCE
static void headsetSm_AudioRouted(audio_source_t source, audio_routing_change_t change);

/* audio source observer interface */
static const audio_source_observer_interface_t headset_sm_observer_interface =
{
    .OnVolumeChange = NULL,
    .OnAudioRoutingChange = headsetSm_AudioRouted
};

/*! \brief handler for handling notifications from audio sources */
static void headsetSm_AudioRouted(audio_source_t source, audio_routing_change_t change)
{
    DEBUG_LOG_DEBUG("headsetSm_AudioRouted, enum:audio_source_t=%d enum:audio_routing_change_t=%d", source, change);
    if(source == audio_source_usb)
    {
        switch(change)
        {
            case source_routed:
            case source_unrouted:
                {
                    if(headsetSmIsActiveState(headsetGetState()))
                    {
                        headsetSetState(headsetSmDetermineCoreState());
                    }
                }
                break;
            default:
                DEBUG_LOG_DEBUG("headsetSm_AudioRouted, Invalid change!");
                Panic();
        }
    }
}
#endif
static void headsetSmHandleAncUpdateStateEnableInd(void)
{
    if(AncStateManager_IsEnabled())
    {
        headsetSMStopIdleTimer();
    }
}

static void headsetSmHandleAncUpdateStateDisableInd(void)
{
    if(headsetSmIsIdleTimerNeedToStart())
    {
        headsetSMStartIdleTimer();
    }
}

/*! \brief Function to start or stop headset idle timer for leakthrough*/
static void headsetSmHandleLeakthroughStateInd(LEAKTHROUGH_UPDATE_STATE_IND_T *msg)
{
    if(msg->state)
    {
        headsetSMStopIdleTimer();
    }
    else
    {
        if (headsetSmIsIdleTimerNeedToStart())
        {
            headsetSMStartIdleTimer();
        }
    }
}

#ifdef ENABLE_APP_MD_GAIA
/*! \brief function to start headset idle timer */
void headsetSMStartIdleTimer(void)
{
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
    if (appConfigIdleTimeoutMs_HandsetConected() != AUTO_OFF_TIMEOUT_NEVER)
#else
    if((appConfigIdleTimeoutMs_HandsetConected() != AUTO_OFF_TIMEOUT_NEVER) && (headsetSmIsIdleTimerNeedToStart()))
#endif
    {
        MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE);
        //MessageSendLater(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE, NULL, D_SEC(40));
        MessageSendLater(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE, NULL, D_MIN(appConfigIdleTimeoutMs_HandsetConected()));
    }
    else
    {
        MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE);
    }
}
#else
/*! \brief function to start headset idle timer */
static void headsetSMStartIdleTimer(void)
{
    if(headsetConfigIdleTimeoutMs() && headsetSmIsIdleTimerNeedToStart())
    {
       MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE);
       MessageSendLater(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE, NULL, headsetConfigIdleTimeoutMs()); 
    }
}
#endif

/*! \brief function to stop headset idle timer */
static void headsetSMStopIdleTimer(void)
{
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_IDLE);
}

/*! \brief function to check if the last reset was caused by Panic */
static bool isHeadsetResetSourcePanic(void)
{
    vm_reset_source source = VmGetResetSource();
    if(source == RESET_SOURCE_PANIC || source == RESET_SOURCE_APP_PANIC)
    {
        return TRUE;
    }
    return FALSE;
}

/*! \brief Prepare for streaming wired audio */
static bool headsetPrepareForWiredAudio(void)
{
    /* Disable wired audio controller */
    HeadsetWiredAudioController_Disable();
	
    /* Disable Headset topology */
    StereoTopology_Stop(headsetSmGetTask());
    return TRUE;
}

/*! \brief Prepare for streaming Bluetooth audio */
static bool headsetPrepareForBtAudio(void)
{
    /* Enable Headset topology */
    StereoTopology_Start(headsetSmGetTask());
    return TRUE;
}

/*! \brief Initiate disconnect of all links */
static void headsetSmInitiateLinkDisconnection(uint16 timeout_ms)
{
    bool disconnecting_link = headsetSmDisconnectLink();

    DEBUG_LOG_VERBOSE("headsetSmInitiateLinkDisconnection");

    if (!disconnecting_link)
    {
        headsetSmClearDisconnectLock();
        DEBUG_LOG_VERBOSE("headsetSmInitiateLinkDisconnection: Lock cleared");
    }
    else
    {
        headsetSmSetDisconnectLock();
        DEBUG_LOG_VERBOSE("headsetSmInitiateLinkDisconnection: Lock set");
    }

    MessageSendConditionally(headsetSmGetTask(), SM_INTERNAL_LINK_DISCONNECTION_COMPLETE,
                             NULL, &headsetSmGetDisconnectLock());

    /* Start a timer to force reset if we fail to complete disconnection */
    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION, NULL, timeout_ms);
}

static void headsetSmClearPsStore(void)
{
    DEBUG_LOG_FN_ENTRY("headsetSmClearPsStore");

    for (int i=0; i<TDL_SIZE; i++)
    {
        PsStore(ATTRIBUTE_BASE_PSKEY_INDEX+i, NULL, 0);
        PsStore(TDL_BASE_PSKEY_INDEX+i, NULL, 0);
    }

    PsStore(TDL_INDEX_PSKEY, NULL, 0);
    PsStore(PS_KEY_HFP_CONFIG, NULL, 0);

    /* Clear out any in progress DFU status */
#ifdef INCLUDE_DFU
    Dfu_ClearPsStore();
#endif
}

#ifdef ENABLE_APP_FACTORY_RESET
/*! \brief Delete handset pairing and reboot device. */
static void headsetSmDeletePairingAndReset(void)
{
    DEBUG_LOG_ALWAYS("headsetSmDeletePairingAndReset");

    /* cancel the link disconnection, may already be gone if it fired to get us here */
    MessageCancelFirst(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION);

    headsetSmClearPsStore();

#ifdef INCLUDE_FAST_PAIR
    /* Delete the account keys */
    FastPair_DeleteAccountKeys();
#endif

    SystemState_EmergencyShutdown();
}
#else
/*! \brief Delete handset pairing and reboot device. */
static void headsetSmDeletePairingAndReset(void)
{
    DEBUG_LOG_ALWAYS("headsetSmDeletePairingAndReset");

    /* cancel the link disconnection, may already be gone if it fired to get us here */
    MessageCancelFirst(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION);

    headsetSmClearPsStore();

#ifdef INCLUDE_FAST_PAIR
    /* Delete the account keys */
    FastPair_DeleteAccountKeys();
#endif

    SystemReboot_Reboot();
}
#endif

/*! \brief Handle indication all requested links are now disconnected. */
static void headsetSmHandleInternalLinkDisconnectionComplete(void)
{
    DEBUG_LOG_DEBUG("headsetSmHandleInternalLinkDisconnectionComplete 0x%x", headsetGetState());
    MessageCancelFirst(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION);

    /* Delete all devices for headset device type. */
    BtDevice_DeleteAllDevicesOfType(DEVICE_TYPE_HANDSET);

#ifdef INCLUDE_FAST_PAIR
    /* Delete the account keys */
    FastPair_DeleteAccountKeys();
#endif

}

/*! \brief Handle failure to successfully disconnect links within timeout.
*/
static void headsetSmLinkDisconnectionTimeout(void)
{
    DEBUG_LOG_DEBUG("headsetSmLinkDisconnectionTimeout 0x%x, Lock = %d", headsetGetState(), headsetSmGetDisconnectLock());

    if(headsetSmGetDisconnectLock())
    {
        MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_LINK_DISCONNECTION_COMPLETE);
        headsetSmClearDisconnectLock();
    }
}

/*! \brief Start/Restart headset limbo timer, if its not zero */
static void headsetSmStartLimboTimer(void)
{
    if(headsetConfigLimboTimeoutMs())
    {
        DEBUG_LOG_DEBUG("headsetSmStartLimboTimer : Limbo Timer started LimboTimeoutMs %u",headsetConfigLimboTimeoutMs());
        MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LIMBO);
        MessageSendLater(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LIMBO, NULL, headsetConfigLimboTimeoutMs());
    }
}

/*! \brief Stop headset limbo timer. */
static void headsetSmStopLimboTimer(void)
{
    DEBUG_LOG_DEBUG("headsetSmStopLimboTimer : Limbo Timer stopped");
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_TIMEOUT_LIMBO);
}

#if 0
/*! \brief Take action following chargers indication of charger disconnect */
static void headsetSmHandleChargerMessageDetached(void)
{
    DEBUG_LOG_VERBOSE("headsetSmHandleChargerMessageDetached , state %d", headsetGetState());
    if(HEADSET_STATE_LIMBO ==  headsetGetState())
    {
        headsetSmStartLimboTimer();
    }
}
#endif

/*! \brief Take action following power's indication of imminent shutdown.
    Can be received in any state. */
static void headsetSmHandlePowerShutdownPrepareInd(void)
{
    DEBUG_LOG_VERBOSE("headsetSmHandlePowerShutdownPrepareInd, state %d", headsetGetState()); 
    switch (headsetGetState())
    {
        case HEADSET_STATE_LIMBO:
            DEBUG_LOG_VERBOSE("headsetSmHandlePowerShutdownPrepareInd, SilentCommitEnabled %d", Dfu_IsSilentCommitEnabled());
            /* Do not respond to shut down request if silent commit is pending
             * because DFU reboot will be in progress and that will get
             * interrupted.
             */
            if(!Dfu_IsSilentCommitEnabled())
            {
                appPowerShutdownPrepareResponse(headsetSmGetTask());
            }
            break;
        default : 
            headsetSetState(HEADSET_STATE_TERMINATING);
            break;
    }
}

/*! \brief Request a factory reset. */
static void headsetSmFactoryReset(void)
{   
#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
    MessageSend(headsetSmGetTask(), SM_INTERNAL_APP_FACTORY_RESET_HANDLE, NULL);
#else
    MessageSend(headsetSmGetTask(), SM_INTERNAL_FACTORY_RESET, NULL);
#endif
}

/*! \brief Request handset pair. */
static void headsetSmPairHandset(void)
{
    MessageSend(headsetSmGetTask(), SM_INTERNAL_PAIR_HANDSET, NULL);
}

/*! \brief Request handset connection. */
static void headsetSmConnectHandset(void)
{
    if (!IsWiredAudioActive())
    {
        DEBUG_LOG_DEBUG("appSmConnectHandset");
        StereoTopology_ConnectMruHandset();
    }
}

static void headsetSmDisconnectLruHandset(void)
{
    DEBUG_LOG_DEBUG("headsetSmDisconnectLruHandset");
    StereoTopology_DisconnectLruHandset();
}

/*! \brief Enable BR/EDR multipoint */
static void headsetSmEnableMultipoint(void)
{
    DEBUG_LOG_DEBUG("headsetSmEnableMultipoint");

    if(!HandsetService_IsBrEdrMultipointEnabled())
    {
        /* Configure Handset Service */
        HandsetService_Configure(handset_service_multipoint_config);
    }

    /* Make headset to be connectable(i.e. start PAGE scanning) only if one
       handset is already connected. Headset is not discoverable so headset
       cannot be seen by handset who wants to connect to.
       However, if Handset was connected to headset in the past and entry of headset
       exists in handset's PDL then handset can connect to headset. */
    if(HandsetService_IsBrEdrMultipointEnabled() && (HandsetService_GetNumberOfConnectedBredrHandsets() == 1))
    {
        HandsetService_ConnectableRequest(NULL);
    }
}

/*! \brief Disable BR/EDR multipoint */
static void headsetSmDisableMultipoint(void)
{
    DEBUG_LOG_DEBUG("headsetSmDisableMultipoint");

    if (HandsetService_IsBrEdrMultipointEnabled())
    {
        HandsetService_Configure(handset_service_singlepoint_config);

        if (HandsetService_GetNumberOfConnectedBredrHandsets() > HandsetService_GetMaxNumberOfConnectedBredrHandsets())
        {
            /* If multipoint was disabled but 2 handsets are connected then
               one of them needs to be disconnected. */
            StereoTopology_DisconnectLruHandset();
        }
    }
}

/*! \brief Request handset delete. */
static void headsetSmDeleteHandsets(void)
{
    MessageSend(headsetSmGetTask(), SM_INTERNAL_DELETE_HANDSETS, NULL);
}

/*! \brief Request headset power off. */
static void headsetSmPowerOff(void)
{
    MessageSend(headsetSmGetTask(), SM_INTERNAL_POWER_OFF, NULL);

#ifdef ENABLE_APP_POWERON_ENTER_PAIRING
    if(appGetAllowPoweronReconnectFlag() == TRUE)
    {
        appSetAllowPoweronReconnectFlag(FALSE);
    }
#endif
}

void appHeadsetSmPowerOff(void)
{
    MessageSend(headsetSmGetTask(), SM_INTERNAL_POWER_OFF, NULL);
}

/*! \brief Handle power on confirmation
 */
static void headsetSmHandlePoweredOn(void)
{
    if(
#ifdef ALLOW_USB_BT_COEXISTENCE
            !IsWiredAudioActive() && !HeadsetUsb_IsAudioConnected()
#else
            !IsWiredAudioActive()
#endif
      )
    {
        if (BtDevice_IsPairedWithHandset())
        {
            DEBUG_LOG_DEBUG("headsetSmHandlePoweredOn, already paired with handset");

            /* Move to idle state and initiate connection */
            headsetSetState(HEADSET_STATE_IDLE);
            return;
        }
        else
        {
            DEBUG_LOG_DEBUG("headsetSmHandlePoweredOn, no device in PDL, move to pairing");

            /* Stop Wired Audio PIO monitoring */
            WiredAudioSource_StopMonitoring(headsetSmGetTask());

            /* Disable USB Audio Feature */
            HeadsetUsb_AudioDisable(headsetSmGetTask());

            headsetSmClearUserPairing();
            headsetSetState(HEADSET_STATE_PAIRING);
        }
    }
    else
    {
        /* Changing state to HEADSET_STATE_IDLE if wired audio source connected message is not received.
        This will handle the case of 'Wired audio source is removed before sending connected message' */
        if(headsetGetState() == HEADSET_STATE_POWERING_ON)
        {
            headsetSetState(HEADSET_STATE_IDLE);
        }
    }
}

#ifndef ENABLE_APP_SCO_AUTO_POWEROFF_TIMER
/*! \brief Idle timeout */
static void headsetSmHandleTimeoutIdle(void)
{
    DEBUG_LOG_DEBUG("headsetSmHandleTimeoutIdle, state %d, handset connected %d", headsetGetState(), appDeviceIsHandsetConnected());
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
    if(HEADSET_STATE_IDLE == headsetGetState())
#else
    if((HEADSET_STATE_IDLE == headsetGetState()) && (!appDeviceIsHandsetConnected()))
#endif
    {
        headsetSmPowerOff();
    }
}
#else
/*! \brief Idle timeout */
static void headsetSmHandleTimeoutIdle(void)
{
    DEBUG_LOG_DEBUG("headsetSmHandleTimeoutIdle, state %d, ScoActive %d", headsetGetState(), HfpProfile_IsScoActive());
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
    if((HEADSET_STATE_IDLE == headsetGetState()) && (HfpProfile_IsScoActive() == FALSE))
#else
    if((HEADSET_STATE_IDLE == headsetGetState()) && (!appDeviceIsHandsetConnected()))
#endif
    {
        headsetSmPowerOff();
    }
}
#endif

/*! \brief Limbo timeout */
static void headsetSmHandleTimeoutLimbo(void)
{
    DEBUG_LOG_DEBUG("headsetSmHandleTimeoutLimbo, state %d", headsetGetState());
    if((HEADSET_STATE_LIMBO == headsetGetState()))
    {
        appPowerOffRequest();
    }
}

#ifdef INCLUDE_DFU
/*! \brief Check if DFU was active.
 */
static void headsetCheckDfu(void)
{
    dfu_reboot_reason_t reboot_reason = Dfu_GetRebootReason();
    DEBUG_LOG_DEBUG("headsetCheckDfu: Reboot Reason enum:dfu_reboot_reason_t:%d", reboot_reason);
    /* Limbo to power-on transition should happen only for interactive commit
     * or when the upgrade is aborted on the final commit screen.
     * Silent commit happens when user puts HS in limbo state so, the same
     * state should be retained even after reboot. */
    if((headsetSmIsDfuActive(reboot_reason) || headsetSmIsDfuRevertReset(reboot_reason)) && !Dfu_IsSilentCommitEnabled())
    {
        if(headsetSmIsDfuActive(reboot_reason))
        {
            /*
             * Safe restart/stretch of DFU reconnection timer as limbo to power-on
             * transition is delayed.
             */
            UpgradeRestartReconnectionTimer();
        }
        Ui_InjectBackEndUiInput(ui_input_sm_power_on,
                                headsetConfigDfuCommitDelayForLimboToPowerOn());
        if(headsetSmIsDfuRevertReset(reboot_reason))
        {
            Dfu_SetRebootReason(REBOOT_REASON_NONE);
        }
    }
}
#endif

/*****************************************************************************
 * SM state transition handler functions
 *****************************************************************************/

/*! \brief Enter initialising state

    This function is called whenever the state changes to
    HEADSET_STATE_LIMBO.
    It is reponsible for initialising global aspects of the application,
    i.e. non application module related state.
*/
static void headsetEnterLimbo(void)
{
    DEBUG_LOG_DEBUG("headsetEnterLimbo : HEADSET_STATE_LIMBO");
    headsetSmStartLimboTimer();
#ifdef INCLUDE_DFU
    headsetCheckDfu();
    
    /* check and inform the DFU domain if the headset is in limbo state (not-in-use state) 
     * and silent commit is enabled */
    if(Dfu_IsSilentCommitEnabled())
    {
        Dfu_HandleDeviceNotInUse();
    }
#endif
    if(headsetSmGetAutoPowerOn())
    {
        DEBUG_LOG_DEBUG("headsetEnterLimbo : Auto PowerOn");
        headsetSmClearAutoPowerOn();
        Ui_InjectUiInput(ui_input_sm_power_on);
    }
}

/*! \brief Exit limbo state.
 */
static void headsetExitLimbo(void)
{
    DEBUG_LOG_DEBUG("headsetExitLimbo");
    headsetSmStopLimboTimer();
}

/*! \brief Enter powering on state.
 */
static void headsetEnterPoweringOn(void)
{
    DEBUG_LOG_ALWAYS("headsetEnterPoweringOn : HEADSET_STATE_POWERING_ON");
}

/*! \brief Exit powering on state.
 */
static void headsetExitPoweringOn(void)
{
    DEBUG_LOG_DEBUG("headsetExitPoweringOn");
}

/*! \brief Enter powering off state.
 */
static void headsetEnterPoweringOff(void)
{
    DEBUG_LOG_ALWAYS("headsetEnterPoweringOff : HEADSET_STATE_POWERING_OFF");
}

/*! \brief Exit powering off state.
 */
static void headsetExitPoweringOff(void)
{
    DEBUG_LOG_DEBUG("headsetExitPoweringOff");
}

/*! \brief Enter actions when we enter the factory reset state.
 */
static void headsetEnterFactoryReset(void)
{
    DEBUG_LOG_DEBUG("headsetEnterFactoryReset : HEADSET_STATE_FACTORY_RESET");
    headsetSmDeletePairingAndReset();
}

/*! \brief Exit factory reset. */
static void headsetExitFactoryReset(void)
{
    /* Should never happen */
    Panic();
}

/*! \brief Enter Idle state.
 */
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
static void headsetEnterIdle(void)
{
    DEBUG_LOG_DEBUG("headsetEnterIdle : HEADSET_STATE_IDLE");

    if (appGetPairingFlag() == FALSE)
    {
        headsetSMStartIdleTimer();
    }
    else
    {
        appSetPairingFlag(FALSE);
    }

    Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle);
}
#else
static void headsetEnterIdle(void)
{
    DEBUG_LOG_DEBUG("headsetEnterIdle : HEADSET_STATE_IDLE");

    if(headsetSmIsIdleTimerNeedToStart())
    {
        headsetSMStartIdleTimer();
    }

    Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle);
}
#endif

/*! \brief Exit Idle on state.
 */
static void headsetExitIdle(void)
{
    DEBUG_LOG_DEBUG("headsetExitIdle");

    headsetSMStopIdleTimer();

    Ui_InformContextChange(ui_provider_app_sm, context_app_sm_exit_idle);
}

/*! \brief Enter Busy state.
 */
static void headsetEnterBusy(void)
{
   DEBUG_LOG_DEBUG("headsetEnterBusy : HEADSET_STATE_BUSY");

   if(IsWiredAudioActive())
   {   
      headsetPrepareForWiredAudio();
   }
}

/*! \brief Exit Busy state.
 */
static void headsetExitBusy(void)
{
    DEBUG_LOG_DEBUG("headsetExitBusy");
    if(!IsWiredAudioActive())
    {
       headsetPrepareForBtAudio();
    }
}

/*! \brief Enter Pairing state.
 */
static void headsetEnterPairing(void)
{
    DEBUG_LOG_DEBUG("headsetEnterPairing : HEADSET_STATE_PAIRING");

    if(headsetSmIsUserPairing())
    {
        StereoTopology_ProhibitHandsetConnection(TRUE);
    }
    else
    {
        Pairing_Pair(headsetSmGetTask(), headsetSmIsUserPairing());
    }
    HandsetService_ConnectableRequest(headsetSmGetTask());
}

/*! \brief Exit Pairing on state.
 */
static void headsetExitPairing(void)
{
    DEBUG_LOG_DEBUG("headsetExitPairing");
    Pairing_PairStop(NULL);
    headsetSmClearUserPairing();
    StereoTopology_ProhibitHandsetConnection(FALSE);
}

/*! \brief Enter Terminating state.
 */
static void headsetEnterTerminating(void)
{
    DEBUG_LOG_DEBUG("headsetEnterTerminating : HEADSET_STATE_TERMINATING");
    bdaddr addr;
    if (appDeviceGetHandsetBdAddr(&addr) && HandsetService_IsBredrConnected(&addr))
    {
        ConManagerSendCloseAclRequest(&addr, TRUE);
    }
    WiredAudioSource_StopMonitoring(headsetSmGetTask());
    appPowerShutdownPrepareResponse(headsetSmGetTask());

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_FORCE_POWER_OFF);
    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_FORCE_POWER_OFF, NULL, 1500);
#endif
}

/*! \brief Exit Terminating state.
 */
static void headsetExitTerminating(void)
{
    DEBUG_LOG_DEBUG("headsetExitTerminating");
}

/*! \brief Provides Headset Application state machine context changes to the User Interface module.

    \param[in]  void

    \return     current_sm_ctxt - current application context of sm module.
*/
static unsigned headsetSm_GetApplicationCurrentContext(void)
{
    sm_provider_context_t context = context_app_sm_powered_on;

     switch(headsetGetState())
     {
        case HEADSET_STATE_NULL : /* fall through */
        case HEADSET_STATE_LIMBO: /* fall through */
        case HEADSET_STATE_POWERING_ON: /* fall through */
        case HEADSET_STATE_POWERING_OFF: /* fall through */
        case HEADSET_STATE_TERMINATING: /* fall through */
        case HEADSET_STATE_FACTORY_RESET: context = context_app_sm_powered_off;
            break;

        case HEADSET_STATE_BUSY: /* fall through */
        case HEADSET_STATE_IDLE:
            /* It could so happen if USB co-exists with BT that, USB could be streaming while BT is just connected */
            if(IsUsbAudioRouted())
                context = context_app_sm_active;
            else
                context = appDeviceIsBredrHandsetConnected() == TRUE ? context_app_sm_idle_connected : context_app_sm_idle;
            break;

        default : break;
     }

    return (unsigned)context;
}


/* This function is called to change the applications state, it automatically
   calls the entry and exit functions for the new and old states.
*/
static void headsetSetState(headsetState new_state)
{
    smTaskData* sm = SmGetTaskData();
    headsetState previous_state = SmGetTaskData()->state;

    DEBUG_LOG_STATE("headsetSetState, state 0x%02x to 0x%02x", previous_state, new_state);

    /* Handle state exit functions */
    switch (previous_state)
    {
        case HEADSET_STATE_NULL:
            /* This can occur when DFU is entered during INIT. */
            break;

        case HEADSET_STATE_LIMBO:
            headsetExitLimbo();
            break;

        case HEADSET_STATE_POWERING_ON:
            headsetExitPoweringOn();
            break;

        case HEADSET_STATE_POWERING_OFF:
            headsetExitPoweringOff();
            break;

        case HEADSET_STATE_FACTORY_RESET:
            headsetExitFactoryReset();
            break;

        case HEADSET_STATE_PAIRING:
            headsetExitPairing();
            break;

        case HEADSET_STATE_IDLE:
            headsetExitIdle();
            break;

        case HEADSET_STATE_BUSY:
            headsetExitBusy();
            break;

        case HEADSET_STATE_TERMINATING:
            headsetExitTerminating();
            break;          

        default:
            DEBUG_LOG_ERROR("Attempted to exit unsupported state 0x%02x", SmGetTaskData()->state);
            Panic();
            break;
    }
    /* Set new state */
    SmGetTaskData()->state = new_state;
    /* Handle state entry functions */
    switch (new_state)
    {
        case HEADSET_STATE_LIMBO:
            headsetEnterLimbo();
            break;

        case HEADSET_STATE_FACTORY_RESET:
            headsetEnterFactoryReset();
            break;

        case HEADSET_STATE_POWERING_ON:
            headsetEnterPoweringOn();
            break;

        case HEADSET_STATE_PAIRING:
            headsetEnterPairing();
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
            headsetSMStartIdleTimer();
#endif
#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
            MessageSendLater(headsetSmGetTask(), SM_INTERNAL_PAIRING_PROMPTS_CYCLE, NULL, 2000);
#endif
            break;

        case HEADSET_STATE_IDLE:
            headsetEnterIdle();
#ifdef ENABLE_APP_HID_COMMAND
            if (g_IsAppDisableUsbAudio == TRUE)
            {
                g_IsAppDisableUsbAudio = FALSE;
                WiredAudioSource_StopMonitoring(headsetSmGetTask());
                HeadsetUsb_AudioDisable(headsetSmGetTask());
            }
#endif
            break;

        case HEADSET_STATE_POWERING_OFF:
            headsetEnterPoweringOff();
            break;

        case HEADSET_STATE_BUSY:
            headsetEnterBusy();
            break;
            
        case HEADSET_STATE_TERMINATING:
            headsetEnterTerminating();
            break;      

        default:
            DEBUG_LOG_ERROR("Attempted to enter unsupported state 0x%02x", new_state);
            Panic();
            break;
    }

    Ui_InformContextChange(ui_provider_app_sm, headsetSm_GetApplicationCurrentContext());
    DEBUG_LOG_VERBOSE("headsetSetState, new state 0x%02x", sm->state);
}

static headsetState headsetGetState(void)
{
    return SmGetTaskData()->state;
}

void appHeadsetSetState(headsetState new_state)
{
    headsetSetState(new_state);
}

headsetState appHeadsetGetState(void)
{
    return SmGetTaskData()->state;
}

/*! \brief Handle request to start factory reset. */
static void headsetSmHandleInternalFactoryReset(void)
{
    if (headsetGetState() > HEADSET_STATE_POWERING_ON)
    {
        DEBUG_LOG_DEBUG("headsetSmHandleInternalFactoryReset");
        headsetSetState(HEADSET_STATE_FACTORY_RESET);
    }
    else
        DEBUG_LOG_WARN("headsetSmHandleInternalFactoryReset cannot be done in state %d", headsetGetState());
}


/*! \brief Handle request to start handset pair. */
static void headsetSmHandleInternalPairHandset(void)
{
    /* The A2DP media channel is not closed on avrcp pause for a few seconds. 
    Headset stays in busy state till the media channel is closed. Headset state 
    machine allows pair handset request in the above case as well. */
#ifndef ENABLE_APP_MUSIC_STREAMING_CAN_PAIRING
    if (headsetSmStateIsIdle(headsetGetState()) || IsA2dpStreamingAndAvrcpPaused())
#else
    if (headsetSmStateIsIdle(headsetGetState()) || Av_IsA2dpSinkStreaming())
#endif
    {
       /* Stop Wired Audio PIO monitoring */
       WiredAudioSource_StopMonitoring(headsetSmGetTask());

       /* Disable USB Audio Feature */
       HeadsetUsb_AudioDisable(headsetSmGetTask());

       DEBUG_LOG_DEBUG("headsetSmHandleInternalPairHandset USER PAIRING REQUEST");
       headsetSmSetUserPairing();
       headsetSetState(HEADSET_STATE_PAIRING);
    }
    else
        DEBUG_LOG_WARN("headsetSmHandleInternalPairHandset can only pair in IDLE state");
}

/*! \brief Delete pairing for all handsets.
    \note There must be no connections to a handset for this to succeed. */
static void headsetSmHandleInternalDeleteHandsets(void)
{
    DEBUG_LOG_DEBUG("headsetSmHandleInternalDeleteHandsets");

    switch (headsetGetState())
    {
        case HEADSET_STATE_IDLE:
        case HEADSET_STATE_BUSY:
            /* Stop being connectable when deleting handset pairing */
            HandsetService_CancelConnectableRequest(headsetSmGetTask());
            headsetSmInitiateLinkDisconnection(headsetConfigDisconnectTimeoutMs());
            break;

        default:
            DEBUG_LOG_WARN("headsetSmHandleInternalDeleteHandsets bad state %u",
                                                        headsetGetState());
            break;
    }
}

/*! \brief Handle request to power off headset. */
static void headsetSmHandleInternalPowerOff(void)
{
    if (headsetGetState() > HEADSET_STATE_LIMBO)
    {
        SystemState_PowerOff();
    }
}

#ifdef ENABLE_APP_BATTERY_CHECK_LED
static void headsetSmHandleBatteryPrecentCheck(void)
{
    uint8 batt_percent = Soc_ConvertLevelToPercentage(appBatteryGetVoltageAverage());

    DEBUG_LOG_DEBUG("headsetSmHandleBatteryPrecentCheck: %d", batt_percent);

    if (batt_percent <= 30)                                 //Red LED
    {
        UiLeds_NotifyUiIndication(get_LED_Battery_Low_LEDIndex());
    }
    else if ((batt_percent > 30) && (batt_percent < 70))   //Amber LED
    {
        UiLeds_NotifyUiIndication(get_LED_Battery_Medium_LEDIndex());
    }
    else                                                    //Green LED
    {
        UiLeds_NotifyUiIndication(get_LED_Battery_High_LEDIndex());
    }
}
#endif

/*! \brief Handle request to power on headset. */
static void headsetSmHandlePowerOn(void)
{
    if (headsetGetState() == HEADSET_STATE_LIMBO)
    {
        SystemState_PowerOn();

#ifdef ENABLE_APP_BATTERY_CHECK_LED
        headsetSmHandleBatteryPrecentCheck();
#endif

#ifdef ENABLE_APP_POWERON_ENTER_PAIRING
        appSetAllowPoweronReconnectFlag(TRUE);
        appPoweronReconnect();
#endif

#ifdef ENABLE_APP_BATTERY_LOW_WARNING
        appBatteryLevelCheck();
#endif

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
        MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_POWER_ON_3S_TIMER);
        MessageSendLater(headsetSmGetTask(), SM_INTERNAL_POWER_ON_3S_TIMER, NULL, D_SEC(3));
#endif
    }
}

/*! \brief handles sm module specific ui inputs

    Invokes routines based on ui input received from ui module.

    \param[in] id - ui input

    \returns void
 */
static void headsetSmHandleUiInput(MessageId ui_input)
{
    switch (ui_input)
    {
        case ui_input_sm_power_on:
#ifdef ENABLE_APP_USB_AUDIO
            /* --- When the USB is inserted, disable power on or power off --- start -- */
            if(Charger_IsConnected() == TRUE)
            {
                break;
            }
            /* --- When the USB is inserted, disable power on or power off ---  end  -- */
#endif
#ifdef ENABLE_APP_POWEROFF_DISPLAY_DISCONNECTED_PROMPT
            appSetEnablePlayDisconnectPromptFlag(TRUE);
#endif
            DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_sm_power_on");
            headsetSmHandlePowerOn();
            break;

        case ui_input_sm_power_off:
#ifdef ENABLE_APP_USB_AUDIO
            /* --- When the USB is inserted, disable power on or power off --- start -- */
            if(Charger_IsConnected() == TRUE)
            {
                break;
            }
            /* --- When the USB is inserted, disable power on or power off ---  end  -- */
#endif

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
            if (g_EnablePowerOff == FALSE)
            {
                break;
            }
#endif

#ifdef ENABLE_APP_POWEROFF_DISPLAY_DISCONNECTED_PROMPT
            appSetEnablePlayDisconnectPromptFlag(FALSE);
#endif

#ifdef ENABLE_APP_MD_GAIA
            appPoweroffStorePskeyData();
#endif
            DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_sm_power_off");
            headsetSmPowerOff();
            break;

        case ui_input_connect_handset:
             DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_connect_handset");
             headsetSmConnectHandset();
            break;

        case ui_input_disconnect_lru_handset:
            DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_disconnect_lru_handset");
            headsetSmDisconnectLruHandset();
            break;

        case ui_input_sm_pair_handset:
#ifdef ENABLE_APP_USB_AUDIO
            /* --- When the USB is inserted, disable pairing --- start -- */
            if(Charger_IsConnected() == TRUE)
            {
                break;
            }
            /* --- When the USB is inserted, disable pairing ---  end  -- */
#endif
            DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_sm_pair_handset");
            headsetSmPairHandset();
            break;

        case ui_input_sm_delete_handsets:
            DEBUG_LOG_VERBOSE("headsetSmHandleUiInput received ui_input_sm_delete_handset");
            headsetSmDeleteHandsets();
            break;

        case ui_input_enable_multipoint:
            headsetSmEnableMultipoint();
            break;

        case ui_input_disable_multipoint:
            headsetSmDisableMultipoint();
            break;

        case ui_input_factory_reset_request:
            headsetSmFactoryReset();
            break;

#ifdef ENABLE_APP_ENTER_DUT_MODE
        case ui_input_dut_mode:
            /* can enter DUT MODE when in ear */
            if(HEADSET_STATE_PAIRING == headsetGetState())
            {
                /* send INN_ENTER_DUT_LED to blink led*/
                appEnterDutMode();
                ConManagerAllowHandsetConnect(TRUE);
                ConnectionEnterDutMode();
            }
            break;
#endif

#ifdef ENABLE_APP_PAIRING_ENTER_OR_CANCEL
        case ui_input_app_pairing_enter_or_cancel:
            {
                DEBUG_LOG_DEBUG("----- ui_input_app_pairing_enter_or_cancel -------");

#ifndef ENABLE_APP_MUSIC_STREAMING_CAN_PAIRING
                if (HEADSET_STATE_IDLE ==  headsetGetState())
#else
                if ((HEADSET_STATE_IDLE ==  headsetGetState()) || (Av_IsA2dpSinkStreaming() == TRUE))
#endif
                {
                    DEBUG_LOG_DEBUG("-----ui_input_app_pairing_enter_or_cancel: Enter pairing-------");
                    appTestPairHandset();
                }
                else if (HEADSET_STATE_PAIRING ==  headsetGetState())
                {
                    DEBUG_LOG_DEBUG("-----ui_input_app_pairing_enter_or_cancel: Exit pairing-------");
                    headsetExitPairing();
                }
            }
            break;
#endif

#ifdef ENABLE_APP_MIC_MUTE
        case ui_input_app_mfb_button_double_click:
            {
                if (appTestIsHandsetHfpScoActive() == TRUE)
                {
                    if (appGetMicMuteControlStatus() == MIC_MUTE_CONTROL_ENABLE)
                    {
                        g_MuteFlag = !g_MuteFlag;
                        if (g_MuteFlag == TRUE)
                        {
                            UiPrompts_SendEvent(INN_APP_MIC_MUTE_PROMPT, 0);
                            appKymeraScoMicMute(TRUE);
                        }
                        else
                        {
                            UiPrompts_SendEvent(INN_APP_MIC_UNMUTE_PROMPT, 0);
                            appKymeraScoMicMute(FALSE);
                        }
                    }
                    else
                    {
                        if (g_MuteFlag == TRUE)
                        {
                            UiPrompts_SendEvent(INN_APP_MIC_UNMUTE_PROMPT, 0);
                            appKymeraScoMicMute(FALSE);
                            g_MuteFlag = FALSE;
                        }
                    }
                }
            }
            break;
#endif

#ifdef ENABLE_APP_PANIC_TO_OFFLINE_LOG
        case ui_input_app_power_button_triple_click:
        {
            Panic();
        }
        break;
#endif
        default:
            break;
    }
}

/*! \brief Handle completion of application module initialisation. */
static void headsetSmHandleSystemSartedUpToLimbo(void)
{
    DEBUG_LOG_INFO("headsetSmHandleSystemSartedUpToLimbo");

    switch (headsetGetState())
    {
        case HEADSET_STATE_NULL:
            {
#ifdef ENABLE_APP_SINGLE_PRESS_TO_POWERON
                if (Charger_IsConnected() == FALSE)
                {
                    headsetSmSetAutoPowerOn();
                }
#endif
                headsetSetState(HEADSET_STATE_LIMBO);
            }
        break;

        default:
            Panic();
    }
}

static void headsetSmHandleSystemStateChange(SYSTEM_STATE_STATE_CHANGE_T *msg)
{
    DEBUG_LOG_DEBUG("headsetSmHandleSystemStateChange old state 0x%x, new state 0x%x", msg->old_state, msg->new_state);

    if(msg->old_state == system_state_starting_up && msg->new_state == system_state_limbo)
    {
        headsetSmHandleSystemSartedUpToLimbo();
    }
    else if(msg->old_state == system_state_limbo && msg->new_state == system_state_powering_on)
    {
        headsetSetState(HEADSET_STATE_POWERING_ON);
    }
    else if(msg->old_state == system_state_powering_on && msg->new_state == system_state_active)
    {
        headsetSmHandlePoweredOn();
    }
    else if(msg->old_state == system_state_active && msg->new_state == system_state_powering_off)
    {
        headsetSetState(HEADSET_STATE_POWERING_OFF);
    }
}

Task headsetSmGetTask(void)
{
  return &headset_sm.task;
}

/*! \brief Handle completion of handset pairing. */
static void headsetSmHandlePairingPairConfirm(PAIRING_PAIR_CFM_T *cfm)
{
    DEBUG_LOG_DEBUG("headsetSmHandlePairingPairConfirm, status %d", cfm->status);

    switch (headsetGetState())
    {
        case HEADSET_STATE_PAIRING:
            DEBUG_LOG_VERBOSE("PAIRING COMPLETE");

            /* Start Wired Audio PIO monitoring */    
            WiredAudioSource_StartMonitoring(headsetSmGetTask());

            /* Enable USB Audio Feature */
            HeadsetUsb_AudioEnable(headsetSmGetTask());

            headsetSetState(HEADSET_STATE_IDLE);
            break;

        case HEADSET_STATE_FACTORY_RESET:
            /* Nothing to do, even if pairing with handset succeeded, the final
            act of factory reset is to delete handset pairing */
            break;

        default:
            /* Ignore, paired with handset with known address as requested by peer */
            break;
    }
}

/*! \brief Handle notification of Handset Service connection. */
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
static void headsetSmHandleHandsetServiceConnectedInd(HANDSET_SERVICE_CONNECTED_IND_T* ind)
{
    UNUSED(ind);
    /* We need not be in IDLE state, as USB could be streaming while BT connects */
    if(headsetSmStateIsIdle(headsetGetState()))
    {
        /* Inform UI that a handset has been connected */
        Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle_connected);

        headsetSMStartIdleTimer();
    }
}
#else
static void headsetSmHandleHandsetServiceConnectedInd(HANDSET_SERVICE_CONNECTED_IND_T* ind)
{
    UNUSED(ind);
    /* We need not be in IDLE state, as USB could be streaming while BT connects */
    if(headsetSmStateIsIdle(headsetGetState()))
    {
        /* Inform UI that a handset has been connected */
        Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle_connected);
        headsetSMStopIdleTimer();
    }
}
#endif

/*! \brief Handle handset disconnected indication from headset topology. */
static void headsetSmHandleStereoTopologyHandsetDisInd(STEREO_TOPOLOGY_HANDSET_DISCONNECTED_IND_T* ind )
{
    DEBUG_LOG_DEBUG("headsetSmHandleStereoTopologyHandsetDisInd STEREO_TOPOLOGY_HANDSET_DISCONNECTED_IND, status %d", ind->status);
    if(headsetSmIsUserPairing() && headsetSmStateIsPairing(headsetGetState()))
    {
        if(ind->status == handset_service_status_success)
        {
            Pairing_Pair(headsetSmGetTask(), TRUE);
        }
        else if(ind->status ==  handset_service_status_failed)
        {
            headsetSetState(HEADSET_STATE_IDLE);
        }
    }
}

static void headsetSmUpdateDisconnectingLink(void)
{
    /* Update the disconnecting handset link lock status */
    if(headsetSmGetDisconnectLock())
    {
        DEBUG_LOG_DEBUG("headsetSmUpdateDisconnectingLink disconnecting handset");
        if (!HandsetService_IsAnyBredrConnected())
        {
            DEBUG_LOG_DEBUG("headsetSmUpdateDisconnectingLink handset disconnected");
            headsetSmClearDisconnectLock();
            DEBUG_LOG_VERBOSE("headsetSmUpdateDisconnectingLink: Lock cleared");
        }
        else
        {
            DEBUG_LOG_DEBUG("headsetSmUpdateDisconnectingLink: Still connected");
        }
    }
}

/*! \brief Handle notification of handset disconnection. */
#ifdef ENABLE_APP_AUTO_POWEROFF_TIMER
static void headsetSmHandleHandsetServiceDisconnectedInd(HANDSET_SERVICE_DISCONNECTED_IND_T *ind)
{
    DEBUG_LOG_DEBUG("headsetSmHandleHandsetServiceDisconnectedInd status %u", ind->status);
    headsetSmUpdateDisconnectingLink();

    /* if USB is allowed along with BT, then it could so happen that BT disconnects while USB is streaming */
    if(!headsetSmStateIsPairing(headsetGetState()) && headsetSmStateIsIdle(headsetSmDetermineCoreState()))
    {
        /* Inform UI that a handset has been disconnected */
        Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle);
    }

    headsetSMStartIdleTimer();
}
#else
static void headsetSmHandleHandsetServiceDisconnectedInd(HANDSET_SERVICE_DISCONNECTED_IND_T *ind)
{
    DEBUG_LOG_DEBUG("headsetSmHandleHandsetServiceDisconnectedInd status %u", ind->status);
    headsetSmUpdateDisconnectingLink();

    /* if USB is allowed along with BT, then it could so happen that BT disconnects while USB is streaming */
    if(!headsetSmStateIsPairing(headsetGetState()) && headsetSmStateIsIdle(headsetSmDetermineCoreState()))
    {
        /* Inform UI that a handset has been disconnected */
        Ui_InformContextChange(ui_provider_app_sm, context_app_sm_idle);
    }
    if(headsetSmIsIdleTimerNeedToStart())
    {
        headsetSMStartIdleTimer();
    }
}
#endif
static headsetState headsetSmDetermineCoreState(void)
{
    bool busy = IsHeadsetAudioActive();
    headsetState current_state = headsetGetState();
    
    if ((HEADSET_STATE_IDLE == current_state) || (HEADSET_STATE_BUSY == current_state))
    {
        return busy ? HEADSET_STATE_BUSY:
                          HEADSET_STATE_IDLE;
    }
    else
    {
        return current_state;
    }
}

bool headetSmHandleTopologyStopCfm(Message message)
{
    UNUSED(message);
    DEBUG_LOG_VERBOSE("headetSmHandleTopologyStopCfm");

    if(headsetGetState() == HEADSET_STATE_POWERING_OFF)
    {
        /* Stop topology had called as part of power off. */
        headsetSetState(HEADSET_STATE_LIMBO);
        appPowerOffRequest();
    }
    else
    {
       /* Topology has stopped. Enable Wired audio controller. */
       HeadsetWiredAudioController_Enable();
    }
    return TRUE;
}

void headsetSmWiredAudioConnected(void)
{
    headsetState current_state = headsetGetState();

    DEBUG_LOG_INFO("headsetSmWiredAudioConnected Headset state %d",current_state);
    switch(current_state)
    {
        case HEADSET_STATE_POWERING_ON:
        case HEADSET_STATE_IDLE:
        case HEADSET_STATE_BUSY:
        {
          /* Move to Busy state */
            headsetSetState(HEADSET_STATE_BUSY);
        }
        break;

        default:
        break;
    }
}

void headsetSmWiredAudioDisconnected(void)
{
    headsetState current_state = headsetGetState();

    DEBUG_LOG_INFO("headsetSmWiredAudioDisconnected Headset state %d",current_state);
    switch(current_state)
    {
        case HEADSET_STATE_BUSY:
        {
            /* Move to headset idle state */
            headsetSetState(HEADSET_STATE_IDLE);
        }
        break;

        default:
        break;
    }
}

#ifdef ENABLE_APP_MD_GAIA
/*! \brief Reboot the earbud, no questions asked. */
static void appSmHandleInternalReboot(void)
{
    SystemReboot_RebootWithAction(reboot_action_default_state);
}
#endif

void headsetSmHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    if (isMessageUiInput(id))
    {
       headsetSmHandleUiInput(id);
       return;
    }

    switch (id)
    {
#ifdef INCLUDE_DFU
        case DFU_REQUESTED_TO_CONFIRM:
            /* Upgrade almost over, Stay in upgrade mode for upgrade to be completed */
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_REQUESTED_TO_CONFIRM");
            Dfu_SetRebootReason(REBOOT_REASON_DFU_RESET);
            break;

        case DFU_REQUESTED_IN_PROGRESS:
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_REQUESTED_IN_PROGRESS");
            Dfu_SetRebootReason(REBOOT_REASON_ABRUPT_RESET);
            break;

        case DFU_STARTED:
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_STARTED");
            break;

        case DFU_COMPLETED:
            GattServerGatt_SetGattDbChanged();
            Dfu_SetRebootReason(REBOOT_REASON_NONE);

#ifdef ENABLE_APP_OTA_FINISH_RECONNECT_LED
            appSetOtaRebootFlag(TRUE);
#endif
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_COMPLETED");
            break;

        case DFU_ABORTED:
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_ABORTED");
            break;

        case DFU_READY_FOR_SILENT_COMMIT:
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_DFU_READY_FOR_SILENT_COMMIT");
            break;
#endif
        case ANC_UPDATE_STATE_ENABLE_IND:
             headsetSmHandleAncUpdateStateEnableInd();
             DEBUG_LOG_DEBUG("headsetSmHandleMessage ANC_UPDATE_STATE_ENABLE_IND");
             break;

        case ANC_UPDATE_STATE_DISABLE_IND:
             headsetSmHandleAncUpdateStateDisableInd();
             DEBUG_LOG_DEBUG("headsetSmHandleMessage ANC_UPDATE_STATE_DISABLE_IND");
             break;

        case LEAKTHROUGH_UPDATE_STATE_IND:
             headsetSmHandleLeakthroughStateInd((LEAKTHROUGH_UPDATE_STATE_IND_T *)message);
             DEBUG_LOG_DEBUG("headsetSmHandleMessage LEAKTHROUGH_UPDATE_ENABLE_IND");
             break;

        case SYSTEM_STATE_STATE_CHANGE:
             headsetSmHandleSystemStateChange((SYSTEM_STATE_STATE_CHANGE_T *)message);
             break;

        /* Pairing completion confirmations */
        case PAIRING_PAIR_CFM:
            headsetSmHandlePairingPairConfirm((PAIRING_PAIR_CFM_T *)message);
            break;

        /*! Handset Service disconnected indication */
        case HANDSET_SERVICE_DISCONNECTED_IND:
             headsetSmHandleHandsetServiceDisconnectedInd((HANDSET_SERVICE_DISCONNECTED_IND_T *) message);

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
             appGaiaDisconnectDeviceHandle(((HANDSET_SERVICE_DISCONNECTED_IND_T *) message)->addr);
             appGaiaDeleteDeviceHandle(((HANDSET_SERVICE_DISCONNECTED_IND_T *) message)->addr);
#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
             appNotifyDeviceStatus(((HANDSET_SERVICE_DISCONNECTED_IND_T *) message)->addr, gaia_notify_device_status_disconnected, DEVICE_PROFILE_HFP);
#endif

#endif
             break;

        /* Handset service connected indication */
        case HANDSET_SERVICE_CONNECTED_IND:
            headsetSmHandleHandsetServiceConnectedInd((HANDSET_SERVICE_CONNECTED_IND_T*)message);

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
            //appGaiaConnectDeviceHandle(((HANDSET_SERVICE_DISCONNECTED_IND_T *) message)->addr);
#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
            //appNotifyDeviceStatus(((HANDSET_SERVICE_DISCONNECTED_IND_T *) message)->addr, gaia_notify_device_status_connected, DEVICE_PROFILE_HFP);
#endif

#endif
            break;

        /* Topology Messages */
        case STEREO_TOPOLOGY_STOP_CFM:
            headetSmHandleTopologyStopCfm(message);
            break;

        case STEREO_TOPOLOGY_HANDSET_DISCONNECTED_IND:
            headsetSmHandleStereoTopologyHandsetDisInd((STEREO_TOPOLOGY_HANDSET_DISCONNECTED_IND_T *)message);
            break;

        /* TODO Add handling for messages from various registered domains/components */

        /* AV status change indications */
        case AV_STREAMING_ACTIVE_IND:
        case AV_STREAMING_INACTIVE_IND:
        /* HFP status change indications */
        case TELEPHONY_CALL_ENDED:
        case TELEPHONY_CALL_ONGOING:
            if(headsetSmIsActiveState(headsetGetState()))
            {
                headsetSetState(headsetSmDetermineCoreState());
            }
#ifdef INCLUDE_DFU
            /* Inform upgrade library when SCO is connected/disconnected */
            if (id == TELEPHONY_CALL_ONGOING || id == TELEPHONY_CALL_ENDED)
            {
                 telephony_message_t *ind = (telephony_message_t *)message;
                 /* This is not applicable for USB as voice source */
                 if(ind->voice_source != voice_source_usb)
                     UpgradeSetScoActive(id == TELEPHONY_CALL_ONGOING);
            }
#endif
            break;


#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
        case CHARGER_MESSAGE_ATTACHED:
            {
                appEnableExternalBatteryCharing();

                /*--- USB is inserted after 10s, check whether the battery is fully charged --- start ---*/
                MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_CHARGER_COMPLETED_HANDLE);
                MessageSendLater(headsetSmGetTask(), SM_INTERNAL_CHARGER_COMPLETED_HANDLE, NULL, D_SEC(10));
                /*--- USB is inserted after 10s, check whether the battery is fully charged ---  end  ---*/

#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
                if (headsetGetState() == HEADSET_STATE_PAIRING)
                {
                    headsetExitPairing();
                    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_PAIRING_PROMPTS_CYCLE);
                }
#endif
                if (Av_IsA2dpSinkStreaming() == TRUE)
                {
                    HeadsetUsb_AudioDisable(headsetSmGetTask());
                    break;
                }

#ifdef ENABLE_APP_USB_ATTACHED_ENTER_LIMBO
                if (headsetGetState() == HEADSET_STATE_LIMBO)
                {
                    /*--- Usb inserted into Limbo mode --- statr ---*/
                    SystemState_PowerOff();
                    break;
                    /*--- Usb inserted into Limbo mode ---  end  ---*/
                }
                else if ((headsetGetState() >= HEADSET_STATE_PAIRING) &&
                   (headsetGetState() <= HEADSET_STATE_BUSY))
                {
                    SystemState_PowerOff();
                }
#endif

#ifdef ENABLE_APP_USB_AUDIO
                /* Enable USB Audio Feature */
                HeadsetUsb_AudioEnable(headsetSmGetTask());
#endif
            }
            break;
#endif

        /* Charger indications */
        case CHARGER_MESSAGE_DETACHED:
            //headsetSmHandleChargerMessageDetached();
#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
            MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_CHARGER_COMPLETED_HANDLE);
            MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_NTC_CHARGER_COMPLETED_HANDLE);
            appDisableExternalBatteryCharing();
#endif
            if (headsetGetState() == HEADSET_STATE_LIMBO)
            {
                SystemState_Shutdown();
            }
            else if((headsetGetState() >= HEADSET_STATE_PAIRING) &&
                    (headsetGetState() <= HEADSET_STATE_BUSY))
            {
                SystemState_EmergencyShutdown();
            }
            break;

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
        case CHARGER_MESSAGE_COMPLETED:
            {
                if (BatteryRegion_GetCurrent() == 70)
                {
                    //appBatteryChargeComplete();
                    appCharingComplteteHandle();
                }
            }
            break;
#endif

        case CHARGER_MESSAGE_CHARGING_OK:
        case CHARGER_MESSAGE_CHARGING_LOW:
            /* Consume frequently occuring charger messages with no operation required. */
            break;

        /* Power indications */
        case APP_POWER_SHUTDOWN_PREPARE_IND:
            DEBUG_LOG_DEBUG("headsetSmHandleMessage APP_POWER_SHUTDOWN_PREPARE_IND");
            headsetSmHandlePowerShutdownPrepareInd();
            break;

        case SM_INTERNAL_FACTORY_RESET:
            headsetSmHandleInternalFactoryReset();
            break;

        case SM_INTERNAL_PAIR_HANDSET:
            headsetSmHandleInternalPairHandset();
            break;

        case SM_INTERNAL_DELETE_HANDSETS:
            headsetSmHandleInternalDeleteHandsets();
            break;

        case SM_INTERNAL_POWER_OFF:
#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
            if (appKymeraIsPlayingPrompt() == TRUE)
            {
                MessageSendLater(headsetSmGetTask(), SM_INTERNAL_POWER_OFF, NULL, 500);
                break;
            }
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
            if(Charger_IsConnected() == TRUE)
            {
                headsetSetState(HEADSET_STATE_LIMBO);
                break;
            }
#endif
            headsetSmHandleInternalPowerOff();
            break;

        case SM_INTERNAL_TIMEOUT_IDLE:
#ifdef ENABLE_APP_OTA_LED
            if (TwsTopology_IsOtaEnable() == TRUE)
            {
                break;
            }
#endif
            headsetSmHandleTimeoutIdle();
            break;

        case SM_INTERNAL_TIMEOUT_LIMBO:
            headsetSmHandleTimeoutLimbo();
            break;

        case SM_INTERNAL_LINK_DISCONNECTION_COMPLETE:
            headsetSmHandleInternalLinkDisconnectionComplete();
            break;

        case SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION:
            headsetSmLinkDisconnectionTimeout();
            break;

#ifdef ENABLE_APP_LINE_IN_AUDIO
        case SM_INTERNAL_APP_LINE_IN_DETECH:
            {
                //DEBUG_LOG_DEBUG("-----SM_INTERNAL_APP_LINE_IN_DETECH-------");
                //DEBUG_LOG_DEBUG("IsAppLineInAvailable = %d",IsAppLineInAvailable());
                if (g_Record_Line_In_State != IsAppLineInAvailable())
                {
                    g_Record_Line_In_State = IsAppLineInAvailable();

                    if (g_Record_Line_In_State == FALSE)
                    {
                        WiredAudioDetect_StartMonitoring();
                        WiredAudioSource_UpdateClient();

                        /*--- Power on detech line in need to shutdown to enable line in audio --- start ---*/
                        SystemState_EmergencyShutdown();
                        /*--- Power on detech line in need to shutdown to enable line in audio ---  end  ---*/

                        break;
                    }
                    else
                    {
                        WiredAudioDetect_StopMonitoring();
                    }
                }
                MessageSendLater(headsetSmGetTask(), SM_INTERNAL_APP_LINE_IN_DETECH, NULL, 1000);
            }
            break;
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
        case SM_INTERNAL_CHARGER_COMPLETED_HANDLE:
            {
#if 0
                if ((appBatteryGetVoltageAverage() >= appConfigBatteryFullyCharged()) ||
                    ((Charger_IsConnected() == TRUE) && (isAppBatteryCharingComplete() == TRUE)))
#else
                if (((Charger_IsConnected() == TRUE) && (isAppBatteryCharingComplete() == TRUE)))
#endif
                {
                    appBatteryChargeComplete();
                    break;
                }

                MessageSendLater(headsetSmGetTask(), SM_INTERNAL_CHARGER_COMPLETED_HANDLE, NULL, D_SEC(1));
            }
            break;

        case SM_INTERNAL_NTC_CHARGER_COMPLETED_HANDLE:
            {
                if (Charger_IsConnected() == TRUE)
                {
                    appBatteryChargeComplete();
                }
            }
            break;
#endif

#ifdef ENABLE_APP_MD_GAIA
        case SM_INTERNAL_REBOOT:
            {
                appSmHandleInternalReboot();
            }
            break;
#endif

#ifdef ENABLE_APP_HID_COMMAND
        case SM_INTERNAL_APP_DISABLE_USB_AUDIO:
            {
                g_IsAppDisableUsbAudio = TRUE;
                WiredAudioSource_StopMonitoring(headsetSmGetTask());
                HeadsetUsb_AudioDisable(headsetSmGetTask());
            }
            break;
#endif

#ifdef ENABLE_APP_POWERON_ENTER_PAIRING
        case SM_INTERNAL_POWER_ON_300MS_TIMER:
            {
                if (PioCommonGetPio(0) == FALSE)
                {
                    appSetAllowPoweronReconnectFlag(FALSE);
                    appTestConnectHandset();
                }
                else
                {
                    if (appGetAllowPoweronReconnectFlag() == TRUE)
                    {
                        MessageSendLater(headsetSmGetTask(), SM_INTERNAL_POWER_ON_300MS_TIMER, NULL, 100);
                    }
                }
            }
            break;
#endif

#ifdef ENABLE_APP_PAIRING_PROMPTS_CYCLE
        case SM_INTERNAL_PAIRING_PROMPTS_CYCLE:
            {
                if (appKymeraIsPlayingPrompt() == TRUE)
                {
                    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_PAIRING_PROMPTS_CYCLE, NULL, 500);
                    break;
                }

                if (headsetGetState() == HEADSET_STATE_PAIRING)
                {
                    UiPrompts_SendEvent(PAIRING_ACTIVE, 0);
                    //MessageSendLater(headsetSmGetTask(), SM_INTERNAL_PAIRING_PROMPTS_CYCLE, NULL, 3500);
                    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_PAIRING_PROMPTS_CYCLE, NULL, 1000);
                }
            }
            break;

        case SM_INTERNAL_APP_FACTORY_RESET_HANDLE:
            {
                if (appKymeraIsPlayingPrompt() == TRUE)
                {
                    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_APP_FACTORY_RESET_HANDLE, NULL, 500);
                    break;
                }
#ifdef ENABLE_APP_FACTORY_RESET
                MessageCancelAll(headsetSmGetTask(),SM_INTERNAL_PAIRING_PROMPTS_CYCLE);
                appRestoreDeviceName();
                appRestoreDefaultSetting();
                appFactoryReset();
                //UiPrompts_SendEvent(POWER_OFF, 0);
                MessageSendLater(headsetSmGetTask(), SM_INTERNAL_FACTORY_RESET, NULL, 1800);
#endif
            }
            break;
#endif

#ifdef ENABLE_APP_DISABLE_POWER_OFF_AFTER_POWER_ON_3S
        case SM_INTERNAL_POWER_ON_3S_TIMER:
            {
                g_EnablePowerOff = TRUE;
            }
            break;

        case SM_INTERNAL_FORCE_POWER_OFF:
            {
                if (headsetGetState() == HEADSET_STATE_TERMINATING)
                {
                    SystemState_EmergencyShutdown();
                }
            }
            break;
#endif

#ifdef ENABLE_APP_FIX_PO_NOISE
        case SM_INTERNAL_APP_MUTE_MAIN_VOLUME:
            {
                appSpeakerAmpMute();
                appSetMuteMainFlag(TRUE);
                //appEnableSpeakerUnMuteTimer();
            }
            break;

        case SM_INTERNAL_APP_UNMUTE_MAIN_VOLUME:
            {
                appSpeakerAmpUnMute();
                appSetMuteMainFlag(FALSE);
            }
            break;
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
        case SM_INTERNAL_APP_GAIA_CMD_CONNECT_DEVICE_TIMER:
            {
                appGaiaConnectDeviceFailHandle();
            }
            break;
#endif

        default:
            UnexpectedMessage_HandleMessage(id);
            break;
    }
}
/*! \brief  Provides Headset active state context to the User Interface module.Serves as a callback to the
 *          UI for current context. Returns only context_app_sm_active / context_app_sm_inactive.

    \param[in]  void

    \return     current_sm_ctxt - current context of sm module.
*/
static unsigned headsetSm_GetApplicationActiveStateContext(void)
{
    sm_provider_context_t context = context_app_sm_active;

    if(headsetSm_GetApplicationCurrentContext() == context_app_sm_powered_off)
    {
        context = context_app_sm_inactive;
    }

    return (unsigned)context;
}

/*! \brief  Indicates whether the Application is currently screening Logical Inputs
            (i.e. button presses) to inhibit the generation of UI Inputs in the UI domain

    \param[in]  logical_input - the logical input to check for

    \return     bool - TRUE if screening is active.
*/
static bool headsetSm_IsLogicalInputScreeningActive(unsigned logical_input)
{
    bool screen_logical_input = FALSE;
    if (headsetSm_GetApplicationCurrentContext() == context_app_sm_powered_off)
    {
        /* Ensure we don't screen the POWER_ON button press, or any other non-screened events */
        if (HeadsetUi_IsLogicalInputScreenedInLimboState(logical_input))
        {
            screen_logical_input = TRUE;
        }
    }
    return screen_logical_input;
}

/*! \brief Initiate disconnect of handset link */
bool headsetSmDisconnectLink(void)
{
    bdaddr handset_addr = {0};
    bool disconnecting = FALSE;

    if (appDeviceGetHandsetBdAddr(&handset_addr) && appDeviceIsBredrHandsetConnected())
    {
        StereoTopology_DisconnectAllHandsets();
        disconnecting = TRUE;
    }

    return disconnecting;
}

/*! \brief Initialise the main application state machine.
 */
bool headsetSmInit(Task init_task)
{
    smTaskData* sm = SmGetTaskData();
    memset(sm, 0, sizeof(*sm));
    sm->task.handler = headsetSmHandleMessage;
    sm->state = HEADSET_STATE_NULL;
    sm->disconnect_lock= 0;
    sm->user_pairing = FALSE;
#ifdef ENABLE_HEADSET_AUTO_POWER_ON
    sm->auto_poweron = TRUE;
#else
    sm->auto_poweron = FALSE;
#endif

    if( (SystemReboot_GetAction() == reboot_action_active_state) || headsetSmIsLastResetDueToPanic() )
    {
        headsetSmSetAutoPowerOn();
        SystemReboot_ResetAction();
    }
    /* register with connection manager to get notification of (dis)connections */
    ConManagerRegisterConnectionsClient(&sm->task);

    /* register with Telephony service for changes in state */
    Telephony_RegisterForMessages(&sm->task);

    /* register with AV to receive notifications of A2DP and AVRCP activity */
    appAvStatusClientRegister(&sm->task);

    /* register with power to receive sleep/shutdown messages. */
    appPowerClientRegister(&sm->task);

    /* register with charger monitor to receive charger messages. */    
    (void)Charger_ClientRegister(&sm->task);

    /* register with handset service as we need disconnect and connect notification */
    HandsetService_ClientRegister(&sm->task);

    /* Register for topology message indications */
    StereoTopology_RegisterMessageClient(headsetSmGetTask());

    /* Register for connection manager TP message indication */
    ConManagerRegisterTpConnectionsObserver(cm_transport_bredr, headsetSmGetTask());
    
    /* Register for system state change indications */
    SystemState_RegisterForStateChanges(&sm->task);

    Ui_RegisterUiInputConsumer(headsetSmGetTask(), sm_ui_inputs, ARRAY_DIM(sm_ui_inputs));

    /* Register sm as ui provider*/
    Ui_RegisterUiProvider(ui_provider_app_sm, headsetSm_GetApplicationActiveStateContext );
    Ui_RegisterLogicalInputScreeningDecider(headsetSm_IsLogicalInputScreeningActive);

    /* Register with ANC state manager to receive ANC ON/OFF notifications */
    AncStateManager_ClientRegister(&sm->task);

    /* Register to receive Leakthrough Enable/Disable notifications */
    AecLeakthrough_ClientRegister(&sm->task);

#ifdef ALLOW_USB_BT_COEXISTENCE
    /* Register for audio sources notifications for USB */
    AudioSources_RegisterObserver(audio_source_usb, &headset_sm_observer_interface);
#endif /*#ifndef ALLOW_USB_BT_COEXISTENCE */


    /* If DFU support is enabled, then set the QoS as low latency for better
     * DFU performance over LE Transport.
     * This will come at the cost of high power consumption.
     */

#ifdef ENABLE_APP_MD_GAIA
    appPowerOnGetPsSetting();
#endif

#ifdef ENABLE_APP_LINE_IN_AUDIO
    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_APP_LINE_IN_DETECH, NULL, 500);
#endif

    UNUSED(init_task);
    return TRUE;
}

#ifdef ENABLE_APP_MD_GAIA
void appSmReboot_Delay(void)
{
    appPoweroffStorePskeyData();

    appTestDeleteHandset();

    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_REBOOT, NULL, D_SEC(6));
}

uint32 appConfigIdleTimeoutMs_HandsetConected(void)
{
    return g_AppGaiaChangePskeyData.idle_auto_poweroff_time;
}

void set_appConfigIdleTimeoutMs_HandsetConected(uint8 timer)
{
    g_AppGaiaChangePskeyData.idle_auto_poweroff_time = timer;
}

static void appConfigIdleTimeoutInit(void)
{
    uint8 config;

    if(PsRetrieve(PSKEY_INN_AUTO_OFF_FEATURE_APP_CONFIG,&config,1) == 1)
    {
        switch(config)
        {
            case AUTO_OFF_TIMEOUT_NEVER:
                set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_NEVER);
                g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_NEVER;
                break;

            case AUTO_OFF_TIMEOUT_30MIN:
                set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_30MIN);
                g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_30MIN;
                break;

            case AUTO_OFF_TIMEOUT_1HOUR:
                set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_1HOUR);
                g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_1HOUR;
                break;

            case AUTO_OFF_TIMEOUT_3HOUR:
                set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_3HOUR);
                g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_3HOUR;
                break;

            default:
                set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_30MIN);
                g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_30MIN;
                break;
        }
    }
    else
    {
        set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_30MIN);
        g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_30MIN;
    }
}

#ifdef ENABLE_APP_EQ_SWITCH
uint8 appGetEqmode(void)
{
    return g_AppGaiaChangePskeyData.eq_mode;
}

static void appPoweronSetEqmode(uint8 mode)
{
    g_AppGaiaChangePskeyData.eq_mode = mode;
}

bool appHeadSetEqMode(uint8 mode)
{
    uint8 bankNum = 0x00;
    bool status = FALSE;

    switch (mode)
    {
        case EQ_MODE_BASS_BOOST:
        {
            bankNum = EQ_BASS_BOOST_BANK_ID;
        }
        break;

        case EQ_MODE_BASS_CUT:
        {
            bankNum = EQ_BASS_CUT_BANK_ID;
        }
        break;

        case EQ_MODE_PODCAST:
        {
            bankNum = EQ_PODCAST_BANK_ID;
        }
        break;

        case EQ_MODE_OFF:
        {
            bankNum = EQ_OFF_BANK_ID;
        }
        break;

        case EQ_MODE_AUDIOPHILE:
        {
            bankNum = EQ_AUDIOPHILE_BANK_ID;
        }
        break;

        default:
        {
            bankNum = EQ_OFF_BANK_ID;
        }
        break;
    }

    status = appSetEqMode(bankNum);

    if (status == TRUE)
    {
        g_AppGaiaChangePskeyData.eq_mode = mode;
    }

    return status;
}

static void appPowerOnEqModeInit(void)
{
    uint8 config;

    if(PsRetrieve(PSKEY_INN_EQ_MODE_APP_CONFIG,&config,1) == 1)
    {
        if (config > EQ_MODE_MIN && config < EQ_MODE_MAX)
        {
            appPoweronSetEqmode(config);
            g_AppPoweronGetPskeyData.eq_mode = config;
        }
        else
        {
            appPoweronSetEqmode(EQ_MODE_OFF);
            g_AppPoweronGetPskeyData.eq_mode = EQ_MODE_OFF;
        }
    }
    else
    {
        appPoweronSetEqmode(EQ_MODE_OFF);
        g_AppPoweronGetPskeyData.eq_mode = EQ_MODE_OFF;
    }
}
#endif

#ifdef ENABLE_APP_SIDETONE
void appEnableSideTone(uint8 val)
{
    switch(val)
    {
        case SIDETONE_SETTING_ENABLE:
            {
                appSetSideTone(TRUE);
                g_AppGaiaChangePskeyData.sidetone_status = SIDETONE_SETTING_ENABLE;
            }
            break;

        case SIDETONE_SETTING_DISABLE:
            {
                appSetSideTone(FALSE);
                g_AppGaiaChangePskeyData.sidetone_status = SIDETONE_SETTING_DISABLE;
            }
            break;

        default:
            break;
    }
}

uint8 appGetSidetoneStatus(void)
{
    return g_AppGaiaChangePskeyData.sidetone_status;
}

static void appPoweronSetSidetoneStatus(uint8 status)
{
    g_AppGaiaChangePskeyData.sidetone_status = status;
}

static void appPowerOnSidetoneInit(void)
{
    uint8 config;

    if(PsRetrieve(PSKEY_INN_SIDETONE_APP_CONFIG,&config,1) == 1)
    {
        if (config > SIDETONE_SETTING_MIN && config < SIDETONE_SETTING_MAX)
        {
            appPoweronSetSidetoneStatus(config);
            g_AppPoweronGetPskeyData.sidetone_status = config;
        }
        else
        {
            appPoweronSetSidetoneStatus(SIDETONE_SETTING_ENABLE);
            g_AppPoweronGetPskeyData.sidetone_status = SIDETONE_SETTING_ENABLE;
        }
    }
    else
    {
        appPoweronSetSidetoneStatus(SIDETONE_SETTING_ENABLE);
        g_AppPoweronGetPskeyData.sidetone_status = SIDETONE_SETTING_ENABLE;
    }
}
#endif

#ifdef ENABLE_APP_MIC_MUTE
uint8 appGetMicMuteControlStatus(void)
{
    return g_AppGaiaChangePskeyData.mic_mute_control_status;
}

void appSetMicMuteControlStatus(uint8 status)
{
    if (status != MIC_MUTE_CONTROL_GET_STATUS_ID)
    {
        g_AppGaiaChangePskeyData.mic_mute_control_status = status;
    }
}


static void appPowerOnMicMuteControlStateInit(void)
{
    uint8 config;

    if(PsRetrieve(PSKEY_INN_MIC_MUTE_STATE,&config,1) == 1)
    {
        if (config > MIC_MUTE_CONTROL_MIN && config < MIC_MUTE_CONTROL_MAX)
        {
            appSetMicMuteControlStatus(config);
            g_AppPoweronGetPskeyData.mic_mute_control_status = config;
        }
        else
        {
            appSetMicMuteControlStatus(MIC_MUTE_CONTROL_ENABLE);
            g_AppPoweronGetPskeyData.mic_mute_control_status = MIC_MUTE_CONTROL_ENABLE;
        }
    }
    else
    {
        appSetMicMuteControlStatus(MIC_MUTE_CONTROL_ENABLE);
        g_AppPoweronGetPskeyData.mic_mute_control_status = MIC_MUTE_CONTROL_ENABLE;
    }
}
#endif

static void appPowerOnGetPsSetting(void)
{
#ifdef ENABLE_APP_EQ_SWITCH
    appPowerOnEqModeInit();
#endif

#ifdef ENABLE_APP_SIDETONE
    appPowerOnSidetoneInit();
#endif

#ifdef ENABLE_APP_MIC_MUTE
    appPowerOnMicMuteControlStateInit();
#endif

    appConfigIdleTimeoutInit();
}

void appPoweroffStorePskeyData(void)
{
    uint8 configed = 0x00;
    if (g_AppGaiaChangePskeyData.eq_mode != g_AppPoweronGetPskeyData.eq_mode)
    {
        configed = g_AppGaiaChangePskeyData.eq_mode;
        if(PsStore(PSKEY_INN_EQ_MODE_APP_CONFIG,&configed,1) != 0x01)
        {
            DEBUG_LOG_INFO("appPoweroffStorePskeyData store eq pskey fail");
        }
    }

    if (g_AppGaiaChangePskeyData.sidetone_status != g_AppPoweronGetPskeyData.sidetone_status)
    {
        configed = g_AppGaiaChangePskeyData.sidetone_status;
        if(PsStore(PSKEY_INN_SIDETONE_APP_CONFIG,&configed,1) != 0x01)
        {
            DEBUG_LOG_INFO("appPoweroffStorePskeyData store sidetone pskey fail");
        }
    }

    if (g_AppGaiaChangePskeyData.mic_mute_control_status != g_AppPoweronGetPskeyData.mic_mute_control_status)
    {
        configed = g_AppGaiaChangePskeyData.mic_mute_control_status;
        if(PsStore(PSKEY_INN_MIC_MUTE_STATE,&configed,1) != 0x01)
        {
            DEBUG_LOG_INFO("appPoweroffStorePskeyData store mic mute control state pskey fail");
        }
    }

    if (g_AppGaiaChangePskeyData.idle_auto_poweroff_time != g_AppPoweronGetPskeyData.idle_auto_poweroff_time)
    {
        configed = g_AppGaiaChangePskeyData.idle_auto_poweroff_time;
        if(PsStore(PSKEY_INN_AUTO_OFF_FEATURE_APP_CONFIG,&configed,1) != 0x01)
        {
            DEBUG_LOG_INFO("appPoweroffStorePskeyData store idle_auto_poweroff_time pskey fail");
        }
    }
}

void appGaiaRestoreDefaultSetting(void)
{
    /*Reset EQ mode*/
    if (appGetEqmode() != EQ_MODE_OFF)
    {
        appHeadSetEqMode(EQ_MODE_OFF);
    }

    /*Reset sidetone*/
    if (appGetSidetoneStatus() != SIDETONE_SETTING_ENABLE)
    {
        appEnableSideTone(SIDETONE_SETTING_ENABLE);
    }

    /*Reset mic mute control state*/
    if (appGetMicMuteControlStatus() != MIC_MUTE_CONTROL_ENABLE)
    {
        appSetMicMuteControlStatus(MIC_MUTE_CONTROL_ENABLE);
    }

    /*Reset auto poweroff timeout*/
    if (appConfigIdleTimeoutMs_HandsetConected() != AUTO_OFF_TIMEOUT_30MIN)
    {
        set_appConfigIdleTimeoutMs_HandsetConected(AUTO_OFF_TIMEOUT_30MIN);
    }
}

void appRestoreDefaultSetting(void)
{
     uint8 configed = 0;
     /*Reset EQ mode*/
     if (appGetEqmode() != EQ_MODE_OFF)
     {
         configed = EQ_MODE_OFF;
         appHeadSetEqMode(configed);
         g_AppPoweronGetPskeyData.eq_mode = EQ_MODE_OFF;
         if(PsStore(PSKEY_INN_EQ_MODE_APP_CONFIG,&configed,1) != 1)
         {
             DEBUG_LOG_INFO("appRestoreDefaultSetting store eq mode pskey fail");
         }
     }

     /*Reset sidetone*/
     if (appGetSidetoneStatus() != SIDETONE_SETTING_ENABLE)
     {
         configed = SIDETONE_SETTING_ENABLE;
         appEnableSideTone(configed);
         g_AppPoweronGetPskeyData.sidetone_status = SIDETONE_SETTING_ENABLE;
         if(PsStore(PSKEY_INN_SIDETONE_APP_CONFIG,&configed,1) != 1)
         {
             DEBUG_LOG_INFO("appRestoreDefaultSetting store sidetone pskey fail");
         }
     }

     /*Reset mic mute control state*/
     if (appGetMicMuteControlStatus() != MIC_MUTE_CONTROL_ENABLE)
     {
         configed = MIC_MUTE_CONTROL_ENABLE;
         appSetMicMuteControlStatus(configed);
         g_AppPoweronGetPskeyData.mic_mute_control_status = MIC_MUTE_CONTROL_ENABLE;
         if(PsStore(PSKEY_INN_MIC_MUTE_STATE,&configed,1) != 1)
         {
             DEBUG_LOG_INFO("appRestoreDefaultSetting store ic mute control state pskey fail");
         }
     }

     /*Reset auto poweroff timeout*/
     if (appConfigIdleTimeoutMs_HandsetConected() != AUTO_OFF_TIMEOUT_30MIN)
     {
         configed = AUTO_OFF_TIMEOUT_30MIN;
         set_appConfigIdleTimeoutMs_HandsetConected(configed);
         g_AppPoweronGetPskeyData.idle_auto_poweroff_time = AUTO_OFF_TIMEOUT_30MIN;
         if(PsStore(PSKEY_INN_AUTO_OFF_FEATURE_APP_CONFIG,&configed,1) != 1)
         {
             DEBUG_LOG_INFO("appRestoreDefaultSetting store Auto-off feature pskey fail");
         }
         if(appHeadsetGetState() == HEADSET_STATE_IDLE)
         {
             headsetSMStartIdleTimer();
         }
     }
}

void ChangeLocalName(void *param)
{
   ConnectionChangeLocalName(strlen((char*)param),(uint8*)param);

   LocalName_SetTymPsKeyName(strlen((char*)param),(uint8*)param);

   //must also update LocalName
   LocalName_Init(NULL);
}

void appRestoreDeviceName(void)
{
    uint8 notify_name[32] = "M&D MH40W";
    /*payload[0] is name length*/
    ChangeLocalName(notify_name);
}
#endif

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
void appCharingComplteteHandle(void)
{
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_NTC_CHARGER_COMPLETED_HANDLE);
    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_NTC_CHARGER_COMPLETED_HANDLE, NULL, D_SEC(5));
}
#endif

void appHeadsetSMStartIdleTimer(void)
{
    headsetSMStartIdleTimer();
}

void appHeadsetSMStopIdleTimer(void)
{
    headsetSMStopIdleTimer();
}


#ifdef ENABLE_APP_HID_COMMAND
void appHeadsetSmHandlePowerOn(void)
{
    headsetSmHandlePowerOn();
}
#endif

#ifdef ENABLE_APP_MIC_MUTE
void appSetMicMuteFlag(bool val)
{
    g_MuteFlag = val;
}
#endif

#ifdef ENABLE_APP_POWEROFF_DISPLAY_DISCONNECTED_PROMPT
void appSetEnablePlayDisconnectPromptFlag(bool val)
{
    g_EnablePlayDisconnectPrompt = val;
}

bool appGetEnablePlayDisconnectPromptFlag(void)
{
    return g_EnablePlayDisconnectPrompt;
}
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
void appGaiaConnectDeviceCancelTimer(void)
{
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_APP_GAIA_CMD_CONNECT_DEVICE_TIMER);
}

void appGaiaConnectDeviceStartTimer(void)
{
    MessageCancelAll(headsetSmGetTask(), SM_INTERNAL_APP_GAIA_CMD_CONNECT_DEVICE_TIMER);
    MessageSendLater(headsetSmGetTask(), SM_INTERNAL_APP_GAIA_CMD_CONNECT_DEVICE_TIMER, NULL, D_SEC(10));
}
#endif

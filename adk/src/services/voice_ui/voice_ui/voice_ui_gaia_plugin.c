/*!
\copyright  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file
\brief      GAIA interface with Voice Assistants.
*/

#ifdef INCLUDE_GAIA

#include "gaia_framework.h"
#include "voice_ui_config.h"
#include "voice_ui_container.h"
#include "voice_ui_gaia_plugin.h"
#include <gaia_features.h>
#include <logging.h>

#ifdef ENABLE_APP_MD_GAIA
#include "hfp_profile_battery_level.h"
#include "local_name.h"
#include "headset_sm.h"
#include "tym_ps.h"
#include "device_info.h"
#include "headset_test.h"
#define HEADSET_DEVICE_NAME_SIZE_MAX 18

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
#include "connection_manager.h"
#include "device_properties.h"
#include "handset_service.h"
#include "remote_name.h"

typedef enum
{
    ONLY_PRIMARY_DEVICE = 0x01,
    PRIMARY_AND_SECONDARY_DEVICE = 0x02,
    NO_DEVICE_CONNECTED = 0xFF,
}DEVICE_CONNECTED_STATUE;

typedef enum
{
    DEVICE_CONNECTED_INFO_MIN_IDX = 0x00,
    DEVICE_CONNECTED_INFO_MAX_IDX = 0x07,
}DEVICE_CONNECTED_NUM;

typedef struct
{
    bool isReceiveGaiaCmd;
    uint8 responsePayload[2];
    bdaddr saveAddr;
    GAIA_TRANSPORT *t;
}AppGaiaSetPdlStateInfo;

static AppGaiaSetPdlStateInfo appPdlRemoveInfo = {0x00};
static AppGaiaSetPdlStateInfo appPdlConnectInfo = {0x00};
static AppGaiaSetPdlStateInfo appPdlDisconnectInfo = {0x00};
#endif
#endif

/* may report voice_ui_provider_none as well as implemented providers */
#define MAX_NO_VA_REPORTED (MAX_NO_VA_SUPPORTED + 1)

#if 0
static void voiceUiGaiaPlugin_SendInvalidParameter(GAIA_TRANSPORT *t, uint8 pdu_id)
{
    GaiaFramework_SendError(t, GAIA_VOICE_UI_FEATURE_ID, pdu_id, invalid_parameter);
}


static void voiceUiGaiaPlugin_SendResponse(GAIA_TRANSPORT *t, uint8 pdu_id, uint8 length, uint8 * payload)
{
    GaiaFramework_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, pdu_id, length, payload);
}

static void voiceUiGaiaPlugin_GetSelectedAssistant(GAIA_TRANSPORT *t)
{
    uint8 selected_assistant = VoiceUi_GetSelectedAssistant();
    voiceUiGaiaPlugin_SendResponse(t, voice_ui_gaia_get_selected_assistant, 1, &selected_assistant);
}


static void voiceUiGaiaPlugin_SetSelectedAssistant(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    bool status = FALSE;

    if (payload_length == 1)
    {
        status = VoiceUi_SelectVoiceAssistant(payload[0], voice_ui_reboot_allowed);
    }

    if (status)
    {
        voiceUiGaiaPlugin_SendResponse(t, voice_ui_gaia_set_selected_assistant, 0, NULL);
    }
    else
    {
        voiceUiGaiaPlugin_SendInvalidParameter(t, voice_ui_gaia_set_selected_assistant);
    }
}


static void voiceUiGaiaPlugin_GetSupportedAssistants(GAIA_TRANSPORT *t)
{
    uint16 count;
    uint8 response[MAX_NO_VA_REPORTED + 1];

    count = VoiceUi_GetSupportedAssistants(response + 1);
    response[0] = count;
    voiceUiGaiaPlugin_SendResponse(t, voice_ui_gaia_get_supported_assistants, count + 1, response);
}
#endif

#ifdef ENABLE_APP_MD_GAIA
/* After set command, remote device acknowledge */
//static const uint8 success_back_value    = 0xF1;
static const uint8 failed_back_value     = 0xF0;

static void gaiaHeadsetPlugin_GetProductID(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    DEBUG_LOG_INFO("gaiaHeadsetPlugin_GetProductID");
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_product_id, failed_back_value);
        return;
    }

    if (payload[0] == 0xFF)
    {
        static const uint8 value[2] = { 0xF0, 0x05 };
        DEBUG_LOG_INFO("gaiaHeadsetPlugin_GetProductID:Success");

        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_product_id, sizeof(value), value);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_product_id, failed_back_value);
    }
}

static void gaiaHeadsetPlugin_GetBatteryLevel(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_battery_level, failed_back_value);
        return;
    }

    if (payload[0] == 0xFF)
    {
        uint8 value = AppGaiaGethfpProfileBatteryLevel();
        DEBUG_LOG_INFO("gaiaHeadsetPlugin_GetBatteryLevel  BatteryGetPercen: %d",value);

        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_battery_level, sizeof(value), &value);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_battery_level, failed_back_value);
    }

}

static void gaiaHeadsetPlugin_DeviceNameRevision(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    DEBUG_LOG("gaiaHeadsetPlugin_DeviceNameRevision");

    DEBUG_LOG("payload_length = %d, payload[0] = %d", payload_length, payload[0]);

    if(payload_length < 2 || payload[0] > HEADSET_DEVICE_NAME_SIZE_MAX)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_device_name_revision, failed_back_value);
        return;
    }

    uint8 notify_name[32] = { '\0' };
    uint8 val = 0x01;
    memcpy(notify_name,&payload[1],payload[0]);

    /*payload[0] is name length*/
    ChangeLocalName(notify_name);
    GaiaFramework_MD_SendResponse(t, GAIA_CORE_FEATURE_ID, gaia_device_name_revision, sizeof(val), &val);

    /* changed name down,reboot to update name delay 2s*/
    appSmReboot_Delay();
}

static void gaiaHeadsetPlugin_GetAutoOffStatus(GAIA_TRANSPORT *t)
{
    uint8 value;

    switch(appConfigIdleTimeoutMs_HandsetConected())
    {
        case AUTO_OFF_TIMEOUT_NEVER:
            value = AUTO_OFF_TIMEOUT_NEVER_ID;
            break;

        case AUTO_OFF_TIMEOUT_30MIN:
            value = AUTO_OFF_TIMEOUT_30MIN_ID;
            break;

        case AUTO_OFF_TIMEOUT_1HOUR:
            value = AUTO_OFF_TIMEOUT_1HOUR_ID;
            break;

        case AUTO_OFF_TIMEOUT_3HOUR:
            value = AUTO_OFF_TIMEOUT_3HOUR_ID;
            break;

        default:
            value = AUTO_OFF_TIMEOUT_NEVER_ID;
            break;
    }

    DEBUG_LOG_INFO("gaiaEarbudPlugin_GetAutoOffStatus: %d", value);

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_auto_off_timer, sizeof(value), &value);
}


static void gaiaHeadsetPlugin_SetAutoOffFeature(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_auto_off_timer, failed_back_value);
        return;
    }

    uint8 configed = 0;
    uint8 rx = payload[0];
    switch(payload[0])
    {
        case AUTO_OFF_GET_TIMEOUT_TIMER_ID:
            gaiaHeadsetPlugin_GetAutoOffStatus(t);
            return;

        case AUTO_OFF_TIMEOUT_NEVER_ID:
            configed = AUTO_OFF_TIMEOUT_NEVER;
            break;

        case AUTO_OFF_TIMEOUT_30MIN_ID:
            configed = AUTO_OFF_TIMEOUT_30MIN;
            break;

        case AUTO_OFF_TIMEOUT_1HOUR_ID:
            configed = AUTO_OFF_TIMEOUT_1HOUR;
            break;

        case AUTO_OFF_TIMEOUT_3HOUR_ID:
            configed = AUTO_OFF_TIMEOUT_3HOUR;
            break;

        default:
            break;
    }

    if(configed)
    {
        if (appConfigIdleTimeoutMs_HandsetConected() != configed)
        {
            set_appConfigIdleTimeoutMs_HandsetConected(configed);
        }
#if 0
        if(PsStore(PSKEY_INN_AUTO_OFF_FEATURE_APP_CONFIG,&configed,1) != 1)
        {
            DEBUG_LOG_INFO("store Auto-off feature pskey fail");
        }
#endif
        if(appHeadsetGetState() == HEADSET_STATE_IDLE)
        {
            appHeadsetSMStartIdleTimer();
        }

        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_auto_off_timer, sizeof(rx), &rx);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_auto_off_timer, failed_back_value);
    }
}

#ifdef ENABLE_APP_EQ_SWITCH
static void gaiaHeadsetPlugin_GetEqModeStatus(GAIA_TRANSPORT *t)
{
    uint8 value;

    switch(appGetEqmode())
    {
        case EQ_MODE_BASS_BOOST:
            value = EQ_MODE_BASS_BOOST_ID;
            break;

        case EQ_MODE_BASS_CUT:
            value = EQ_MODE_BASS_CUT_ID;
            break;

        case EQ_MODE_PODCAST:
            value = EQ_MODE_PODCAST_ID;
            break;

        case EQ_MODE_OFF:
            value = EQ_MODE_OFF_ID;
            break;

        case EQ_MODE_AUDIOPHILE:
            value = EQ_MODE_AUDIOPHILE_ID;
            break;

        default:
            value = EQ_MODE_OFF_ID;
            break;
    }

    DEBUG_LOG_INFO("gaiaEarbudPlugin_GetAutoOffStatus: %d", value);

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_eq_setting, sizeof(value), &value);

    return;
}

static void gaiaHeadsetPlugin_SetEqMode(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_eq_setting, failed_back_value);
        return;
    }

    uint8 configed = 0;

    switch(payload[0])
    {
        case EQ_MODE_GET_STATUS_ID:
            gaiaHeadsetPlugin_GetEqModeStatus(t);
            return;

        case EQ_MODE_BASS_BOOST_ID:
            configed = EQ_MODE_BASS_BOOST;
            break;

        case EQ_MODE_BASS_CUT_ID:
            configed = EQ_MODE_BASS_CUT;
            break;

        case EQ_MODE_PODCAST_ID:
            configed = EQ_MODE_PODCAST;
            break;

        case EQ_MODE_OFF_ID:
            configed = EQ_MODE_OFF;
            break;

        case EQ_MODE_AUDIOPHILE_ID:
            configed = EQ_MODE_AUDIOPHILE;
            break;

        default:
            break;
    }

    if(configed)
    {
        if (appGetEqmode() != configed)
        {
            appHeadSetEqMode(configed);
        }
#if 0
        if(PsStore(PSKEY_INN_EQ_MODE_APP_CONFIG,&configed,1) != 1)
        {
            DEBUG_LOG_INFO("store Auto-off feature pskey fail");
        }
#endif
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_eq_setting, sizeof(configed), &configed);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_eq_setting, failed_back_value);
    }

    return;
}
#endif

static void gaiaHeadsetPlugin_GetApplicationVersion(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_firmware_version, failed_back_value);
        return;
    }

    if (payload[0] == 0xFF)
    {
        const char * response_payload = DeviceInfo_GetFirmwareVersion();
        uint8 response_payload_length = strlen(response_payload);

        DEBUG_LOG("gaiaCorePlugin_GetApplicationVersion, %s", response_payload);

        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_firmware_version, response_payload_length, (uint8 *)response_payload);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_firmware_version, failed_back_value);
    }
}

static void gaiaHeadsetPlugin_RestoreAppDefaultSetting(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_restore_app_default_setting, failed_back_value);
        return;
    }

    if (payload[0] == 0x00)
    {
        appGaiaRestoreDefaultSetting();
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_restore_app_default_setting, 0x00, NULL);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_restore_app_default_setting, failed_back_value);
    }
}

#ifdef ENABLE_APP_SIDETONE
static void gaiaHeadsetPlugin_GetSidetoneStatus(GAIA_TRANSPORT *t)
{
    uint8 value;

    switch(appGetSidetoneStatus())
    {
        case SIDETONE_SETTING_ENABLE:
            value = SIDETONE_SETTING_ENABLE;
            break;

        case SIDETONE_SETTING_DISABLE:
            value = SIDETONE_SETTING_DISABLE;
            break;

        default:
            value = SIDETONE_SETTING_ENABLE;
            break;
    }

    DEBUG_LOG_INFO("gaiaEarbudPlugin_GetAutoOffStatus: %d", value);

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_sidetone_setting, sizeof(value), &value);

    return;
}

static void gaiaHeadsetPlugin_SidetoneSetting(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_sidetone_setting, failed_back_value);
        return;
    }

    uint8 configed = 0;

    switch(payload[0])
    {
        case SIDETONE_GET_STATUS_ID:
            gaiaHeadsetPlugin_GetSidetoneStatus(t);
            return;

        case SIDETONE_SETTING_ENABLE:
            configed = SIDETONE_SETTING_ENABLE;
            break;

        case SIDETONE_SETTING_DISABLE:
            configed = SIDETONE_SETTING_DISABLE;
            break;

        default:
            break;
    }

    if(configed)
    {
        if (appGetSidetoneStatus() != configed)
        {
            appEnableSideTone(configed);
        }
#if 0
        if(PsStore(PSKEY_INN_SIDETONE_APP_CONFIG,&configed,1) != 1)
        {
            DEBUG_LOG_INFO("store Auto-off feature pskey fail");
        }
#endif
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_sidetone_setting, sizeof(configed), &configed);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_sidetone_setting, failed_back_value);
    }

    return;
}
#endif

static void gaiaHeadsetPlugin_FactoryReset(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_restore_app_default_setting, failed_back_value);
        return;
    }

    if (payload[0] == 0x00)
    {
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_factroy_reset, 0x00, NULL);
        appTestFactoryReset();
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_factroy_reset, failed_back_value);
    }
}

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
static void gaiaHeadsetPlugin_GetDeviceConnectedNum(GAIA_TRANSPORT *t, unsigned saved_num)
{
    uint8 number_info[3] = {0xFF,0xFF,0xFF};
    unsigned paired_num = BtDevice_GetNumberOfHandsetsConnected();
    //unsigned paired_num = BtDevice_GetConnectedBredrHandsets(&devices_arry);

    if(paired_num == 0)
    {
        number_info[1] = NO_DEVICE_CONNECTED;
    }
    else if(paired_num == 1)
    {
        number_info[1] = ONLY_PRIMARY_DEVICE;
    }
    else
    {
        number_info[1] = PRIMARY_AND_SECONDARY_DEVICE;
    }

    number_info[2] = saved_num;

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, get_connected_devices_info, sizeof(number_info), number_info);
}

static void gaiaHeadsetPlugin_GetDeviceSavedInfo(GAIA_TRANSPORT *t, unsigned app_device_index, device_t device, const bdaddr addr)
{
    uint16 name_size;
    uint8 saved_info[38] = {0x00};
    const char * name = RemoteName_Get(device, &name_size);

    saved_info[0] = 0xF2;
    saved_info[1] = app_device_index;
    saved_info[2] = ConManagerIsConnected(&addr);
    saved_info[3] = 0x06 + name_size;
    saved_info[4] = addr.nap >> 8;
    saved_info[5] = addr.nap & 0x00ff;
    saved_info[6] = addr.uap;
    saved_info[7] = (addr.lap >> 16) & 0x00ff;
    saved_info[8] = (addr.lap >> 8) & 0x0000ff;
    saved_info[9] = addr.lap & 0x000000ff;
    for(int i = 0;i < name_size;i++)
    {
        saved_info[10 + i] = name[i];
        //DEBUG_LOG_INFO("saved_info Len1 name:%c",name[i]);
    }

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, get_connected_devices_info, 10 + name_size, saved_info);

    free((void *)name);
    name = NULL;
}

static void gaiaHeadsetPlugin_GetConnectedDevicesInfo(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if (payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, get_connected_devices_info, failed_back_value);
        return;
    }

    bdaddr *bd_addr = NULL;
    unsigned saved_num;
    uint8 device_idx = 0xff;
    bdaddr device_addr = {0x00};

    if (BtDevice_GetAllHandsetBdAddr(&bd_addr, &saved_num))
    {
        DEBUG_LOG_INFO("gaiaHeadsetPlugin_GetConnectedDevicesInfo saved_num:%d",saved_num);

        switch (payload[0])
        {
            case connected_device_get_num:
            {
                gaiaHeadsetPlugin_GetDeviceConnectedNum(t, saved_num);
            }
            break;

            case connected_device_get_saved:
            {
                device_idx = payload[1] - 1;

                if ((device_idx >= DEVICE_CONNECTED_INFO_MIN_IDX) &&
                    (device_idx <= DEVICE_CONNECTED_INFO_MAX_IDX) && (payload[1] <= saved_num))
                {
                    memcpy(&device_addr, &bd_addr[device_idx], sizeof(bdaddr));
                    device_t device = BtDevice_GetDeviceForBdAddr(&device_addr);

                    gaiaHeadsetPlugin_GetDeviceSavedInfo(t, payload[1], device, device_addr);
                }
                else
                {
                    GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, get_connected_devices_info, failed_back_value);
                }
            }
            break;

            default:
                break;
        }

        free(bd_addr);
        bd_addr  = NULL;
    }
}

static void gaiaHeadsetPlugin_SetConnectedDevicesStatus(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    unsigned saved_num;
    bdaddr addr = {0x00};
    bdaddr *bd_addr = NULL;
    bool is_save_addr = FALSE;
    uint8 responsePayload[2] = {0x00};
    uint8 responsePayloadlen = sizeof(responsePayload) / sizeof(uint8);

    addr.nap = payload[2]<<8 | payload[3];
    addr.uap = payload[4];
    addr.lap = payload[5]<<16 | payload[6]<<8 | payload[7];

    if (BtDevice_GetAllHandsetBdAddr(&bd_addr, &saved_num))
    {
        for (unsigned i = 0; i < saved_num; i++)
        {
           if (memcmp(&(bd_addr[i]), &addr, sizeof(bdaddr)) == 0)
           {
               is_save_addr = TRUE;
               break;
           }
        }

        free(bd_addr);
        bd_addr  = NULL;

        if(payload_length != 8 || is_save_addr == FALSE)
        {
            GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, failed_back_value);
            return;
        }

        responsePayload[0] = payload[0];
        responsePayload[1] = payload[1];

        if ((payload[0] == connected_device_set_saved) || (payload[0] == connected_device_set_paired))
        {
            switch (payload[1])
            {
                case paired_device_disconnected:
                {
                    if(ConManagerIsConnected(&addr) == TRUE)
                    {
                        appPdlDisconnectInfo.t = t;
                        appPdlDisconnectInfo.isReceiveGaiaCmd = TRUE;
                        memcpy(&(appPdlDisconnectInfo.saveAddr), &addr, sizeof(bdaddr));
                        memcpy(appPdlDisconnectInfo.responsePayload, responsePayload, sizeof(responsePayload));

                        //GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, responsePayloadlen, responsePayload);
                        HandsetService_DisconnectRequest(&(t->task), &addr, 0);//Active disconnect all
                        DEBUG_LOG_INFO("gaiaHeadsetPlugin_SetConnectedDevicesStatus: device_disconnected");
                    }
                    else
                    {
                        GaiaFramework_MD_SendResponse_Fail(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, responsePayloadlen, responsePayload);
                    }
                }
                break;

                case saved_device_removed:
                case paired_device_removed:
                {
                    if(ConManagerIsConnected(&addr) == TRUE)
                    {
                        appPdlRemoveInfo.t = t;
                        appPdlRemoveInfo.isReceiveGaiaCmd = TRUE;
                        memcpy(&(appPdlRemoveInfo.saveAddr), &addr, sizeof(bdaddr));
                        memcpy(appPdlRemoveInfo.responsePayload, responsePayload, sizeof(responsePayload));

                        HandsetService_DisconnectRequest(&(t->task), &addr, 0);//Active disconnect all
                    }
                    else
                    {
                        if(appDeviceDelete(&addr) == TRUE)
                        {
                            GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, responsePayloadlen, responsePayload);
                            DEBUG_LOG_INFO("gaiaHeadsetPlugin_SetConnectedDevicesStatus: device remove success");
                        }
                        else
                        {
                            GaiaFramework_MD_SendResponse_Fail(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, responsePayloadlen, responsePayload);
                            DEBUG_LOG_INFO("gaiaHeadsetPlugin_SetConnectedDevicesStatus: device remove Fail");
                        }
                    }
                }
                break;

                case saved_device_connected:
                {
                    if(ConManagerIsConnected(&addr) == FALSE)
                    {
                        appPdlConnectInfo.t = t;
                        appPdlConnectInfo.isReceiveGaiaCmd = TRUE;
                        memcpy(&(appPdlConnectInfo.saveAddr), &addr, sizeof(bdaddr));
                        memcpy(appPdlConnectInfo.responsePayload, responsePayload, sizeof(responsePayload));

                        appGaiaConnectDeviceStartTimer();
                        HandsetService_ConnectAddressRequest(&(t->task),&addr,DEVICE_PROFILE_HFP|DEVICE_PROFILE_A2DP|DEVICE_PROFILE_AVRCP);//Active connect
                        DEBUG_LOG_INFO("gaiaHeadsetPlugin_SetConnectedDevicesStatus: device connected");
                    }
                    else
                    {
                        GaiaFramework_MD_SendResponse_Fail(t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, responsePayloadlen, responsePayload);
                        DEBUG_LOG_INFO("gaiaHeadsetPlugin_SetConnectedDevicesStatus: device connected fail");
                    }
                }
                break;

                default:
                break;
            }
        }
    }
}
#endif

#ifdef ENABLE_APP_MIC_MUTE
static void gaiaHeadsetPlugin_GetMicMuteControlStatus(GAIA_TRANSPORT *t)
{
    uint8 value;

    switch(appGetMicMuteControlStatus())
    {
        case MIC_MUTE_CONTROL_ENABLE:
            value = MIC_MUTE_CONTROL_ENABLE;
            break;

        case MIC_MUTE_CONTROL_DISABLE:
            value = MIC_MUTE_CONTROL_DISABLE;
            break;

        default:
            value = MIC_MUTE_CONTROL_ENABLE;
            break;
    }

    DEBUG_LOG_INFO("gaiaHeadsetPlugin_GetMicMuteControlStatus: %d", value);

    GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_mic_mute_control, sizeof(value), &value);

    return;
}

static void gaiaHeadsetPlugin_MicMuteControlStatus(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_mic_mute_control, failed_back_value);
        return;
    }

    uint8 configed = 0;

    switch(payload[0])
    {
        case MIC_MUTE_CONTROL_GET_STATUS_ID:
            gaiaHeadsetPlugin_GetMicMuteControlStatus(t);
            return;

        case MIC_MUTE_CONTROL_ENABLE:
            configed = MIC_MUTE_CONTROL_ENABLE;
            break;

        case MIC_MUTE_CONTROL_DISABLE:
            configed = MIC_MUTE_CONTROL_DISABLE;
            break;

        default:
            break;
    }

    if(configed)
    {
        if (appGetMicMuteControlStatus() != configed)
        {
            appSetMicMuteControlStatus(configed);
        }
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_mic_mute_control, sizeof(configed), &configed);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_mic_mute_control, failed_back_value);
    }

    return;
}
#endif

#ifdef ENABLE_APP_GAIA_GET_CURRENT_CONNECTED_DEVICE_ADDRESS
static void gaiaHeadsetPlugin_GaiaGetCurrentConnectedDeviceAddress(GAIA_TRANSPORT *t, uint16 payload_length, const uint8 *payload)
{
    if(payload_length == 0)
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_current_connected_device_address, failed_back_value);
        return;
    }

    bool status = FALSE;
    typed_bdaddr tpaddr = {0x00};
    uint8 tx_buff[6] = {0x00};
    uint8 len = sizeof(tx_buff) / sizeof(uint8);

    if (payload[0] == 0xff)
    {
        status = BtDevice_GetPublicAddress(&(t->tp_bd_addr.taddr), &tpaddr);
        if (status == TRUE)
        {
            tx_buff[0] = tpaddr.addr.nap >> 8;
            tx_buff[1] = tpaddr.addr.nap & 0x00ff;
            tx_buff[2] = tpaddr.addr.uap;
            tx_buff[3] = (tpaddr.addr.lap >> 16) & 0x00ff;
            tx_buff[4] = (tpaddr.addr.lap >> 8) & 0x0000ff;
            tx_buff[5] = tpaddr.addr.lap & 0x000000ff;

            //DEBUG_LOG_INFO("gaiaHeadsetPlugin_GaiaGetCurrentConnectedDeviceAddress %04x,%02x,%06lx", tpaddr.addr.nap, tpaddr.addr.uap, tpaddr.addr.lap);
        }
    }

    if(status)
    {
        GaiaFramework_MD_SendResponse(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_current_connected_device_address, len, tx_buff);
    }
    else
    {
        GaiaFramework_MD_SendError(t, GAIA_VOICE_UI_FEATURE_ID, gaia_get_current_connected_device_address, failed_back_value);
    }

    return;
}
#endif

#endif

void HeadsetGaiaPlugin_va_notification(uint8_t data)
{
    uint8 wuw_notification[3] = {0x00};
    wuw_notification[0] = 0x55;
    wuw_notification[1] = 0xAA;
    wuw_notification[2] = data;

    DEBUG_LOG_INFO("gaiaEarbudPlugin_SendWuwNotification wuw_notification[0]:%d, wuw_notification[1]:%d, wuw_notification[2]:%d",wuw_notification[0], wuw_notification[1],wuw_notification[2]);

    GaiaFramework_MD_SendNotification(GAIA_VOICE_UI_FEATURE_ID, gaia_va_test_command_id, sizeof(wuw_notification), wuw_notification);
}

static gaia_framework_command_status_t voiceUiGaiaPlugin_MainHandler(GAIA_TRANSPORT *t, uint8 pdu_id, uint16 payload_length, const uint8 *payload)
{
    DEBUG_LOG("voiceUiGaiaPlugin_MainHandler, pdu_id %u", pdu_id);

    switch (pdu_id)
    {
#if 0
    case voice_ui_gaia_get_selected_assistant:
        voiceUiGaiaPlugin_GetSelectedAssistant(t);
        break;

    case voice_ui_gaia_set_selected_assistant:
        voiceUiGaiaPlugin_SetSelectedAssistant(t, payload_length, payload);
        break;

    case voice_ui_gaia_get_supported_assistants:
        voiceUiGaiaPlugin_GetSupportedAssistants(t);
        break;
#endif

#ifdef ENABLE_APP_MD_GAIA
    case gaia_get_product_id:
        {
            gaiaHeadsetPlugin_GetProductID(t, payload_length, payload);
        }
        break;

    case gaia_auto_off_timer:
        {
            gaiaHeadsetPlugin_SetAutoOffFeature(t, payload_length, payload);
        }
        break;

    case gaia_get_battery_level:
        {
            gaiaHeadsetPlugin_GetBatteryLevel(t, payload_length, payload);
        }
        break;

    case gaia_restore_app_default_setting:
        {
            gaiaHeadsetPlugin_RestoreAppDefaultSetting(t, payload_length, payload);
        }
        break;

    case gaia_get_firmware_version:
        {
            gaiaHeadsetPlugin_GetApplicationVersion(t, payload_length, payload);
        }
        break;

#ifdef ENABLE_APP_EQ_SWITCH
    case gaia_eq_setting:
        {
            gaiaHeadsetPlugin_SetEqMode(t, payload_length, payload);
        }
        break;
#endif

    case gaia_device_name_revision:
        {
            gaiaHeadsetPlugin_DeviceNameRevision(t, payload_length, payload);
        }
        break;

#ifdef ENABLE_APP_SIDETONE
    case gaia_sidetone_setting:
        {
            gaiaHeadsetPlugin_SidetoneSetting(t, payload_length, payload);
        }
        break;
#endif

    case gaia_factroy_reset:
        {
            gaiaHeadsetPlugin_FactoryReset(t, payload_length, payload);
        }
        break;
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
    case get_connected_devices_info:
        {
            gaiaHeadsetPlugin_GetConnectedDevicesInfo(t, payload_length, payload);
        }
        break;

    case set_connected_devices_status:
        {
            gaiaHeadsetPlugin_SetConnectedDevicesStatus(t, payload_length, payload);
        }
        break;
#endif

#ifdef ENABLE_APP_MIC_MUTE
    case gaia_mic_mute_control:
        {
            gaiaHeadsetPlugin_MicMuteControlStatus(t, payload_length, payload);
        }
        break;
#endif

#ifdef ENABLE_APP_GAIA_GET_CURRENT_CONNECTED_DEVICE_ADDRESS
    case gaia_get_current_connected_device_address:
        {
            gaiaHeadsetPlugin_GaiaGetCurrentConnectedDeviceAddress(t, payload_length, payload);
        }
        break;
#endif
    case gaia_va_test_command_id:
    {
        HeadsetGaiaPlugin_va_notification(0x00);
    }
    break;
    default:
        DEBUG_LOG_ERROR("voiceUiGaiaPlugin_MainHandler, unhandled call for %u", pdu_id);
        return command_not_handled;
    }

    return command_handled;
}


static void voiceUiGaiaPlugin_TransportConnect(GAIA_TRANSPORT *t)
{
    DEBUG_LOG_INFO("voiceUiGaiaPlugin_TransportConnect, transport %p", t);
}

static void upgradeGaiaPlugin_TransportDisconnect(GAIA_TRANSPORT *t)
{
    DEBUG_LOG_INFO("voiceUiGaiaPlugin_TransportDisconnect, transport %p", t);
}

static void voiceUiGaiaPlugin_SendAllNotifications(GAIA_TRANSPORT *transport)
{
    UNUSED(transport);
    DEBUG_LOG("voiceUiGaiaPlugin_SendAllNotifications");
    VoiceUiGaiaPlugin_NotifyAssistantChanged(VoiceUi_GetSelectedAssistant());
}


void VoiceUiGaiaPlugin_Init(void)
{
    static const gaia_framework_plugin_functions_t functions =
    {
        .command_handler = voiceUiGaiaPlugin_MainHandler,
        .send_all_notifications = voiceUiGaiaPlugin_SendAllNotifications,
        .transport_connect = voiceUiGaiaPlugin_TransportConnect,
        .transport_disconnect = upgradeGaiaPlugin_TransportDisconnect,
    };

    GaiaFramework_RegisterFeature(GAIA_VOICE_UI_FEATURE_ID, GAIA_VOICE_UI_VERSION, &functions);
}


void VoiceUiGaiaPlugin_NotifyAssistantChanged(voice_ui_provider_t va_provider)
{
    uint8 provider_id = va_provider;
    GaiaFramework_SendNotification(GAIA_VOICE_UI_FEATURE_ID, voice_ui_gaia_assistant_changed, 1, &provider_id);
}

#ifdef ENABLE_APP_MD_GAIA
void appUpdateGaiaBatterylevel(uint8 val)
{
    uint8 batteryPrecent = val;
    GaiaFramework_MD_SendDataNotification(GAIA_VOICE_UI_FEATURE_ID, gaia_get_battery_level, sizeof(batteryPrecent), &batteryPrecent);
}

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
void appGaiaDeleteDeviceHandle(bdaddr addr)
{
    DEBUG_LOG_INFO("appGaiaDeleteDeviceHandle %04x,%02x,%06lx", addr.nap, addr.uap, addr.lap);
    DEBUG_LOG_INFO("appGaiaDeleteDeviceHandle isConnected: %d", ConManagerIsConnected(&(appPdlRemoveInfo.saveAddr)));

    if (memcmp(&(appPdlRemoveInfo.saveAddr), &addr, sizeof(bdaddr)) == 0)
    {
        if (appPdlRemoveInfo.isReceiveGaiaCmd == TRUE)
        {
            GaiaFramework_MD_SendResponse(appPdlRemoveInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlRemoveInfo.responsePayload), appPdlRemoveInfo.responsePayload);
            DEBUG_LOG_INFO("appGaiaDeleteDeviceHandle: device remove Success");

            tp_bdaddr tpa_ddr = {0x00};
            tpa_ddr.transport = TRANSPORT_BLE_ACL;
            tpa_ddr.taddr.type = TYPED_BDADDR_PUBLIC;
            tpa_ddr.taddr.addr = appPdlRemoveInfo.saveAddr;
            HandsetService_DisconnectTpAddrRequest(&(appPdlRemoveInfo.t->task), &tpa_ddr, 0);

            if(appDeviceDelete(&(appPdlRemoveInfo.saveAddr)) == FALSE)
            {
                GaiaFramework_MD_SendResponse_Fail(appPdlRemoveInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlRemoveInfo.responsePayload), appPdlRemoveInfo.responsePayload);
                DEBUG_LOG_INFO("appGaiaDeleteDeviceHandle: device remove Fail");
            }

            memset(&appPdlRemoveInfo, 0x00, sizeof(appPdlRemoveInfo));
        }
    }
}

void appGaiaConnectDeviceHandle(bdaddr addr)
{
    DEBUG_LOG_INFO("appGaiaConnectDeviceHandle %04x,%02x,%06lx", addr.nap, addr.uap, addr.lap);
    DEBUG_LOG_INFO("appGaiaConnectDeviceHandle isConnected: %d", ConManagerIsConnected(&(appPdlConnectInfo.saveAddr)));

    if (memcmp(&(appPdlConnectInfo.saveAddr), &addr, sizeof(bdaddr)) == 0)
    {
        if (appPdlConnectInfo.isReceiveGaiaCmd == TRUE)
        {
            if(ConManagerIsConnected(&(appPdlConnectInfo.saveAddr)) == TRUE)
            {
                GaiaFramework_MD_SendResponse(appPdlConnectInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlConnectInfo.responsePayload), appPdlConnectInfo.responsePayload);
                DEBUG_LOG_INFO("appGaiaConnectDeviceHandle: device connect Success");
            }

            memset(&appPdlConnectInfo, 0x00, sizeof(appPdlConnectInfo));
            appGaiaConnectDeviceCancelTimer();
        }
    }
}

void appGaiaConnectDeviceFailHandle(void)
{
    if (appPdlConnectInfo.isReceiveGaiaCmd == TRUE)
    {
        if(ConManagerIsConnected(&(appPdlConnectInfo.saveAddr)) == FALSE)
        {
            GaiaFramework_MD_SendResponse_Fail(appPdlConnectInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlConnectInfo.responsePayload), appPdlConnectInfo.responsePayload);
            DEBUG_LOG_INFO("appGaiaConnectDeviceFailHandle: device connect fail");
        }

        memset(&appPdlConnectInfo, 0x00, sizeof(appPdlConnectInfo));
    }
}

void appGaiaDisconnectDeviceHandle(bdaddr addr)
{
    DEBUG_LOG_INFO("appGaiaDisconnectDeviceHandle %04x,%02x,%06lx", addr.nap, addr.uap, addr.lap);
    DEBUG_LOG_INFO("appGaiaDisconnectDeviceHandle isConnected: %d", ConManagerIsConnected(&(appPdlDisconnectInfo.saveAddr)));

    if (memcmp(&(appPdlDisconnectInfo.saveAddr), &addr, sizeof(bdaddr)) == 0)
    {
        if (appPdlDisconnectInfo.isReceiveGaiaCmd == TRUE)
        {
            if(ConManagerIsConnected(&(appPdlDisconnectInfo.saveAddr)) == FALSE)
            {
                GaiaFramework_MD_SendResponse(appPdlDisconnectInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlDisconnectInfo.responsePayload), appPdlDisconnectInfo.responsePayload);
                DEBUG_LOG_INFO("appGaiaDisconnectDeviceHandle: device disconnect Success");

                tp_bdaddr tpa_ddr = {0x00};
                tpa_ddr.transport = TRANSPORT_BLE_ACL;
                tpa_ddr.taddr.type = TYPED_BDADDR_PUBLIC;
                tpa_ddr.taddr.addr = appPdlDisconnectInfo.saveAddr;
                HandsetService_DisconnectTpAddrRequest(&(appPdlDisconnectInfo.t->task), &tpa_ddr, 0);
            }
            else
            {
                GaiaFramework_MD_SendResponse_Fail(appPdlDisconnectInfo.t, GAIA_VOICE_UI_FEATURE_ID, set_connected_devices_status, sizeof(appPdlDisconnectInfo.responsePayload), appPdlDisconnectInfo.responsePayload);
                DEBUG_LOG_INFO("appGaiaDisconnectDeviceHandle: device disconnect fail");
            }

            memset(&appPdlDisconnectInfo, 0x00, sizeof(appPdlDisconnectInfo));
        }
    }
}
#endif
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
void appNotifyDeviceStatus(bdaddr addr, bool state, uint8 profile)
{
    DEBUG_LOG_INFO("appNotifyDeviceStatus: state %d, isconnected: %d", state, ConManagerIsConnected(&addr));

    if (profile == DEVICE_PROFILE_HFP)
    {
        uint8 tx_buff[7] = {0x00};
        uint8 len = sizeof(tx_buff) / sizeof (uint8);

        tx_buff[0] = state;
        tx_buff[1] = addr.nap >> 8;
        tx_buff[2] = addr.nap & 0x00ff;
        tx_buff[3] = addr.uap;
        tx_buff[4] = (addr.lap >> 16) & 0x00ff;
        tx_buff[5] = (addr.lap >> 8) & 0x0000ff;
        tx_buff[6] = addr.lap & 0x000000ff;

        GaiaFramework_MD_SendNotification(GAIA_VOICE_UI_FEATURE_ID, gaia_notify_device_connected_status, len, tx_buff);
    }
}
#endif
#endif

#endif /* INCLUDE_GAIA */

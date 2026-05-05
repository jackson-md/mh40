/*!
\copyright  Copyright (c) 2020 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file
\brief      GAIA interface with Voice Assistants.
*/

#ifndef _VOICE_UI_GAIA_
#define _VOICE_UI_GAIA_

#ifdef INCLUDE_GAIA

#include "voice_ui_container.h"
#include "gaia.h"

/*! \brief Voice UI GAIA component version
*/
#define GAIA_VOICE_UI_VERSION (1)

/*! \brief Commands handled by the Voice UI GAIA Component
*/
typedef enum
{
    /*! Get the currently selected Voice Assistant ID */
    voice_ui_gaia_get_selected_assistant = 0,
    /*! Select a Voice Assistant to use */
    voice_ui_gaia_set_selected_assistant = 1,
    /*! Get vector of supported assistant IDs */
    voice_ui_gaia_get_supported_assistants = 2,

#ifdef ENABLE_APP_MD_GAIA
     /*! Get product ID and Activate Connections */
    gaia_get_product_id = 0x10,
    /*! Active Connection */
    gaia_auto_off_timer = 0x12,
    /*! Battery level */
    gaia_get_battery_level = 0x14,
    /*! restore app default setting */
    gaia_restore_app_default_setting = 0x15,
    /*! Get firmware_version */
    gaia_get_firmware_version = 0x16,
    /*! change EQ mode */
    gaia_eq_setting = 0x17,
    /*! change device name */
    gaia_device_name_revision = 0x19,
    /*! sidetone set */
    gaia_sidetone_setting = 0x1c,
    /*! sidetone set */
    gaia_factroy_reset = 0x1d,

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
    /*! Get Connected Devices info about Number,Paired and Saved */
    get_connected_devices_info = 0x1E,
    /*! Connected Devices Set Status */
    set_connected_devices_status = 0x1F,

#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
    gaia_notify_device_connected_status = 0xF2,
#endif

#endif

#ifdef ENABLE_APP_MIC_MUTE
    /*! mic mute control state */
    gaia_mic_mute_control = 0x20,
#endif

#ifdef ENABLE_APP_GAIA_GET_CURRENT_CONNECTED_DEVICE_ADDRESS
    gaia_get_current_connected_device_address = 0x21,
#endif
    gaia_va_test_command_id = 0x22,

#endif
} voice_ui_gaia_command_t;

#ifdef ENABLE_APP_MD_GAIA
#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
typedef enum
{
    connected_device_get_num = 0xFF,
    connected_device_get_paired = 0xF1,
    connected_device_get_saved = 0xF2,
} headset_plugin_connected_devices_get_types_t;

typedef enum
{
    connected_device_set_paired = 0xF1,
    connected_device_set_saved = 0xF2,
} headset_plugin_connected_devices_set_types_t;

typedef enum
{
    paired_device_disconnected = 0x00,
    paired_device_removed = 0x01,
    saved_device_connected = 0x02,
    saved_device_removed = 0x03,
} headset_plugin_devices_set_status_t;

#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
typedef enum
{
    gaia_notify_device_status_disconnected = 0x00,
    gaia_notify_device_status_connected = 0x01,
} headset_plugin_gaia_notify_device_status_t;
#endif

#endif

#endif
typedef enum
{
    voice_ui_gaia_assistant_changed = 0
} voice_ui_gaia_notification_t;

/*! \brief Initialise the Voice UI GAIA component.
*/
void VoiceUiGaiaPlugin_Init(void);

/*! \brief Notify the GAIA host that the VA provider has changed.w
 *  \param va_provider Identifies the new VA provider.
*/
void VoiceUiGaiaPlugin_NotifyAssistantChanged(voice_ui_provider_t va_provider);

#ifdef ENABLE_APP_MD_GAIA
extern void appUpdateGaiaBatterylevel(uint8 val);

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
extern void appGaiaDeleteDeviceHandle(bdaddr addr);
extern void appGaiaConnectDeviceHandle(bdaddr addr);
extern void appGaiaConnectDeviceFailHandle(void);
extern void appGaiaDisconnectDeviceHandle(bdaddr addr);
void HeadsetGaiaPlugin_va_notification(uint8_t data);

#ifdef ENABLE_APP_MD_GAIA_NOTIFY_DEVICE_STATUS
extern void appNotifyDeviceStatus(bdaddr addr, bool state, uint8 profile);
#endif

#endif

#endif

#endif /* INCLUDE_GAIA */
#endif /* _VOICE_UI_GAIA_ */

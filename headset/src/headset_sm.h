/*!
\copyright  Copyright (c) 2019 - 2021 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    %%version
\file       headset_sm.h
\brief      headset application State machine.
*/

#ifndef HEADSET_SM_H_
#define HEADSET_SM_H_

#include "headset_sm_config.h"

/*!
@startuml

    note "Headset Application States" as N1
   
    [*] -down-> HEADSET_STATE_NULL: HS State Init
    HEADSET_STATE_NULL -down->HEADSET_STATE_LIMBO :SSM transition to system_state_limbo
    HEADSET_STATE_NULL: Initial state
    state Headsetisactive {
        HEADSET_STATE_LIMBO->HEADSET_STATE_LIMBO: Start Limbo Timer
        HEADSET_STATE_LIMBO: Stable state after Startup
        HEADSET_STATE_LIMBO->HEADSET_STATE_POWERING_ON:User Initiated Power On/SSM transition to system_state_powering_on
        HEADSET_STATE_POWERING_ON-down->HEADSET_STATE_IDLE: If already a device in PDL
        HEADSET_STATE_IDLE:Can have BT connection or not
        HEADSET_STATE_POWERING_ON -down-> HEADSET_STATE_PAIRING: If no device in PDL
        HEADSET_STATE_POWERING_ON : A transition state for powering On
        HEADSET_STATE_PAIRING -> HEADSET_STATE_IDLE: If pairing successful or failed
        HEADSET_STATE_IDLE-down->HEADSET_STATE_BUSY:Start Music Streaming/Call(BT)
        HEADSET_STATE_IDLE-left->HEADSET_STATE_IDLE:Start Idle timer in case of no activity(eg:No BT)
        HEADSET_STATE_IDLE->HEADSET_STATE_PAIRING:User Initiated Pairing
    }
    HEADSET_STATE_IDLE-up->HEADSET_STATE_POWERING_OFF:Idle Timer Expires and Ok to power off(eg:No BT)
    HEADSET_STATE_BUSY->HEADSET_STATE_IDLE:End Music Streaming/Call
    Headsetisactive -up->HEADSET_STATE_POWERING_OFF: User Initiated Power OFF
    Headsetisactive -up->HEADSET_STATE_TERMINATING : Emergency Shutdown
    HEADSET_STATE_TERMINATING: State to handle Emergency Shutdown
    HEADSET_STATE_TERMINATING->PowerdOff
    HEADSET_STATE_POWERING_OFF ->HEADSET_STATE_POWERING_OFF: Disconnect Link if needed
    HEADSET_STATE_POWERING_OFF: State to handle normal PowerOFF
    HEADSET_STATE_POWERING_OFF ->HEADSET_STATE_LIMBO 
    HEADSET_STATE_LIMBO -left->PowerdOff:No ChargerConnected/Limbo Timer Expires 
    state PowerdOff #LightBlue
    PowerdOff: This state is realized in hardware, either Dormant or Off
    note left of HEADSET_STATE_LIMBO
        A Limbo timer and Charger 
        determines the transition
        to PowerdOff
        If Charger is connected
        Headset will be in Limbo
        Else after Limbo timer 
        expires state transition
        to PowerdOff
    end note
    state HEADSET_FACTORY_RESET:Reset the system, Can be entered after Power On
    HEADSET_FACTORY_RESET-left->[*]
@enduml
*/

/*! \brief Headset Application states.
 */
typedef enum sm_headset_states
{
    /*!< Initial state before state machine is running. */
    HEADSET_STATE_NULL                = 0x0000,
    HEADSET_STATE_FACTORY_RESET,
    HEADSET_STATE_LIMBO,
    HEADSET_STATE_POWERING_ON,
    HEADSET_STATE_PAIRING,
    HEADSET_STATE_IDLE,
    HEADSET_STATE_BUSY,
    HEADSET_STATE_TERMINATING,
    HEADSET_STATE_POWERING_OFF
} headsetState;

/*! \brief SM UI Provider contexts */
typedef enum
{
    context_app_sm_inactive,
    context_app_sm_active,
    context_app_sm_powered_off,
    context_app_sm_powered_on,
    context_app_sm_idle,
    context_app_sm_idle_connected,
    context_app_sm_exit_idle,

} sm_provider_context_t;

/*! \brief Main application state machine task data. */
typedef struct
{
    TaskData task;                      /*!< SM task */
    headsetState state;                 /*!< Application state */
    uint16 disconnect_lock;             /*!< Disconnect message lock */
    bool user_pairing:1;                /*!< User initiated pairing */
    bool auto_poweron:1;                /*!< Auto power on flag */
} smTaskData;

#ifdef ENABLE_APP_MD_GAIA
typedef enum
{
    AUTO_OFF_TIMEOUT_NEVER_ID 		= 0x01,
    AUTO_OFF_TIMEOUT_30MIN_ID 		= 0x03,
    AUTO_OFF_TIMEOUT_1HOUR_ID 		= 0x04,
    AUTO_OFF_TIMEOUT_3HOUR_ID 		= 0x05,
    AUTO_OFF_TIMEOUT_30MIN    		= 0x1E,
    AUTO_OFF_TIMEOUT_1HOUR    		= 0x3C,
    AUTO_OFF_TIMEOUT_3HOUR    		= 0xB4,
    AUTO_OFF_TIMEOUT_NEVER    		= 0xFF,
    AUTO_OFF_GET_TIMEOUT_TIMER_ID  	= 0xFF,
}AUTO_OFF_SETTING_PARAMETER;

typedef enum
{
    EQ_MODE_MIN             = 0x00,
    EQ_MODE_BASS_BOOST 		= 0x01,
    EQ_MODE_BASS_CUT 		= 0x02,
    EQ_MODE_PODCAST 		= 0x03,
    EQ_MODE_OFF             = 0x04,
    EQ_MODE_AUDIOPHILE      = 0x05,
    EQ_MODE_MAX             = 0x06,
    EQ_MODE_BASS_BOOST_ID   = 0x01,
    EQ_MODE_BASS_CUT_ID 	= 0x02,
    EQ_MODE_PODCAST_ID 		= 0x03,
    EQ_MODE_OFF_ID          = 0x04,
    EQ_MODE_AUDIOPHILE_ID   = 0x05,
    EQ_MODE_GET_STATUS_ID   = 0xFF,
}EQ_MODE_SETTING;

typedef enum
{
    EQ_OFF_BANK_ID             = 0x00,
    EQ_BASS_BOOST_BANK_ID      = 0X01,
    EQ_BASS_CUT_BANK_ID        = 0X02,
    EQ_PODCAST_BANK_ID         = 0X03,
    EQ_AUDIOPHILE_BANK_ID      = 0X04,
}EQ_BANK_ID;

typedef enum
{
    SIDETONE_SETTING_MIN            = 0x00,
    SIDETONE_SETTING_DISABLE 		= 0x01,
    SIDETONE_SETTING_ENABLE 		= 0x02,
    SIDETONE_SETTING_MAX            = 0x03,
    SIDETONE_GET_STATUS_ID          = 0xFF,
}SIDETONE_SETTING;

#ifdef ENABLE_APP_MIC_MUTE
typedef enum
{
    MIC_MUTE_CONTROL_MIN               = 0x00,
    MIC_MUTE_CONTROL_DISABLE           = 0x01,
    MIC_MUTE_CONTROL_ENABLE            = 0x02,
    MIC_MUTE_CONTROL_MAX               = 0x03,
    MIC_MUTE_CONTROL_GET_STATUS_ID     = 0xFF,
}MIC_MUTE_CONTROL_SETTING;
#endif

#endif

/*!< Application state machine. */
extern smTaskData headset_sm;

/*! Get pointer to application state machine task data struture. */
#define SmGetTaskData()          (&headset_sm)

/*! \brief Initialise the main application state machine.
 */
bool headsetSmInit(Task init_task);
/*! \brief provides state manager(sm) task to other components

    \param[in]  void

    \return     Task - headset sm task.
*/
Task headsetSmGetTask(void);

/*! \brief Application state machine message handler.
    \param task The SM task.
    \param id The message ID to handle.
    \param message The message content (if any).
*/
void headsetSmHandleMessage(Task task, MessageId id, Message message);

/*! \brief Method to handle wired audio device arrival.*/
void headsetSmWiredAudioConnected(void);

/*! \brief Method to handle wired audio device removal.*/
void headsetSmWiredAudioDisconnected(void);

/*! \brief Initiate disconnect of handset link */
bool headsetSmDisconnectLink(void);

/*! \brief Method to handle topology stop confirmation. */
bool headetSmHandleTopologyStopCfm(Message message);

/*20221010 Add by Ou --- start ---*/
void appHeadsetSetState(headsetState new_state);
extern headsetState appHeadsetGetState(void);
extern void appHeadsetSMStartIdleTimer(void);
extern void appHeadsetSMStopIdleTimer(void);
extern void appHeadsetSmPowerOff(void);
/*20221010 Add by Ou ---  end  ---*/

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
extern void appCharingComplteteHandle(void);
#endif

#ifdef ENABLE_APP_MD_GAIA
extern void appSmReboot_Delay(void);
extern void set_appConfigIdleTimeoutMs_HandsetConected(uint8 timer);
extern uint32 appConfigIdleTimeoutMs_HandsetConected(void);
extern uint8 appGetEqmode(void);
extern bool appHeadSetEqMode(uint8 mode);
extern void appEnableSideTone(uint8 val);
extern uint8 appGetSidetoneStatus(void);
extern void appGaiaRestoreDefaultSetting(void);
extern void appRestoreDefaultSetting(void);
extern void appPoweroffStorePskeyData(void);
extern void ChangeLocalName(void *param);
extern void appRestoreDeviceName(void);
#endif

#ifdef ENABLE_APP_HID_COMMAND
extern void appHeadsetSmHandlePowerOn(void);
#endif

#ifdef ENABLE_APP_MIC_MUTE
extern void appSetMicMuteFlag(bool val);
extern uint8 appGetMicMuteControlStatus(void);
extern void appSetMicMuteControlStatus(uint8 status);
#endif

#ifdef ENABLE_APP_POWEROFF_DISPLAY_DISCONNECTED_PROMPT
extern void appSetEnablePlayDisconnectPromptFlag(bool val);
extern bool appGetEnablePlayDisconnectPromptFlag(void);
#endif

#ifdef ENABLE_APP_MD_GAIA_GET_PDL_INFO
extern void appGaiaConnectDeviceCancelTimer(void);
extern void appGaiaConnectDeviceStartTimer(void);
#endif

#endif /* HEADSET_SM_H_ */


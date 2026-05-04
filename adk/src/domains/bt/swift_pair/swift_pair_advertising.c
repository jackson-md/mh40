/*!
\copyright  Copyright (c) 2022 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file
\brief
*/

#include "swift_pair_advertising.h"

#include "le_advertising_manager.h"
#include "local_addr.h"
#include <logging.h>

#ifndef ENABLE_APP_SWIFT_PAIR_DISCONNECTED_BLE
#define SWIFT_PAIR_ADV_LENGTH 10
#define SWIFT_PAIR_MICROSOFT_VENDOR_ID 0x0006
#define SWIFT_PAIR_MICROSOFT_BEACON_ID 0x03
#define SWIFT_PAIR_MICROSOFT_SUB_SCENARIO_ID 0X02
#define SWIFT_PAIR_RESERVED_RSSI_BYTE 0x80
#else
#include "bt_device.h"
#define SWIFT_PAIR_ADV_LENGTH 16
#define SWIFT_PAIR_MICROSOFT_VENDOR_ID 0x0006
#define SWIFT_PAIR_MICROSOFT_BEACON_ID 0x03
#define SWIFT_PAIR_MICROSOFT_SUB_SCENARIO_ID 0X01
#define SWIFT_PAIR_RESERVED_RSSI_BYTE 0x80
#endif

/* CoD for an audio sink device is set to 0x240404 (3 bytes) considering the appropriate value for Major Service Class(Bit 18 & 21 set),
   Major Device Class(Bit 10 set) and Minor Device Class(Bit 2 set) according to Bluetooth specification. Bit description is provided below.

   Major Service Class
    CoD Bit 21 ---> Audio (Example: Speaker, Microphone, Headset service, ...)
    CoD Bit 18 ---> Rendering (Example: Printing, Speaker, ...)
   Major Device Class
    CoD Bit 10 ---> Audio/Video (Example: headset,speaker,stereo, video display, vcr, ...)
   Minor Device Class
    CoD Bit 2  ---> Wearable Headset Device */
#define SWIFT_PAIR_CLASS_OF_DEVICE 0x240404

#ifndef ENABLE_APP_SWIFT_PAIR_DISCONNECTED_BLE
static const uint8 sp_payload[SWIFT_PAIR_ADV_LENGTH] =
{
    SWIFT_PAIR_ADV_LENGTH - 1,
    ble_ad_type_manufacturer_specific_data,
    SWIFT_PAIR_MICROSOFT_VENDOR_ID & 0xFF,
    (SWIFT_PAIR_MICROSOFT_VENDOR_ID >> 8) & 0xFF,
    SWIFT_PAIR_MICROSOFT_BEACON_ID,
    SWIFT_PAIR_MICROSOFT_SUB_SCENARIO_ID,
    SWIFT_PAIR_RESERVED_RSSI_BYTE,
    SWIFT_PAIR_CLASS_OF_DEVICE & 0xFF,
    (SWIFT_PAIR_CLASS_OF_DEVICE >> 8) & 0xFF,
    (SWIFT_PAIR_CLASS_OF_DEVICE >> 16) & 0xFF,
};
#else
static uint8 sp_payload[SWIFT_PAIR_ADV_LENGTH] = {0};
#endif

static const le_adv_item_data_t sp_advert_payload =
{
    SWIFT_PAIR_ADV_LENGTH,
    sp_payload
};

static le_adv_mgr_register_handle swift_pair_registered_handle = NULL;
static bool IsInPairingMode = FALSE;

#ifndef ENABLE_APP_SWIFT_PAIR_DISCONNECTED_BLE
static bool swiftPair_GetData(le_adv_item_data_t * item)
{
    *item = sp_advert_payload;
    return TRUE;
}
#else
static bool swiftPair_GetData(le_adv_item_data_t * item)
{
    bdaddr bd_addr = {0};
    bool val = appDeviceGetMyBdAddr(&bd_addr);

    sp_payload[0] =SWIFT_PAIR_ADV_LENGTH - 1;
    sp_payload[1] =ble_ad_type_manufacturer_specific_data;
    sp_payload[2] =SWIFT_PAIR_MICROSOFT_VENDOR_ID & 0xFF;
    sp_payload[3] =(SWIFT_PAIR_MICROSOFT_VENDOR_ID >> 8) & 0xFF;
    sp_payload[4] =SWIFT_PAIR_MICROSOFT_BEACON_ID;
    sp_payload[5] =SWIFT_PAIR_MICROSOFT_SUB_SCENARIO_ID;
    sp_payload[6] =SWIFT_PAIR_RESERVED_RSSI_BYTE;

    if (val == TRUE)
    {
        sp_payload[12] = (bd_addr.nap >> 8) & 0xff;
        sp_payload[11] = (bd_addr.nap) & 0xff;
        sp_payload[10] = (bd_addr.uap) & 0xff;
        sp_payload[9] = (bd_addr.lap >> 16) &0xff;
        sp_payload[8] = (bd_addr.lap >> 8) &0xff;
        sp_payload[7] = (bd_addr.lap) & 0xff;
    }

    sp_payload[13] =SWIFT_PAIR_CLASS_OF_DEVICE & 0xFF;
    sp_payload[14] =(SWIFT_PAIR_CLASS_OF_DEVICE >> 8) & 0xFF;
    sp_payload[15] =(SWIFT_PAIR_CLASS_OF_DEVICE >> 16) & 0xFF;

    *item = sp_advert_payload;
    return TRUE;
}
#endif
#ifdef LE_ADVERTISING_MANAGER_NEW_API

static inline bool swiftPair_CanAdvertise(void)
{
    return (IsInPairingMode || LocalAddr_IsPublic());
}

static bool swiftPair_GetItemData(le_adv_item_data_t * item)
{
    PanicNull(item);
    bool can_advertise = FALSE;
    item->data = NULL;
    item->size = 0;
    if(swiftPair_CanAdvertise())
    {
        can_advertise = swiftPair_GetData(item);
    }
    return can_advertise;
}

static unsigned swiftPair_GetItemDataSize(void)
{
    unsigned data_size = 0;
    if(swiftPair_CanAdvertise())
    {
        data_size = sp_advert_payload.size;
    }
    return data_size;
}

static bool swiftPair_GetItemInfo(le_adv_item_info_t * info)
{
    PanicNull(info);
    *info = (le_adv_item_info_t){ .placement = le_adv_item_data_placement_advert,
                                        .type = le_adv_type_legacy_connectable_scannable,
                                        .data_size = swiftPair_GetItemDataSize() };
    return TRUE;
}

le_adv_item_callback_t swiftPair_AdvertisingManagerCallback =
{
    .GetItemData = &swiftPair_GetItemData,
    .GetItemInfo = &swiftPair_GetItemInfo
};

#else

#define SWIFT_PAIR_ADV_PARAMS_REQUESTED(params) ((params->completeness == le_adv_data_completeness_full) && \
            (params->placement == le_adv_data_placement_advert) && (params->data_set == le_adv_data_set_handset_identifiable))

#define SWIFT_PAIR_ADV_PAYLOAD 0
#define SWIFT_PAIR_ADV_ITEMS 1

static bool swiftPair_CanAdvertiseWithParams(const le_adv_data_params_t * params)
{
    bool can_advertise = FALSE;

    if ((params->data_set != le_adv_data_set_peer)
            && (IsInPairingMode && SWIFT_PAIR_ADV_PARAMS_REQUESTED(params)))
    {
        can_advertise = TRUE;
    }
    return can_advertise;
}

static unsigned int swiftPair_GetNumberOfAdvertisingItems(const le_adv_data_params_t * params)
{
    unsigned int number = 0;

    if(swiftPair_CanAdvertiseWithParams(params))
    {
        number = SWIFT_PAIR_ADV_ITEMS;
    }

    return number;
}


static le_adv_data_item_t swiftPair_GetAdvertisingData(const le_adv_data_params_t * params, unsigned int id)
{
    le_adv_data_item_t data_item = { .size = 0, .data = NULL };

    if (swiftPair_CanAdvertiseWithParams(params))
    {
        if(id == SWIFT_PAIR_ADV_PAYLOAD)
        {
            DEBUG_LOG("swiftPair_GetAdvertisingData: swift pair advert payload advertise ");
            swiftPair_GetData(&data_item);
            return data_item;
        }
        else
        {
            DEBUG_LOG("swiftPair_AdvGetDataItem: Not in pairing mode or Invalid data_set_identifier %d \n", id);
        }
    }

    return data_item;
}


static void swiftPair_ReleaseAdvertisingData(const le_adv_data_params_t * params)
{
    UNUSED(params);
    return;
}


static const le_adv_data_callback_t swiftPair_AdvertisingManagerCallback =
{
    .GetNumberOfItems = &swiftPair_GetNumberOfAdvertisingItems,
    .GetItem = &swiftPair_GetAdvertisingData,
    .ReleaseItems = &swiftPair_ReleaseAdvertisingData
};

#endif

bool SwiftPair_SetupLeAdvertisingData(void)
{
#ifdef LE_ADVERTISING_MANAGER_NEW_API
    swift_pair_registered_handle = LeAdvertisingManager_RegisterAdvertisingItemCallback(NULL, &swiftPair_AdvertisingManagerCallback);
#else
    swift_pair_registered_handle = LeAdvertisingManager_Register(NULL, &swiftPair_AdvertisingManagerCallback);
#endif
    return (swift_pair_registered_handle ? TRUE : FALSE);
}

bool SwiftPair_UpdateLeAdvertisingData(bool discoverable)
{
    IsInPairingMode = discoverable;
#ifdef LE_ADVERTISING_MANAGER_NEW_API
    return (swift_pair_registered_handle ? LeAdvertisingManager_UpdateAdvertisingItem(swift_pair_registered_handle) : FALSE);
#else
    return (swift_pair_registered_handle ? LeAdvertisingManager_NotifyDataChange(NULL, swift_pair_registered_handle) : FALSE);
#endif
}

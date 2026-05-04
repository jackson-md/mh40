/* Copyright (c) 2021 Qualcomm Technologies International, Ltd. */
/*   %%version */
/****************************************************************************
FILE
    link_status_if.h

CONTAINS
    Definitions for types used by link status messages to system task.

DESCRIPTION
    This file is seen by the stack/CDA devices, and customer applications, and
    contains things that are common between them.
*/

/*!
 @file link_status_if.h
 @brief Types used by the link status messages to system task.

 See #MessageLinkStatusTracking() for documentation on receiving
 messages on link status events.
*/
#ifndef __APP_LINK_STATUS_IF_H__
#define __APP_LINK_STATUS_IF_H__

typedef enum
{
    LINK_STATUS_LINK_QUALITY_INVALID,
    LINK_STATUS_LINK_QUALITY_LOW,
    LINK_STATUS_LINK_QUALITY_MEDIUM,
    LINK_STATUS_LINK_QUALITY_STANDARD,
    LINK_STATUS_LINK_QUALITY_HIGH,
    LINK_STATUS_LINK_QUALITY_ULTRA_HIGH
}link_status_link_quality;

typedef enum
{
    LINK_STATUS_CONNECTION_TYPE_SCO,
    LINK_STATUS_CONNECTION_TYPE_ACL,
    LINK_STATUS_CONNECTION_TYPE_ESCO,
    LINK_STATUS_CONNECTION_TYPE_BLE,
    LINK_STATUS_CONNECTION_TYPE_ANT,
    LINK_STATUS_CONNECTION_TYPE_CIS,
    LINK_STATUS_CONNECTION_TYPE_BIS
}link_status_connection_type;

typedef enum
{
    LINK_STATUS_PHY_RATE_1MBPS = 1,
    LINK_STATUS_PHY_RATE_2MBPS = 2,
    LINK_STATUS_PHY_RATE_3MBPS = 3
}link_status_phy_rate;
#endif /*__APP_LINK_STATUS_IF_H__*/

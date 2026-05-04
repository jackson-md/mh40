/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   %%version */
/****************************************************************************
FILE
    psu_if.h

CONTAINS
    Definitions for the psu subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __PSU_IF_H__
#define __PSU_IF_H__

/*! @brief PSU regulator identifiers. */
typedef enum
{
    PSU_BYP_LDO,      /*!< Linear regulator */
    PSU_ANALOG_SMPS,  /*!< Switch mode regulator powering the analog */
    PSU_DIGITAL_SMPS, /*!< Switch mode regulator powering the digits */
    PSU_ALL_SMPS,     /*!< Switch mode regulators powering the analog and digits */
    PSU_ALL,
    NUM_OF_PSU
}psu_id;

/*! @brief PSU regulator configuration keys. */
typedef enum
{
    PSU_ENABLE = 0,               /*!< enable power regulator. */
    PSU_SMPS_INPUT_SEL_VBAT = 1,  /*!< force SMPS source = VBAT */
    PSU_VOLTAGE_SENSE_REMOTE = 3, /*!< force psu to use remote voltage sensing. */
    /** \brief Keep the Bypass LDO in HP mode in deep-sleep.
     *
     * Some devices support the feature to change this setting at run-time. On
     * devices that do not support the feature, changing this setting will have
     * no effect.
     */
    PSU_BYP_LDO_DEEP_SLEEP_FORCE_HP = 4
} psu_config_key;

#endif /* __PSU_IF_H__  */


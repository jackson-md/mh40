/*!
\copyright  Copyright (c) 2019-2021 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file       headset_led.c
\brief      Source file for the Headset Application user interface LED indications.
*/
#include "headset_led.h"
#include "headset_ui.h"
#include "led_manager_config.h"

const led_manager_hw_config_t headset_led_config =
#if defined(HAVE_1_LED)
{
    .number_of_leds = 1,
    .leds_use_pio = TRUE,
    .led0_pio = CHIP_LED_0_PIO,
    .led1_pio = 0,
    .led2_pio = 0,
};
#elif defined(HAVE_3_LEDS)
{
#ifdef ENABLE_APP_ADD_LED3
    .number_of_leds = 4,
#else
    .number_of_leds = 3,
#endif

#ifdef ENABLE_APP_BREATHING_PAIRING_LED
    .leds_use_pio = FALSE,
#else
    .leds_use_pio = TRUE,
#endif
    .led0_pio = CHIP_LED_0_PIO,
    .led1_pio = CHIP_LED_1_PIO,
    .led2_pio = CHIP_LED_2_PIO,

#ifdef ENABLE_APP_ADD_LED3
    .led3_pio = CHIP_LED_3_PIO,
#endif

};
#else
#error LED config not correctly defined.
#endif
/*!@{ \name Definition of LEDs, and basic colour combinations

    The basic handling for LEDs is similar, whether there are
    3 separate LEDs, a tri-color LED, or just a single LED.
 */
#if defined(HAVE_3_LEDS)
#define LED_0_STATE  (1 << 0)
#define LED_1_STATE  (1 << 1)
#define LED_2_STATE  (1 << 2)

#ifdef ENABLE_APP_ADD_LED3
#define LED_3_STATE  (1 << 3)
#endif

#else
/* We only have 1 LED so map all control to the same LED */
#define LED_0_STATE  (1 << 0)
#define LED_1_STATE  (1 << 0)
#define LED_2_STATE  (1 << 0)
#endif

#ifdef ENABLE_APP_ADD_LED3
#define LED_RED     (LED_0_STATE)
#define LED_ORANGE  (LED_1_STATE)
#define LED_GREEN   (LED_2_STATE)
#define LED_WHITE   (LED_3_STATE)
#else
#define LED_BLUE    (LED_0_STATE)
#define LED_GREEN   (LED_1_STATE)
#define LED_RED     (LED_2_STATE)
#define LED_WHITE   (LED_0_STATE | LED_1_STATE | LED_2_STATE)
#define LED_YELLOW  (LED_RED | LED_GREEN)
#define LED_AMBER   (LED_RED | LED_BLUE)
#endif

/*!@} */


/*! \brief An LED filter used for low charging level

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_low(uint16 led_state)
{
    UNUSED(led_state);
    return LED_RED;
}

/*! \brief An LED filter used for charging level OK

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_ok(uint16 led_state)
{
    UNUSED(led_state);
    return LED_GREEN;
}

/*! \brief An LED filter used for charging complete

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_complete(uint16 led_state)
{
    UNUSED(led_state);
    return LED_GREEN;
}

/*! \cond led_patterns_well_named
    No need to document these. The public interface is
    from public functions such as HeadsetUi_Init()
 */

const led_pattern_t app_led_pattern_power_on[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),    LED_WAIT(2000),
    LED_OFF(LED_WHITE),
    LED_UNLOCK,
    LED_END
};

const led_pattern_t app_led_pattern_power_off[] =
{
    //LED_SYNC(1000),
    LED_LOCK,
    LED_ON(LED_RED),  LED_WAIT(100),
    LED_OFF(LED_RED), LED_WAIT(300),
    LED_ON(LED_RED),  LED_WAIT(100),
    LED_OFF(LED_RED), LED_WAIT(1300),
    LED_UNLOCK,
    LED_END
};

const led_pattern_t app_led_pattern_error[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100),
    LED_OFF(LED_WHITE), LED_WAIT(100),
    LED_REPEAT(1, 2),
    LED_UNLOCK,
    LED_END
};

const led_pattern_t app_led_pattern_idle[] =
{
    LED_SYNC(3000),
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100),
    LED_OFF(LED_WHITE), LED_WAIT(100),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const led_pattern_t app_led_pattern_idle_connected[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const led_pattern_t app_led_pattern_pairing[] =
{
#ifdef ENABLE_APP_BREATHING_PAIRING_LED
    LED_LOCK,
    LED_PWM_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0,0)
#else
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(100),
    LED_OFF(LED_BLUE), LED_WAIT(100),
    LED_UNLOCK,
    LED_REPEAT(0, 0)
#endif

};

const led_pattern_t app_led_pattern_pairing_deleted[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),  LED_WAIT(100),
    LED_OFF(LED_WHITE), LED_WAIT(100),
    LED_REPEAT(1, 2),
    LED_UNLOCK,
    LED_END
};


#ifdef INCLUDE_AV
const led_pattern_t app_led_pattern_streaming[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

#ifdef INCLUDE_AV
const led_pattern_t app_led_pattern_streaming_aptx[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

#ifdef INCLUDE_AV
const led_pattern_t app_led_pattern_streaming_aptx_adaptive[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

const led_pattern_t app_led_pattern_sco[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const led_pattern_t app_led_pattern_call_incoming[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

#ifdef ENABLE_APP_BATTERY_CHECK_LED
const led_pattern_t app_led_pattern_battery_high[] =
{
    LED_LOCK,
    LED_ON(LED_GREEN), LED_WAIT(2000),
    LED_OFF(LED_GREEN),
    LED_UNLOCK,
    LED_END
};

const led_pattern_t app_led_pattern_battery_medium[] =
{
    LED_LOCK,
    LED_ON(LED_ORANGE), LED_WAIT(2000),
    LED_OFF(LED_ORANGE),
    LED_UNLOCK,
    LED_END
};

const led_pattern_t app_led_pattern_battery_low[] =
{
    LED_LOCK,
    LED_ON(LED_RED), LED_WAIT(2000),
    LED_OFF(LED_RED),
    LED_UNLOCK,
    LED_END
};
#endif

const led_pattern_t app_led_pattern_battery_low_warning[] =
{
    LED_LOCK,
    LED_ON(LED_RED), LED_WAIT(100),
    LED_OFF(LED_RED),LED_WAIT(100),
    LED_UNLOCK,
    LED_END,
};

#ifdef ENABLE_APP_BATTERY_CHARGER_PIO_SETTING
const led_pattern_t app_led_pattern_battery_charging_led[] =
{
    LED_LOCK,
    LED_ON(LED_ORANGE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

const led_pattern_t app_led_pattern_battery_charge_completed[] =
{
    LED_LOCK,
    LED_ON(LED_GREEN),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const led_pattern_t app_led_pattern_battery_charge_detached[] =
{
    LED_LOCK,
    LED_OFF(LED_ORANGE),
    LED_OFF(LED_GREEN),
    LED_OFF(LED_RED),
    LED_OFF(LED_WHITE),
    LED_UNLOCK,
    //LED_REPEAT(0, 0),
    LED_END
};

#ifdef ENABLE_APP_OTA_LED
const led_pattern_t app_led_pattern_OTA[] =
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100),
    LED_OFF(LED_WHITE), LED_WAIT(200),
    LED_UNLOCK,
    LED_REPEAT(0, 0)
};
#endif

#ifdef ENABLE_APP_ENTER_DUT_MODE
const led_pattern_t app_led_pattern_DUT[] =
{
    LED_LOCK,
    LED_ON(LED_RED),    LED_WAIT(100),
    LED_OFF(LED_RED),   LED_WAIT(100),
    LED_UNLOCK,
    LED_REPEAT(0, 0)
};
#endif

#ifdef ENABLE_APP_FACTORY_RESET
const led_pattern_t app_led_pattern_factory_reset[] =
{
    LED_LOCK,
    LED_ON(LED_RED),  LED_WAIT(250),
    LED_OFF(LED_RED), LED_WAIT(250),
    LED_REPEAT(1, 3),
    LED_UNLOCK,
    LED_END
};
#endif

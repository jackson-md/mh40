/* Copyright (c) 2016-2021 Qualcomm Technologies International, Ltd. */
/*   %%version */


#ifndef PIOINT_H
#define PIOINT_H

#include "hydra/hydra_macros.h"
#include "hydra/hydra_types.h"
#include "hal/halauxio.h"
#include <stdint.h>


/** Compile time definition for the interrupt used by pioint. */
#define PIOINT_NO 2

/**
 * Initialise the pioint subsystem.
 */
extern void pioint_init(void);

/**
 * Configure interrupts on PIO pins.
 * \param bank PIO bank index.
 * \param mask PIOs allowed trigger the interrupt. These are indicated by a 
 * mask.
 * \param handler Function pointer. This will be called in interrupt context
 * when an event is detected.
 */
extern void pioint_configure(uint16 bank, pio_size_bits mask, void (*handler)(void));


typedef void (*pioint_handler_t)(void);
typedef void (*pioint_handler_with_context_t)(uintptr_t context, pio_size_bits bits);

/** Defines used to indicate if rising or falling edge will trigger interrupt
 * or timestamping */
#define PIOINT_RISING (0x01)
#define PIOINT_FALLING (0x02)

/**
 * Configure interrupts on PIO pins.
 * \param bank PIO bank index.
 * \param mask PIOs allowed trigger the interrupt. These are indicated by a
 * mask.
 * \param handler Function pointer. This will be called in interrupt context
 * when an event is detected.
 * \param context Context value passed to handler function.
 * \param triggers Specified if rising and/or falling edge of PIO will generate
 * interrupt
 */
extern void pioint_configure_with_context(uint16 bank, pio_size_bits mask,
                                          pioint_handler_with_context_t handler,
                                          uintptr_t context, uint8 triggers);

/**
 * Enable timestamping on PIO pin.
 * \param pio PIO index.
 * \param triggers Specified if rising and/or falling edge of PIO will be
 * timestamped
 * \return TRUE if timestamping was enabled, otherwise FALSE (normally due to
 * all HW instances in use already).
 */
extern bool pioint_enable_strobe_timestamp(uint8 pio, uint8 triggers);

/**
 * Disable timestamping on PIO pin.
 * \param pio PIO index.
 * \return TRUE if timestamping was disabled, otherwise FALSE (due to
 * timestamping not being enabled at time of call).
 */
extern bool pioint_disable_strobe_timestamp(uint8 pio);

/**
 * Get last PIO timestamp.
 * \param pio PIO index.
 * \return Returns timestamp of last PIO transition.  Note calling this on
 * a PIO with timestamping disabled will return 0.
 */
extern uint32 pioint_get_strobe_timestamp(uint8 pio);

#endif /* PIOINT_H */


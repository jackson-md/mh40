/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   %%version */
/**
 * \file
 * Manage interrupts on PIOs.
 */

#include "pioint/pioint_private.h"

/** Pointer to the start of the database. */
static pioint_config_entry *head;

/**
 * Handler that crt calls for PIO interrupt event.
 */
static void pioint_handler(void)
{
    pioint_config_entry *cur;
    pio_size_bits raised[NUMBER_OF_PIO_BANKS];
    uint32 i;

    /* Record the events which triggered the interrupt. */
    block_interrupts();
    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
       raised[i] = pio_get_event_cause(i);
       pio_clear_event_cause(i, raised[i]);
    }
    unblock_interrupts();

    /* Now loop over all registered entries. We would only call the handlers
       for ones interested in the PIOs that have changed. */
    for(cur = head; cur != NULL; )
    {
        pioint_config_entry *tmp = cur->next;
        const pio_size_bits triggered_mask = cur->mask & raised[cur->bank];
        if (triggered_mask)
        {
            if (cur->pass_context)
                cur->u.handler_with_context(cur->context, triggered_mask);
            else
                cur->u.handler();
        }
        cur = tmp;
    }
}


static void pioint_configure_internal(uint16 bank, pio_size_bits mask, uintptr_t handler, uintptr_t context, bool pass_context, uint8 triggers)
{
    pioint_config_entry *tmp, *entry, **prev_next;
    bool updated = FALSE, en_flag = FALSE;
    pio_size_bits master_mask[NUMBER_OF_PIO_BANKS];
    uint32 i;
    static bool enabled = FALSE;

    assert(bank < NUMBER_OF_PIO_BANKS);

    if(handler != 0)
    {
        memset(master_mask, 0U, sizeof(master_mask));
        /* Loop through the database and search for an existing handler-bank
           pair. */
        block_interrupts();
        for(prev_next = &head, entry = head; entry != NULL; )
        {
           if((entry->u.intptr == handler) && (entry->context == context) && (entry->bank == bank))
           {
               /* Matching handler-bank pair found */
               if(!mask)
               {
                   /* A mask with only 0s means we want to deregister.
                      Remove the entry from the list. */
                   *prev_next = entry->next;
               }
               /* Update the mask. This will be 0 for deregister calls. */
               entry->mask = mask;

               /* Update context for handler */
               entry->context = context;
               entry->pass_context = pass_context;

               /* Signal update completed */
               updated = TRUE;
           }
           /* Record the enabled interrupt sources */
           master_mask[entry->bank] |= entry->mask;
           if (!entry->mask)
           {
               /* This is only executed for deregister calls. entry->mask is
                  never 0 unless this is a deregister call. */
               tmp = entry->next;
               /* Free the removed entry */
               pfree(entry);
               /* prev_next does not need updating. It already points to the
                  next entry. */
               entry = tmp;
           }
           else
           {
               prev_next = &(entry->next);
               entry = entry->next;
           }
        }
        if((!updated) && mask)
        {
            /* This is a new handler/bank pair, add it in front of the list */
            entry = pnew(pioint_config_entry);
            entry->bank = bank;
            entry->mask = mask;
            entry->u.intptr = handler;
            entry->context = context;
            entry->pass_context = pass_context;
            entry->next = head;
            head = entry;
            /* Record the enabled interrupt sources */
            master_mask[entry->bank] |= entry->mask;
        }
        for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
        {
            uint32 prev_triggers;

            if (triggers & PIOINT_RISING)
            {
                prev_triggers = pio_get_rising_int_triggers(i);
                /* Make sure triggers to be enabled are not already raised */
                pio_clear_event_cause(i, (~prev_triggers) & master_mask[i]);
                /* Update rising triggers */
                pio_set_rising_int_triggers(i, master_mask[i]);
                /* Make sure disabled triggers are not left raised */
                pio_clear_event_cause(i, prev_triggers & (~master_mask[i]));
            }

            if (triggers & PIOINT_FALLING)
            {
                prev_triggers = pio_get_falling_int_triggers(i);
                /* Make sure triggers to be enabled are not already raised */
                pio_clear_event_cause(i, (~prev_triggers) & master_mask[i]);
                /* Update falling triggers */
                pio_set_falling_int_triggers(i, master_mask[i]);
                /* Make sure disabled triggers are not left raised */
                pio_clear_event_cause(i, prev_triggers & (~master_mask[i]));
            }

            /* If any bank has at least one enabled trigger then we set the
               flag to indicate that we need to register for a PIO interrupt */
            en_flag = master_mask[i]?TRUE:en_flag;
        }
        if((int)enabled != (int)en_flag)
        {
            /* Enable interrupts if interrupts are disabled and the flag wants
               them enabled. Similarly, disable interrupts if the interrupts
               are enabled and the flag wants them disabled. */
            enabled = en_flag;
            configure_interrupt(PIOINT_SOURCE, PIOINT_PRIORITY, en_flag?pioint_handler:NULL);
        }
        unblock_interrupts();
    }
}


void pioint_configure_with_context(uint16 bank, pio_size_bits mask, pioint_handler_with_context_t handler, uintptr_t context, uint8 triggers)
{
    pioint_configure_internal(bank, mask, (uintptr_t)handler, context, TRUE, triggers);

}


void pioint_configure(uint16 bank, pio_size_bits mask, pioint_handler_t handler)
{
    pioint_configure_internal(bank, mask, (uintptr_t)handler, 0, FALSE, PIOINT_RISING | PIOINT_FALLING);
}


#define NO_PIO (255)

static uint8 pio_strobe[2] =
{
    NO_PIO, /* PIO in use for HW timestamping instance 1 */
    NO_PIO  /* PIO in use for HW timestamping instance 2 */
};


bool pioint_enable_strobe_timestamp(uint8 pio, uint8 triggers)
{
    if (pio_strobe[0] == NO_PIO)
    {
        pio_strobe[0] = pio;
        hal_set_kalimba_pio_int_timer_pio_strobe1(pio);
        hal_set_kalimba_pio_int_timer_pio_strobe1_rising_en(triggers & PIOINT_RISING ? 1 : 0);
        hal_set_kalimba_pio_int_timer_pio_strobe1_falling_en(triggers & PIOINT_FALLING ? 1 : 0);
        return TRUE;
    }
    else if (pio_strobe[1] == NO_PIO)
    {
        pio_strobe[1] = pio;
        hal_set_kalimba_pio_int_timer_pio_strobe2(pio);
        hal_set_kalimba_pio_int_timer_pio_strobe2_rising_en(triggers & PIOINT_RISING ? 1 : 0);
        hal_set_kalimba_pio_int_timer_pio_strobe2_falling_en(triggers & PIOINT_FALLING ? 1 : 0);
        return TRUE;
    }

    return FALSE;
}


bool pioint_disable_strobe_timestamp(uint8 pio)
{
    if (pio_strobe[0] == pio)
    {
        pio_strobe[0] = NO_PIO;
        hal_set_kalimba_pio_int_timer_pio_strobe1_rising_en(0);
        hal_set_kalimba_pio_int_timer_pio_strobe1_falling_en(0);
        return TRUE;
    }
    else if (pio_strobe[1] == pio)
    {
        pio_strobe[1] = NO_PIO;
        hal_set_kalimba_pio_int_timer_pio_strobe2_rising_en(0);
        hal_set_kalimba_pio_int_timer_pio_strobe2_falling_en(0);
        return TRUE;
    }

    return FALSE;
}


uint32 pioint_get_strobe_timestamp(uint8 pio)
{
    if (pio_strobe[0] == pio)
        return KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME;
    else if (pio_strobe[1] == pio)
        return KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME;
    else
        return 0;
}

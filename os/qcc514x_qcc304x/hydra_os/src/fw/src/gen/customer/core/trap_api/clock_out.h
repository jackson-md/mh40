#ifndef __CLOCK_OUT_H__
#define __CLOCK_OUT_H__
/** \file */
#if TRAPSET_CLOCK_OUT

/**
 *  \brief Output a buffered version of the XTAL clock. 
 *         Drive a buffered version of the 32MHz XTAL on the pin XTAL_CLKOUT
 *         for packages which have this pin. 
 *         
 *         Notes: 
 *         
 *         Only supported on QCC517x and QCC307x devices.
 *         
 *         This feature will increase the current consumption of the device. 
 *         
 *         Deep sleep will automatically be disabled while XTAL_CLKOUT is
 *         enabled this ensures the XTAL remains driven whilst this clock is 
 *         request regardless of the chip activity. 
 *         
 *  \param enable The XTAL clock enable state. 
 *  \return TRUE if the clock was enabled/disabled, FALSE if it failed. 
 * 
 * \ingroup trapset_clock_out
 */
bool XtalClockOutEnable(bool enable);
#endif /* TRAPSET_CLOCK_OUT */
#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file sync_timer.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 02.01.2014
 * \version 23
 *
 * \brief Timer for event loop synchronization - implementation.
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"
#include "lib_mcu_noarch/global.h"
#include "lib_mcu_noarch/sync_timer/sync_timer.h"

/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

// definitions check
#ifndef SYNC_TIMER_TIMERS
	#error "SYNC_TIMER: SYNC_TIMER_TIMERS is not set!"
#endif // SYNC_TIMER_TIMERS

// timer counter
volatile UINT8 ui8_sync_timer_overflow = 0;

// variable containing ticks
volatile tick_type_t ticks = 0;

#ifdef SYNC_TIMER_JIFFIES
volatile UINT32 jiffies = 0;
#endif // SYNC_TIMER_JIFFIES

// array of timer tick information
volatile UINT16 aui16_timers[SYNC_TIMER_TIMERS];


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
BOOL syncTimerGetTimer(const UINT8 ui8_timer, const UINT16 ui16_value)
{
	// indicates whether timer is reached
	BOOL b;

	// check whether timer is running
	if (!(aui16_timers[ui8_timer] & SYNC_TIMER_RUN_MASK))
		return (false);

	b = ((aui16_timers[ui8_timer] & ~(SYNC_TIMER_RUN_MASK)) >= ui16_value);

	// reset timer if already exceeded
	if (b)
	{
#ifdef SYNC_TIMER_MAINTAIN_PERIOD
		aui16_timers[ui8_timer] -= ui16_value;
#else
		aui16_timers[ui8_timer] = SYNC_TIMER_RUN_MASK;
#endif // SYNC_TIMER_MAINTAIN_PERIOD
	}

	return (b);
}

// --------------------------------------------------------------------------
VOID syncTimerRestart(const UINT8 ui8_timer)
{
	// reset tick count and set start bit
	aui16_timers[ui8_timer] = SYNC_TIMER_RUN_MASK;
}

// --------------------------------------------------------------------------
VOID syncTimerStart(const UINT8 ui8_timer)
{
	// set start bit
	sbi(aui16_timers[ui8_timer], SYNC_TIMER_RUN_BIT);
}

// --------------------------------------------------------------------------
VOID syncTimerStop(const UINT8 ui8_timer)
{
	// clear start bit
	cbi(aui16_timers[ui8_timer], SYNC_TIMER_RUN_BIT);
}

// --------------------------------------------------------------------------
VOID syncTimerUpdate(void)
{
	volatile UINT16 tmp_ticks = ticks;
	// update all timers
	for (UINT8 i = 0; i < SYNC_TIMER_TIMERS; ++i)
	{
		// update only running timers
		if (aui16_timers[i] & SYNC_TIMER_RUN_MASK)
			aui16_timers[i] += tmp_ticks;
	}

	// reset tick counters
	ticks -= tmp_ticks;
}

// END

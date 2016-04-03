/*!
 * \file sync_timer.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 02.01.2014
 * \version 23
 *
 * \brief Timer for event loop synchronization - definitions.
 * \details
 * Utilizes one timer/systick of processor to count 1ms ticks. Can be used to
 * perform operations with given delay. E.g. Some part of main() can be executed
 * once a second or 500ms. Synchronization ticks are counted on timer overflow interrupt basis.
 * Ticks are counted each time timer overflows and aditional divider is exceeded. User timers (counters)
 * are updated only when syncTimerUpdate() was invoked. It is vital to call this function frequently to
 * acheive wanted time resolution.
 * \note sync ticks presents following equotation:
 * sync = fcpu/(SYNC_TIMER_PRESCALER*SYNC_TIMER_PRESCALER*SYNC_TIMER_TC_COUNT*SYNC_TIMER_COUNT).
 * Sync value should give execution each 1/1000 of second.
 * \note
 * Timers are started immediately after syncTimerInit() invocation. If you wish to start timer in certain
 * moment you have to reset the timer using syncTimerReset().
 * Timer may be stopped/started again using syncTimerStart() and syncTimerStop() functions.
 * \note
 * This library need to set following constants in config.h to work:
 * - SYNC_TIMER_PRESCALER	- value to divide fcpu signal for timer
 * - SYNC_TIMER_TC_COUNT	- max value for counter
 * - SYNC_TIMER_COUNT		- additional divider for reaching desired sync times
 * - SYNC_TIMER_TIMERS		- numbers of separate timers that are used (counting 1ms)
 * 
 * - SYNC_TIMER_MAINTAIN_PERIOD - if defined sync_timer tries to maintain period, otherwise
 * 								  tick count for particular timer is set to 0.
 * - SYNC_TIMER_EXT_HANDLER	- if defined additional handler defined by user is executed each time tick
 * 							  count is incremented.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in sync_timer_march.c.
 */

#ifndef _SYNC_TIMER_H
#define _SYNC_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \def SYNC_TIMER_RUN_BIT
 * \brief bit in timer counter indicating whether timer started or stopped, used internally
 */
#define SYNC_TIMER_RUN_BIT 15

/*!
 * \def SYNC_TIMER_RUN_MASK
 * \brief mask for setting timer started or stopped, used internally
 */
#define SYNC_TIMER_RUN_MASK BV(SYNC_TIMER_RUN_BIT)

/*!
 * \typedef tick_type_t
 * \brief timer tick type dependent on architecture
 */
typedef TICK_TYPE tick_type_t;

/*!
 * \var jiffies
 * \brief ticks in system specific unit (ms, us, ...)
 * \note variable is unavailable if SYNC_TIMER_JIFFIES is not declared in config.h
 */
#ifdef SYNC_TIMER_JIFFIES
extern volatile UINT32 jiffies;
#endif // SYNC_TIMER_JIFFIES


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn syncTimerGetTimer(const UINT8 ui8_timer, const UINT16 ui16_value)
 * \brief checks whether given timer has exceeded value (do not work for stopped timers)
 * \param ui8_timer number of timer
 * \param ui16_value value which timer counts to
 * \return true if timer has been exceeded, false otherwise
 * \note requires syncTimerUpdate invocation to update counters status
 */
BOOL syncTimerGetTimer(const UINT8 ui8_timer, const UINT16 ui16_value);
/*!
 * \fn syncTimerInit
 * \brief initializes timer for counting ticks, assigns interrupt handler
 */
VOID syncTimerInit(void);
/*!
 * \fn syncTimerRestart(const UINT8 ui8_timer)
 * \brief restarts the timer and clears current tick count
 * \param ui8_timer number of timer
 */
VOID syncTimerRestart(const UINT8 ui8_timer);
/*!
 * \fn syncTimerStart(const UINT8 ui8_timer)
 * \brief starts given stopped timer
 * \param ui8_timer number of timer
 */
VOID syncTimerStart(const UINT8 ui8_timer);
/*!
 * \fn syncTimerStop(const UINT8 ui8_timer)
 * \brief stops given timer
 * \param ui8_timer number of timer
 */
VOID syncTimerStop(const UINT8 ui8_timer);
/*!
 * \fn syncTimerUpdate
 * \brief updates all timer status, have to be invocated frequently, e.g. in main loop
 */
VOID syncTimerUpdate(void);

#ifdef SYNC_TIMER_EXT_HANDLER
/*!
 * \fn syncTimerUpdate
 * \brief user handler executed each time tick counter is incremented
 * \note function is unavailable if SYNC_TIMER_EXT_HANDLER is not declared in config.h
 */
VOID syncTimerExtHandler(tick_type_t ticks);
#endif // SYNC_TIMER_EXT_HANDLER

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _SYNC_TIMER_H

// END

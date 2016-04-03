/*!
 * \file pid.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 01.02.2014
 * \version 1
 *
 * \brief Simple 8-bit Proportional-Integral-Derivative controller - definitions
 * \details
 * Generates PID process value according to given factors.
 */

#ifndef _PID_H
#define _PID_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \struct ST_PID_DATA
 * \brief contains PID controller configuration and values required for
 * process value computation.
 */
struct ST_PID_DATA
{
	INT8 i8_P_factor;
	INT8 i8_I_factor;
	INT8 i8_D_factor;
	INT8 i16_last_proc_val;

	INT8 i8_max_error;

	INT16 i16_int_error;
	INT16 i16_max_int_error;

	INT8 i8_max_result;
	INT8 i8_min_result;
};
/*!
 * \typedef PID_DATA
 * \brief PID controller settings structure
 */
typedef struct ST_PID_DATA PID_DATA;

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn pidInit(const INT8 i8_p_factor, const INT8 i8_i_factor, const INT8 i8_d_factor, const INT8 i8_min, const INT8 i8_max, PID_DATA *st_pid_data)
 * \brief initializes pid controller with given settings
 * \param i8_p_factor proportional factor
 * \param i8_i_factor integral factor
 * \param i8_d_factor derivative factor
 * \param i8_min minimum value that pidProcess can return
 * \param i8_max maximum value that pidProcess can return
 * \param st_pid_data pointer to control structure
 */
void pidInit(const INT8 i8_p_factor, const INT8 i8_i_factor, const INT8 i8_d_factor, const INT8 i8_min, const INT8 i8_max, PID_DATA* st_pid_data);
/*!
 * \fn INT8 pidProcess(const INT16 i16_set_point, const INT16 i16_proc_val, PID_DATA *st_pid_data)
 * \brief computes control value based on process value and setpoint (error)
 * \param i16_set_point expected process value
 * \param i16_proc_val current process value
 * \param st_pid_data pointer to control structure
 * \return proces value
 */
INT8 pidProcess(const INT16 i16_set_point, const INT16 i16_proc_val, PID_DATA* st_pid_data);
/*!
 * \fn pidResetIntegrator(PID_DATA *st_pid_data)
 * \brief resets integral part of pid controller
 * \param st_pid_data pointer to control structure
 */
void pidResetIntegrator(PID_DATA *st_pid_data);

#endif // _PID_H

// END

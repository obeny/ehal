/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file pid.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 18.05.2014
 * \version 2
 *
 * \brief Simple 8-bit Proportional-Integral-Derivative controller - implementation
 * \details
 * Generates PID process value according to given factors.
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/pid/pid.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
void pidInit(const INT8 i8_p_factor, const INT8 i8_i_factor, const INT8 i8_d_factor, const INT8 i8_min, const INT8 i8_max, PID_DATA* st_pid_data)
{
	st_pid_data->i8_P_factor = i8_p_factor;
	st_pid_data->i8_I_factor = i8_i_factor;
	st_pid_data->i8_D_factor = i8_d_factor;

	st_pid_data->i16_last_proc_val = 0;

	st_pid_data->i8_max_error = INT8_MAX / (st_pid_data->i8_P_factor + 1);

	st_pid_data->i16_int_error = 0;
	st_pid_data->i16_max_int_error = ((INT8_MAX / 2) / (st_pid_data->i8_I_factor + 1));

	st_pid_data->i8_min_result = i8_min;
	st_pid_data->i8_max_result = i8_max;
}

// --------------------------------------------------------------------------
INT8 pidProcess(const INT16 i16_set_point, const INT16 i16_proc_val, PID_DATA *st_pid_data)
{
	INT16 i16_error = (INT16)i16_set_point - (INT16)i16_proc_val;
	INT8 i8_p_term, i8_i_term, i8_d_term;

	INT16 i16_result;
	INT16 i16_comp;

	// calculate p-term and limit error overflow
	if (st_pid_data->i8_P_factor == 0)
		i8_p_term = 0;
	else
	{
		if (i16_error > st_pid_data->i8_max_error)
			i8_p_term = INT8_MAX;
		else if ( i16_error < -(st_pid_data->i8_max_error) )
			i8_p_term = INT8_MIN;
		else
			i8_p_term = st_pid_data->i8_P_factor * i16_error;
	}

	// calculate i-term and limit integral runaway
	i16_comp = st_pid_data->i16_int_error + i16_error;
	if (st_pid_data->i8_I_factor == 0)
		i8_i_term = 0;
	else
	{
		if (i16_comp > st_pid_data->i16_max_int_error)
		{
			i8_i_term = (INT8_MAX / 2);
			st_pid_data->i16_int_error = st_pid_data->i16_max_int_error;
		}
		else if (i16_comp < -(st_pid_data->i16_max_int_error - 1))
		{
			i8_i_term = (INT8_MIN / 2);
			st_pid_data->i16_int_error = -st_pid_data->i16_max_int_error;
		}
		else
		{
			st_pid_data->i16_int_error = i16_comp;
			i8_i_term = st_pid_data->i8_I_factor * st_pid_data->i16_int_error;
		}
	}

	// calculate d-term
	if (st_pid_data->i8_D_factor != 0)
		i8_d_term = st_pid_data->i8_D_factor * (st_pid_data->i16_last_proc_val - i16_proc_val);
	else
		i8_d_term = 0;

	st_pid_data->i16_last_proc_val = i16_proc_val;

	// calculate final P I D amplification
	i16_result = i8_p_term + i8_i_term + i8_d_term;
	if (i16_result > INT8_MAX)
		i16_result = INT8_MAX;
	else if (i16_result < INT8_MIN)
		i16_result = INT8_MIN;

	if (i16_result > st_pid_data->i8_max_result)
		i16_result = st_pid_data->i8_max_result;
	else if (i16_result < st_pid_data->i8_min_result)
		i16_result = st_pid_data->i8_min_result;

	// shrink to return value type
	return ((INT8)i16_result);
}

// --------------------------------------------------------------------------
void pidResetIntegrator(PID_DATA* st_pid_data)
{
  st_pid_data->i16_int_error = 0;
}

// END

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file adc_isr.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 04.01.2014
 * \version 1
 *
 * \brief Analog-to-Digital converter control library using interrupts - implementation.
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"
#include "lib_mcu_noarch/adc/adc_isr.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

// check sample count
#ifndef ADC_SAMPLES_COUNT
	#error "ADC: ADC_SAMPLES_COUNT is not set!"
#endif

#if (ADC_SAMPLES_COUNT & ADC_SAMPLES_COUNT_MASK)
	#error "ADC: ADC_SAMPLES_COUNT is not a power of 2!"
#endif

// check channel count
#ifndef ADC_CHANNELS_COUNT
	#error "ADC: ADC_CHANNELS_COUNT is not set!"
#endif

#if (ADC_SAMPLES_COUNT == 1)
	#define ADC_SAMPLES_SHIFT 0
#elif (ADC_SAMPLES_COUNT == 2)
	#define ADC_SAMPLES_SHIFT 1
#elif (ADC_SAMPLES_COUNT == 4)
	#define ADC_SAMPLES_SHIFT 2
#elif (ADC_SAMPLES_COUNT == 8)
	#define ADC_SAMPLES_SHIFT 3
#else
	#define ADC_SAMPLES_SHIFT 0xFF
#endif

volatile UINT16 ui16_adc_measurements[ADC_CHANNELS_COUNT][ADC_SAMPLES_COUNT];
volatile UINT8 ui8_adc_channel_mapping[ADC_CHANNELS_COUNT];
volatile UINT8 ui8_adc_current_channel = 0;
volatile UINT8 ui8_adc_sample = 0;


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
UINT16 adcIsrResult(const UINT8 channel)
{
	UINT16 ui16_result = 0;

	for (UINT8 i = 0; i < ADC_SAMPLES_COUNT; ++i)
		ui16_result += ui16_adc_measurements[channel][i];

#if (ADC_SAMPLES_SHIFT == 0xFF)
	ui16_result /= ADC_SAMPLES_COUNT;
#else
	ui16_result = (ui16_result >> ADC_SAMPLES_SHIFT);
#endif // (ADC_SAMPLES_SHIFT == 0xFF)

	return (ui16_result);
}

// --------------------------------------------------------------------------
void adcIsrSetMapping(const UINT8 hw_channel, const UINT8 index)
{
	ui8_adc_channel_mapping[index] = hw_channel;
}

// END

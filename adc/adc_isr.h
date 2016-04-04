/*!
 * \file adc_isr.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 04.01.2014
 * \version 1
 *
 * \brief Analog-to-Digital converter control library using interrupts - definitions
 * \details
 * Non-blocking reading of ADC values. Each conversion completion inserts result into array related with
 * selected ADC channel. Value can be read at any time. Automatical averaging with given number
 * of samples can be performed.
 *
 * This library needs to work following definitions to be set in config.h:
 * - ADC_SAMPLES_COUNT - samples used to calculate current ADC value
 * - ADC_CHANNELS_COUNT - number of channels for which values have to be collected
 * 
 * \note
 * Logical channels have to be mapped with particular HW channel with adcIsrSetMapping.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in adc_isr_march.c.
 */

#ifndef _ADC_ISR_H
#define _ADC_ISR_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "ehal/global.h"
#include "config.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \def ADC_SAMPLES_COUNT_MASK
 * \brief mask used for circular iteration over channel samples
 */
#define ADC_SAMPLES_COUNT_MASK (ADC_SAMPLES_COUNT - 1)


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn adcIsrInit(const UINT8 ui8_reference, const UINT8 ui8_prescaler)
 * \brief initialize interrupt handled ADC readings
 * \param ui8_reference ADC reference voltage (see ADC_REFERENCE in adc_common_mach.h)
 * \param ui8_prescaler ADC frequency prescaler (see ADC_PRESCALER in adc_common_mach.h)
 */
void adcIsrInit(const UINT8 ui8_reference, const UINT8 ui8_prescaler);
/*!
 * \fn adcIsrSetMapping(const UINT8 ui8_hw_channel, const UINT8 ui8_index)
 * \brief map hardware channel to selected position in logical channel array
 * \param ui8_hw_channel hardrware channel (see ADC_CH in adc_common_mach.h)
 * \param ui8_index logical channel number
 * \note Channel mapping have to be performed before adcIsrStart.
 */
void adcIsrSetMapping(const UINT8 ui8_hw_channel, const UINT8 ui8_index);

/*!
 * \fn adcIsrStart(void)
 * \brief start ADC conversion, enable interrupt
 */
void adcIsrStart(void);
/*!
 * \fn adcIsrStop(void)
 * \brief stop ADC conversion, disable interrupt
 */
void adcIsrStop(void);

/*!
 * \fn adcIsrResult(const UINT8 ui8_channel)
 * \brief read current averaged value for selected logical ADC channel
 * \param ui8_channel logical channel number
 * \return averaged ADC value
 */
UINT16 adcIsrResult(const UINT8 ui8_channel);

#endif // _ADC_ISR_H

// END

/*!
 * \file adc.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 04.01.2014
 * \version 1
 *
 * \brief Analog-to-Digital converter control library using polling - definitions
 * \details
 * Blocking reading of ADC values. When conversion is started it required to wait until
 * it completes. Each conversion completion result can be read only once.
 * 
 * \note
 * Before starting conversion, hardware channel have to be selected.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in adc_march.c.
 */

#ifndef _ADC_H
#define _ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib/adc/adc_common_march.h"
#include "config.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn adcInit(const UINT8 ui8_reference, const UINT8 ui8_prescaler)
 * \brief initialize polling ADC readings
 * \param ui8_reference ADC reference voltage (see ADC_REFERENCE in adc_common_mach.h)
 * \param ui8_prescaler ADC frequency prescaler (see ADC_PRESCALER in adc_common_mach.h)
 */
void adcInit(const UINT8 ui8_reference, const UINT8 ui8_prescaler);
/*!
 * \fn adcOff(void)
 * \brief disable ADC conversion and release connected i/o pins
 */
void adcOff(void);

/*!
 * \fn adcSetChannel(const UINT8 ui8_ch)
 * \brief select ADC hw channel
 * \param ui8_ch hardrware channel (see ADC_CH in adc_common_mach.h)
 */
void adcSetChannel(const UINT8 ui8_ch);

/*!
 * \fn adcStart(void)
 * \brief start ADC conversion
 */
void adcStart(void);

/*!
 * \fn adcConversionIsComplete(void)
 * \brief tells whether latest ADC conversion has finished 
 * \return true if conversion is finished, false otherwise
 */
BOOL adcConversionIsComplete(void);

/*!
 * \fn adcResult(void)
 * \brief read value for selected hardware ADC channel
 * \return ADC value
 */
UINT16 adcResult(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif //_ADC_H

// END

/*!
 * \file spi.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 09.03.2016
 * \version 2
 *
 * \brief Blocking SPI serial interface handler - definitions.
 * \details
 * Blocking implementation of SPI control handler. It supports only 8-bit transfers.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in spi_march.c.
 */

#ifndef _SPI_H
#define _SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib/spi/spi_march.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \enum e_spiclk_pol
 * \brief spi clock polarity
 */
enum e_spiclk_pol
{
	SPI_CLK_POL_POS	= 0,
	SPI_CLK_POL_NEG	= 1
};
/*!
 * \typedef e_spiclk_pol_t
 * \brief spi clock polarity
 */
typedef enum e_spiclk_pol e_spiclk_pol_t;

/*!
 * \enum e_spiclk_pha
 * \brief spi clock phase
 */
enum e_spiclk_pha
{
	SPI_CLK_PHA_SAMPLE	= 0,
	SPI_CLK_PHA_SETUP	= 1
};
/*!
 * \typedef e_spiclk_pha_t
 * \brief spi clock phase
 */
typedef enum e_spiclk_pha e_spiclk_pha_t;

/*!
 * \enum e_spiclk_ord
 * \brief spi data order lsb/msb first
 */
enum e_spiclk_ord
{
	SPI_CLK_ORD_MSB	= 0,
	SPI_CLK_ORD_LSB	= 1
};
/*!
 * \typedef e_spiclk_ord_t
 * \brief spi data order lsb/msb first
 */
typedef enum e_spiclk_ord e_spiclk_ord_t;


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn spiMasterInit(const spi_cfg_st *spi, const e_spiclk_pol_t clk_pol, const e_spiclk_pha_t clk_pha, const e_spiclk_ord_t clk_ord)
 * \brief initializes SPI in master mode
 * \param spi pointer to structure containing hardware specific information required by driver
 * \param clk_pol clock polarity (see e_spiclk_pol)
 * \param clk_pha clock phase (see e_spiclk_pha)
 * \param clk_ord clock data order (see e_spiclk_ord)
 */
void spiMasterInit(const spi_cfg_st *spi, const e_spiclk_pol_t clk_pol, const e_spiclk_pha_t clk_pha, const e_spiclk_ord_t clk_ord);

/*!
 * \fn spiSendByte(const BYTE c)
 * \brief sends byte to slave
 * \param spi pointer to structure containing hardware specific information required by driver
 * \param c byte to send
 */
void spiSendByte(const spi_cfg_st *spi, const BYTE c);
/*!
 * \fn spiSendByteStrobeToggle(const spi_cfg_st *spi, const BYTE c)
 * \brief sets slave select as active, sends byte to slave and sets slave as inactive
 * \param spi pointer to structure containing hardware specific information required by driver
 * \param c byte to send
 */
void spiSendByteStrobeToggle(const spi_cfg_st *spi, const BYTE c);

/*!
 * \fn spiTransferByte(const spi_cfg_st *spi, const BYTE c)
 * \brief sends byte to slave and reads response
 * \param spi pointer to structure containing hardware specific information required by driver
 * \param c byte to send
 * \return byte received from slave
 */
BYTE spiTransferByte(const spi_cfg_st *spi, const BYTE c);
/*!
 * \fn spiTransferByteStrobeToggle(const spi_cfg_st *spi, const BYTE c)
 * \brief sets slave select as active, sends byte to slave, reads response and sets slave as inactive
 * \param spi pointer to structure containing hardware specific information required by driver
 * \param c byte to send
 * \return byte received from slave
 */
BYTE spiTransferByteStrobeToggle(const spi_cfg_st *spi, const BYTE c);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _SPI_H

// END

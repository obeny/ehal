/*!
 * \file global.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 02.01.2014
 * \version 19
 *
 * \brief Globally used macros and type definitions
 * \details
 * File contains various types definitions, helper macros, function attributes.
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/march.h"

/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \def __force_inline
 * \brief enforces function inlining
 */
#define __force_inline inline __attribute__((always_inline))

/*!
 * \def __naked
 * \brief marks function as naked (no prologue, epilogue)
 */
#define __naked __attribute__((naked))

/*!
 * \def __no_return
 * \brief marks function as no_return
 */
#define __no_return __attribute__((noreturn))

/*!
 * \def __weak
 * \brief marks function as weak to allow overriding default implementation
 */
#define __weak __attribute__((weak))

/*!
 * \typedef VOID
 * \brief VOID type
 */
typedef void VOID;

/*!
 * \typedef BYTE
 * \brief BYTE type - unsigned char 8-bit
 */
typedef uint8_t BYTE;
/*!
 * \typedef BOOL
 * \brief BOOL type
 */
typedef bool BOOL;

/*!
 * \typedef INT8
 * \brief INT8 type - signed integer, 8-bit
 */
typedef int8_t INT8;
/*!
 * \typedef INT16
 * \brief INT16 type - signed integer, 16-bit
 */
typedef int16_t INT16;
/*!
 * \typedef INT32
 * \brief INT32 type - signed integer, 32-bit
 */
typedef int32_t INT32;

/*!
 * \typedef UINT8
 * \brief UINT8 type - unsigned integer, 8-bit
 */
typedef uint8_t UINT8;
/*!
 * \typedef UINT16
 * \brief UINT16 type - unsigned integer, 16-bit
 */
typedef uint16_t UINT16;
/*!
 * \typedef UINT32
 * \brief UINT32 type - unsigned integer, 32-bit
 */
typedef uint32_t UINT32;

/*!
 * \typedef _WORD
 * \brief _WORD type - size of dependent on architecture word size
 */
typedef WORD_TYPE _WORD;


/***************************************************************************
 *	MACROS
 ***************************************************************************/

/*!
 * \def BV
 * \brief gives a value with given bit set
 */
#define BV(bit) (1 << (bit))

/*!
 * \def FREE
 * \brief frees dynamically allocated resource and assigns NULL to pointer
 */
#define FREE(ptr) { if (ptr) { free(ptr); ptr = NULL; } }

/*!
 * \def TO_BOOL
 * \brief converts value to true/false
 */
#define TO_BOOL(val) (!(!(val)))

/*!
 * \def sbi
 * \brief sets given bit in value
 */
#define sbi(val, bit) (val) |= (BV(bit))
/*!
 * \def cbi
 * \brief clears given bit in value
 */
#define cbi(val, bit) (val) &= ~(BV(bit))

/*!
 * \def GET_BIT
 * \brief gets value of given bit in value
 */
#define _GET_BIT_(value, bit_n) ((value) & (BV(bit_n)))

/*!
 * \def SET_BIT
 * \brief sets value of given bit in value
 */
#define _SET_BIT_(value, bit_n, bool_val) ((bool_val)?(sbi(value, bit_n)):(cbi(value, bit_n)))

/*!
 * \def TOGGLE_BIT
 * \brief changes given bit in value to opposite
 */
#define _TOGGLE_BIT_(value, bit_n) (value ^= BV(bit_n))

/*!
 * \def NOP
 * \brief performs a short delay with asm "nop" instruction
 */
#define NOP __asm__ __volatile__("nop"::)

/*!
 * \def BIT_IS_CLEAR
 * \brief checks whether given bit is clear in value
 */
#define BIT_IS_CLEAR(value, bit_n) (~(value) & BV(bit_n))
/*!
 * \def BIT_IS_SET
 * \brief checks whether given bit is set in value
 */
#define BIT_IS_SET(value, bit_n) ((value) & BV(bit_n))

/*!
 * \def WAIT_FOR
 * \brief loop until condition is not met
 */
#define WAIT_FOR(x) while((x));
/*!
 * \def WAIT_FOR_BIT_CLEAR
 * \brief perform a delay loop waiting for clearing bit in value
 */
#define WAIT_FOR_BIT_CLEAR(value, bit_n) while(BIT_IS_SET((value), bit_n));
/*!
 * \def WAIT_FOR_BIT_SET
 * \brief perform a delay loop waiting for setting bit in value
 */
#define WAIT_FOR_BIT_SET(value, bit_n) while(BIT_IS_CLEAR((value), bit_n));

/*!
 * \def DIV_POS_INT_ROUND_CLOSEST
 * \brief round integer division result to closest value
 */
#define DIV_POS_INT_ROUND_CLOSEST(N, D) ( ((N) == 0 || (N <= (D/2))) ? 0:((((N) - (((D)/2) + (D & 1)))/(D)) + 1) )

/*!
 * \def QUOTE
 * \brief quotes a value
 */
#define QUOTE(x) #x

#endif // _GLOBAL_H
// END

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/debug/debug.h"

#include <ctype.h>
#include <stdio.h>

/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

#define DEBUG_XXD_COLS 16


 /***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

void xxd(BYTE *buf, UINT32 len)
{
	UINT8 col;
	UINT32 lines = (len % DEBUG_XXD_COLS)?(len / DEBUG_XXD_COLS + 1):(len / DEBUG_XXD_COLS);

	for (UINT32 line = 0; line < lines; ++line, buf += DEBUG_XXD_COLS, len -= DEBUG_XXD_COLS)
	{
		printf("0x%08lx:", (unsigned long)buf);
		for (col = 0; (col < DEBUG_XXD_COLS) && (col < len); ++col)
			printf(" %02X", buf[col]);

		printf(" | ");
		for (col = 0; (col < DEBUG_XXD_COLS) && (col < len); ++col)
			printf("%c", (isprint(buf[col]))?(buf[col]):('.'));

		printf("\n");
	}
}

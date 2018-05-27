#ifndef DISASSEMBLE_INC
#define DISASSEMBLE_INC

#include <stdio.h>

#include "bool.h"
#include "input.h"
#include "sort.h"
#include "options.h"

#define VECTORS_START	0x0000
#define HEADER_START	0x0100
#define ROM0_START	0x0150
#define ROMX_START	0x4000 /*Coincidentally, also the multiple for every bank.*/

#define AMT_ONE_OPS	19
#define AMT_TWO_OPS	6
#define AMT_ABS_JMPS	10
#define AMT_REL_JMPS	5

extern const unsigned char ONE_OPERAND_OPS[AMT_ONE_OPS];
extern const unsigned char TWO_OPERAND_OPS[AMT_TWO_OPS];
extern const unsigned char ABSOLUTE_JUMPS[AMT_ABS_JMPS];
extern const unsigned char RELATIVE_JUMPS[AMT_REL_JMPS];

struct jumps_struct
{
	size_t *absolutes;
	size_t *relatives;
	size_t absolutes_index;
	size_t relatives_index;
	size_t absolutes_size;
	size_t relatives_size;
};

void findPossibleJumps(struct input_struct *input, struct jumps_struct *jumps);
void saveRelativeJumpMacros(FILE *file_pointer);
void simpleDisassemble(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options);
void complexDisassemble(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options);

#endif /*DISASSEMBLE_INC*/

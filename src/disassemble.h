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

struct labels_s
{
	struct array_s absolutes;
	struct array_s relatives;
};

void findPossibleJumps(struct input_s *input, struct labels_s *labels);
void saveRelativeJumpMacros(FILE *file_pointer);
void simpleDisassemble(FILE *file_pointer, struct input_s *input, struct labels_s *labels, struct options_s *options);
void complexDisassemble(FILE *file_pointer, struct input_s *input, struct labels_s *labels, struct options_s *options);

#endif /*DISASSEMBLE_INC*/

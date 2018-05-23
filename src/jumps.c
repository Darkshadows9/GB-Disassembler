#include <stdio.h>
#include <stdlib.h>

#include "bool.h"
#include "input.h"
#include "sort.h"
#include "jumps.h"
#include "disassemble.h"

const unsigned char ONE_OPERAND_OPS[AMT_ONE_OPS] = {0x06, 0x0E, 0x16, 0x26, 0x2E, 0x36, 0x3E, 0xC6, 0xCE, 0xD6, 0xDE, 0xE0, 0xE6, 0xE8, 0xEE, 0xF0, 0xF6, 0xF8, 0xFE};
const unsigned char TWO_OPERAND_OPS[AMT_TWO_OPS] = {0x01, 0x08, 0x11, 0x21, 0xEA, 0xFA};
const unsigned char ABSOLUTE_JUMPS[AMT_ABS_JMPS] = {0xC2, 0xC3, 0xC4, 0xCA, 0xCC, 0xCD, 0xD2, 0xD4, 0xDA, 0xDC};
const unsigned char RELATIVE_JUMPS[AMT_REL_JMPS] = {0x18, 0x20, 0x28, 0x30, 0x38};

void saveRelativeJumpMacros(FILE *file_pointer) /*We need these macros, becase RGBDS doesn't properly support relative jumps with pure hex values, only labels.*/
{
	fprintf(file_pointer, "jumpRelative EQUS	\"db	18\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeNZ EQUS	\"db	20\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeZ EQUS	\"db	28\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeNC EQUS	\"db	30\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeC EQUS	\"db	38\\ndb	\\1\"\n\n");
}

void findPossibleJumps(struct input_struct *input, struct jumps_struct *jumps)
{
	jumps->absolutes_index = 0;
	jumps->relatives_index = 0;
	jumps->absolutes_size = 1024;
	jumps->relatives_size = 1024;
	jumps->absolutes = malloc(sizeof(*jumps->absolutes) * jumps->absolutes_size);
	jumps->relatives = malloc(sizeof(*jumps->relatives) * jumps->relatives_size);
	if (!jumps->absolutes || !jumps->relatives)
	{
		printf("Failed to allocate memory for jump buffers.");
		exit(EXIT_FAILURE);
	}
	for (input->index = 0; input->index < input->size; input->index++)
	{
		unsigned char op = input->buffer[input->index];
		unsigned char a = 0;
		unsigned char b = 0;
		bool no_absolutes = FALSE;

		if (jumps->absolutes_size == jumps->absolutes_index)
		{
			jumps->absolutes_size += 1024;
			size_t *absolutes_temp = realloc(jumps->absolutes, sizeof(*jumps->absolutes) * jumps->absolutes_size);;
			if (!absolutes_temp)
			{
				printf("Failed to reallocate memory for absolutes jump buffer.");
				exit(EXIT_FAILURE);
			}
			jumps->absolutes = absolutes_temp;
		}
		if (jumps->relatives_size == jumps->relatives_index)
		{
			jumps->relatives_size += 1024;
			size_t *relatives_temp = realloc(jumps->relatives, sizeof(*jumps->relatives) * jumps->relatives_size);;
			if (!relatives_temp)
			{
				printf("Failed to reallocate memory for relatives jump buffer.");
				exit(EXIT_FAILURE);
			}
			jumps->relatives = relatives_temp;
		}

		if(input->index + 1 <= input->size)
		{
			a = input->buffer[input->index + 1];
		}
		else
		{
			break;
		}
		if (input->index + 2 <= input->size)
		{
			b = input->buffer[input->index + 2];
		}
		else
		{
			no_absolutes = TRUE;
		}

		size_t i;
		for (i = 0; i < AMT_ABS_JMPS; i++)
		{
			if (op == ABSOLUTE_JUMPS[i])
			{
				if (no_absolutes)
				{
					break;
				}
				jumps->absolutes[jumps->absolutes_index] = (unsigned short)b << 8;
				jumps->absolutes[jumps->absolutes_index] += a;
				if (jumps->absolutes[jumps->absolutes_index] >= ROMX_START)
				{
					jumps->absolutes[jumps->absolutes_index] += (input->index / 4000) * 4000;
				}
				jumps->absolutes_index++;
				input->index += 2;
				break;
			}
		}
		for (i = 0; i < AMT_REL_JMPS; i++)
		{
			if (op == RELATIVE_JUMPS[i])
			{
				jumps->relatives[jumps->relatives_index] = input->index;
				if (a < 128)
				{
					jumps->relatives[jumps->relatives_index] += a;
				}
				else
				{
					jumps->relatives[jumps->relatives_index] += -(a & 128) + (a & ~128);
				}
				jumps->relatives_index++;
				input->index++;
				break;
			}
		}
	}
	size_t *absolutes_temp = realloc(jumps->absolutes, sizeof(*jumps->absolutes) * (jumps->absolutes_index + 1));
	size_t *relatives_temp = realloc(jumps->relatives, sizeof(*jumps->relatives) * (jumps->relatives_index + 1));
	if (!absolutes_temp || !relatives_temp)
	{
		printf("Failed to reallocate memory for jump buffers.");
		exit(EXIT_FAILURE);
	}
	jumps->absolutes = absolutes_temp;
	jumps->relatives = relatives_temp;
	jumps->absolutes_size = jumps->absolutes_index;
	jumps->relatives_size = jumps->relatives_index;
	jumps->absolutes = sortUnsigned(jumps->absolutes, jumps->absolutes_size);
	jumps->absolutes = sortUnsigned(jumps->relatives, jumps->relatives_size);
	jumps->absolutes_index = 0;
	jumps->relatives_index = 0;
	return;
}

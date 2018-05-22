#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int bool;
#define FALSE		0
#define TRUE		!FALSE

#define VECTORS_START	0x0000
#define HEADER_START	0x0100
#define ROM0_START	0x0150
#define ROMX_START	0x4000 /*Coincidentally, also the multiple for every bank.*/

#define AMT_ONE_OPS		19
#define AMT_TWO_OPS		6
#define AMT_ABS_JMPS	10
#define AMT_REL_JMPS	5

const unsigned char ONE_OPERAND_OPS[AMT_ONE_OPS] = {0x06, 0x0E, 0x16, 0x26, 0x2E, 0x36, 0x3E, 0xC6, 0xCE, 0xD6, 0xDE, 0xE0, 0xE6, 0xE8, 0xEE, 0xF0, 0xF6, 0xF8, 0xFE};
const unsigned char TWO_OPERAND_OPS[AMT_TWO_OPS] = {0x01, 0x08, 0x11, 0x21, 0xEA, 0xFA};
const unsigned char ABSOLUTE_JUMPS[AMT_ABS_JMPS] = {0xC2, 0xC3, 0xC4, 0xCA, 0xCC, 0xCD, 0xD2, 0xD4, 0xDA, 0xDC};
const unsigned char RELATIVE_JUMPS[AMT_REL_JMPS] = {0x18, 0x20, 0x28, 0x30, 0x38};

struct input_struct
{
	unsigned char *buffer;
	size_t size;
	size_t index;
};

struct jumps_struct
{
	size_t *absolutes;
	size_t *relatives;
	size_t absolutes_index;
	size_t relatives_index;
	size_t absolutes_size;
	size_t relatives_size;
};

struct options_struct
{
	bool help;
	bool simple_mode;
	bool label_jumps;
};

size_t *shrinkArray(size_t *array, size_t size)
{
	size_t *array_temp = realloc(array, sizeof(*array_temp) * size);
	if (!array_temp)
	{
		printf("Failed to reallocate memory when shrinking array.\n");
		exit(EXIT_FAILURE);
	}
	array = array_temp;
	return array;
}

size_t deleteDuplicateValuesUnsigned(size_t *array, size_t size)
{
	size_t i;
	size_t j = 0;
	for (i = 0; i < size - 1; i++)
	{
		if (array[i] != array[i + 1])
		{
			array[j] = array[i];
			j++;
		}
	}
	array[j] = array[size - 1];
	j++;
	size = j;
	return size;
}

void quickSortUnsigned(size_t *array, size_t elements) /* Based off: http://alienryderflex.com/quicksort/ */
{
	size_t pivot;
	size_t beg[300];
	size_t end[300];
	size_t i = 0;
	size_t swap;
	beg[0] = 0;
	end[0] = elements;
	while(i <= 299)
	{
		size_t left = beg[i];
		size_t right = end[i] - 1;
		if(left < right && right < elements)
		{
			pivot = array[left];
			while(left < right)
			{
				while(array[right] >= pivot && left < right)
				{
					right--;
				}
				if(left < right)
				{
					array[left] = array[right];
					left++;
				}
				while(left < elements && array[left] <= pivot && left < right)
				{
					left++;
				}
				if(left < right)
				{
					array[right] = array[left];
					right--;
				}
			}
			array[left] = pivot;
			beg[i + 1] = left + 1;
			end[i + 1] = end[i];
			end[i] = left;
			i++;
			if(end[i] - beg[i] > end[i - 1] - beg[i - 1])
			{
				swap = beg[i];
				beg[i] = beg[i - 1];
				beg[i - 1] = swap;
				swap = end[i];
				end[i] = end[i - 1];
				end[i - 1] = swap;
			}
		}
		else
		{
			i--;
		}
	}
}

size_t *sortUnsigned(size_t *array, size_t elements)
{
	if (elements > 1)
	{
		quickSortUnsigned(array, elements);
		elements = deleteDuplicateValuesUnsigned(array, elements);
		array = shrinkArray(array, elements);
	}
	return array;
}

void findPossibleJumps(struct input_struct *input, struct jumps_struct *jumps)
{
	jumps->absolutes_index = 0;
	jumps->relatives_index = 0;
	jumps->absolutes_size = 1024;
	jumps->relatives_size = 1024;
	jumps->absolutes = malloc(sizeof(*jumps->absolutes) * jumps->absolutes_size);
	jumps->relatives = malloc(sizeof(*jumps->relatives) * jumps->relatives_size);
	if(!jumps->absolutes || !jumps->relatives)
	{
		printf("Failed to allocate memory for jump buffers.");
		exit(EXIT_FAILURE);
	}
	for(input->index = 0; input->index < input->size; input->index++)
	{
		unsigned char op = input->buffer[input->index];
		unsigned char a = 0;
		unsigned char b = 0;
		bool no_absolutes = FALSE;

		if(jumps->absolutes_size == jumps->absolutes_index)
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
		if(jumps->relatives_size == jumps->relatives_index)
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
		if(input->index + 2 <= input->size)
		{
			b = input->buffer[input->index + 2];
		}
		else
		{
			no_absolutes = TRUE;
		}

		size_t i;
		for(i = 0; i < AMT_ABS_JMPS; i++)
		{
			if(op == ABSOLUTE_JUMPS[i])
			{
				if(no_absolutes)
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
		for(i = 0; i < AMT_REL_JMPS; i++)
		{
			if(op == RELATIVE_JUMPS[i])
			{
				jumps->relatives[jumps->relatives_index] = input->index;
				if(a < 128)
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

size_t decodeOp(FILE *file_pointer, struct input_struct *input, struct options_struct *options, struct jumps_struct *jumps)
{
	size_t extra_increment = 0;
	unsigned char op = input->buffer[input->index];
	unsigned char a = input->buffer[input->index + 1];
	unsigned char b = input->buffer[input->index + 2];

	size_t op_check;
	for(op_check = 0; op_check < AMT_ONE_OPS; op_check++)
	{
		if(op == ONE_OPERAND_OPS[op_check])
		{
			extra_increment = 1;
			break;
		}
	}
	for(op_check = 0; op_check < AMT_TWO_OPS; op_check++)
	{
		if (op == TWO_OPERAND_OPS[op_check])
		{
			extra_increment = 2;
			break;
		}
	}
	extra_increment += 1;

	/*Check the address of every operand of the opcode and make sure we don't need a label there.
	If we do, just translate the hex values up until the label, place it, then decode that op.*/
	/*TODO: Not labeling correctly. Issue with how I derive values and with how I sort things.*/
	if(options->label_jumps)
	{
		size_t i;
		size_t j;
		for(op_check = 0; op_check < extra_increment; op_check++)
		{
			for(i = 0; i < jumps->absolutes_size; i++)
			{
				if(input->index + op_check == jumps->absolutes[i])
				{
					for(j = 0; j < op_check; j++)
					{
						fprintf(file_pointer, "	db $%02X			; $%04zX\n", input->buffer[input->index + j], input->index + j);
					}
					input->index += op_check;
					fprintf(file_pointer, "ROM%04zX:\n", input->index);
					jumps->absolutes_index++;
					break;
				}
				else if(input->index + op_check > jumps->absolutes[i])
				{
					break;
				}
			}
		}
		for (op_check = 0; op_check < extra_increment; op_check++)
		{
			for (i = 0; i < jumps->relatives_size; i++)
			{
				if (input->index + op_check == jumps->relatives[i])
				{
					for (j = 0; j < op_check; j++)
					{
						fprintf(file_pointer, "	db $%02X			; $%04zX", input->buffer[input->index + j], input->index + j);
					}
					input->index += op_check;
					fprintf(file_pointer, "ROM%04zX.\n", input->index);
					jumps->relatives_index++;
					break;
				}
				else if(input->index + op_check > jumps->relatives[i])
				{
				break;
				}
			}
		}
	}
	
	extra_increment = 0;

	/*Switch statement for all GB opcodes.*/
	switch(op)
	{
		case 0x00:
			fprintf(file_pointer, "	nop			");
			break;
		case 0x01:
			fprintf(file_pointer, "	ld	bc, $%02X%02X	", b, a);
			extra_increment += 2;
			break;
		case 0x02:
			fprintf(file_pointer, "	ld	[bc], a		");
			break;
		case 0x03:
			fprintf(file_pointer, "	inc	bc		");
			break;
		case 0x04:
			fprintf(file_pointer, "	inc	b		");
			break;
		case 0x05:
			fprintf(file_pointer, "	dec	b		");
			break;
		case 0x06:
			fprintf(file_pointer, "	ld	b, $%02X		", a);
			extra_increment++;
			break;
		case 0x07:
			fprintf(file_pointer, "	rlca			");
			break;
		case 0x08:
			fprintf(file_pointer, "	ld	[$%02X%02X], sp	", b, a);
			extra_increment += 2;
			break;
		case 0x09:
			fprintf(file_pointer, "	add	hl, bc		");
			break;
		case 0x0A:
			fprintf(file_pointer, "	ld	a, [bc]		");
			break;
		case 0x0B:
			fprintf(file_pointer, "	dec	bc		");
			break;
		case 0x0C:
			fprintf(file_pointer, "	inc	c		");
			break;
		case 0x0D:
			fprintf(file_pointer, "	dec	c		");
			break;
		case 0x0E:
			fprintf(file_pointer, "	ld	c, $%02X		", a);
			extra_increment++;
			break;
		case 0x0F:
			fprintf(file_pointer, "	rrca			");
			break;
		case 0x10:
			fprintf(file_pointer, "	stop	$%02X		", a);
			extra_increment++;
			break;
		case 0x11:
			fprintf(file_pointer, "	ld	de, $%02X%02X	", b, a);
			extra_increment += 2;
			break;
		case 0x12:
			fprintf(file_pointer, "	ld	[de], a		");
			break;
		case 0x13:
			fprintf(file_pointer, "	inc	de		");
			break;
		case 0x14:
			fprintf(file_pointer, "	inc	d		");
			break;
		case 0x15:
			fprintf(file_pointer, "	dec	d		");
			break;
		case 0x16:
			fprintf(file_pointer, "	ld	d, $%02X		", a);
			extra_increment++;
			break;
		case 0x17:
			fprintf(file_pointer, "	rla			");
			break;
		case 0x18:
			if(options->label_jumps)
			{
				size_t temp_value = input->index;
				if(a < 128)
				{
					temp_value += a;
				}
				else
				{
					temp_value += -(a & 128) + (a & ~128);
				}
				fprintf(file_pointer, "	jr	ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jumpRelative	$%02X	", a);
			}
			extra_increment++;
			break;
		case 0x19:
			fprintf(file_pointer, "	add	hl, de		");
			break;
		case 0x1A:
			fprintf(file_pointer, "	ld	a, [de]		");
			break;
		case 0x1B:
			fprintf(file_pointer, "	dec	de		");
			break;
		case 0x1C:
			fprintf(file_pointer, "	inc	e		");
			break;
		case 0x1D:
			fprintf(file_pointer, "	dec	e		");
			break;
		case 0x1E:
			fprintf(file_pointer, "	ld	e, $%02X		", a);
			extra_increment++;
			break;
		case 0x1F:
			fprintf(file_pointer, "	rra			");
			break;
		case 0x20:
			if(options->label_jumps)
			{
				size_t temp_value = input->index;
				if(a < 128)
				{
					temp_value += a;
				}
				else
				{
					temp_value += -(a & 128) + (a & ~128);
				}
				fprintf(file_pointer, "	jr	nz, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jumpRelativeNZ	$%02X	", a);
			}
			extra_increment++;
			break;
		case 0x21:
			fprintf(file_pointer, "	ld	hl, $%02X%02X	", b, a);
			extra_increment += 2;
			break;
		case 0x22:
			fprintf(file_pointer, "	ld	[hl+], a	");
			break;
		case 0x23:
			fprintf(file_pointer, "	inc	hl		");
			break;
		case 0x24:
			fprintf(file_pointer, "	inc	h		");
			break;
		case 0x25:
			fprintf(file_pointer, "	dec	h		");
			break;
		case 0x26:
			fprintf(file_pointer, "	ld	h, $%02X		", a);
			extra_increment++;
			break;
		case 0x27:
			fprintf(file_pointer, "	daa			");
			break;
		case 0x28:
			if(options->label_jumps)
			{
				size_t temp_value = input->index;
				if(a < 128)
				{
					temp_value += a;
				}
				else
				{
					temp_value += -(a & 128) + (a & ~128);
				}
				fprintf(file_pointer, "	jr	z, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jumpRelativeZ	$%02X	", a);
			}
			extra_increment++;
			break;
		case 0x29:
			fprintf(file_pointer, "	add	hl, hl		");
			break;
		case 0x2A:
			fprintf(file_pointer, "	ld	a, [hl+]	");
			break;
		case 0x2B:
			fprintf(file_pointer, "	dec	hl		");
			break;
		case 0x2C:
			fprintf(file_pointer, "	inc	l		");
			break;
		case 0x2D:
			fprintf(file_pointer, "	dec	l		");
			break;
		case 0x2E:
			fprintf(file_pointer, "	ld	l, $%02X		", a);
			extra_increment++;
			break;
		case 0x2F:
			fprintf(file_pointer, "	cpl			");
			break;
		case 0x30:
			if(options->label_jumps)
			{
				size_t temp_value = input->index;
				if(a < 128)
				{
					temp_value += a;
				}
				else
				{
					temp_value += -(a & 128) + (a & ~128);
				}
				fprintf(file_pointer, "	jr	nc, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jumpRelativeNC	$%02X	", a);
			}
			extra_increment++;
			break;
		case 0x31:
			fprintf(file_pointer, "	ld	sp, $%02X%02X	", b, a);
			extra_increment += 2;
			break;
		case 0x32:
			fprintf(file_pointer, "	ld	[hl-], a	");
			break;
		case 0x33:
			fprintf(file_pointer, "	inc	sp		");
			break;
		case 0x34:
			fprintf(file_pointer, "	inc	[hl]		");
			break;
		case 0x35:
			fprintf(file_pointer, "	dec	[hl]		");
			break;
		case 0x36:
			fprintf(file_pointer, "	ld	[hl], $%02X	", a);
			extra_increment++;
			break;
		case 0x37:
			fprintf(file_pointer, "	scf			");
			break;
		case 0x38:
			if(options->label_jumps)
			{
				size_t temp_value = input->index;
				if(a < 128)
				{
					temp_value += a;
				}
				else
				{
					temp_value += -(a & 128) + (a & ~128);
				}
				fprintf(file_pointer, "	jr	c, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jumpRelativeC	$%02X	", a);
			}
			extra_increment++;
			break;
		case 0x39:
			fprintf(file_pointer, "	add	hl, sp		");
			break;
		case 0x3A:
			fprintf(file_pointer, "	ld	a, [hl-]	");
			break;
		case 0x3B:
			fprintf(file_pointer, "	dec	sp		");
			break;
		case 0x3C:
			fprintf(file_pointer, "	inc	a		");
			break;
		case 0x3D:
			fprintf(file_pointer, "	dec	a		");
			break;
		case 0x3E:
			fprintf(file_pointer, "	ld	a, $%02X		", a);
			extra_increment++;
			break;
		case 0x3F:
			fprintf(file_pointer, "	ccf			");
			break;
		case 0x40:
			fprintf(file_pointer, "	ld	b, b		");
			break;
		case 0x41:
			fprintf(file_pointer, "	ld	b, c		");
			break;
		case 0x42:
			fprintf(file_pointer, "	ld	b, d		");
			break;
		case 0x43:
			fprintf(file_pointer, "	ld	b, e		");
			break;
		case 0x44:
			fprintf(file_pointer, "	ld	b, h		");
			break;
		case 0x45:
			fprintf(file_pointer, "	ld	b, l		");
			break;
		case 0x46:
			fprintf(file_pointer, "	ld	b, [hl]		");
			break;
		case 0x47:
			fprintf(file_pointer, "	ld	b, a		");
			break;
		case 0x48:
			fprintf(file_pointer, "	ld	c, b		");
			break;
		case 0x49:
			fprintf(file_pointer, "	ld	c, c		");
			break;
		case 0x4A:
			fprintf(file_pointer, "	ld	c, d		");
			break;
		case 0x4B:
			fprintf(file_pointer, "	ld	c, e		");
			break;
		case 0x4C:
			fprintf(file_pointer, "	ld	c, h		");
			break;
		case 0x4D:
			fprintf(file_pointer, "	ld	c, l		");
			break;
		case 0x4E:
			fprintf(file_pointer, "	ld	c, [hl]		");
			break;
		case 0x4F:
			fprintf(file_pointer, "	ld	c, a		");
			break;
		case 0x50:
			fprintf(file_pointer, "	ld	d, b		");
			break;
		case 0x51:
			fprintf(file_pointer, "	ld	d, c		");
			break;
		case 0x52:
			fprintf(file_pointer, "	ld	d, d		");
			break;
		case 0x53:
			fprintf(file_pointer, "	ld	d, e		");
			break;
		case 0x54:
			fprintf(file_pointer, "	ld	d, h		");
			break;
		case 0x55:
			fprintf(file_pointer, "	ld	d, l		");
			break;
		case 0x56:
			fprintf(file_pointer, "	ld	d, [hl]		");
			break;
		case 0x57:
			fprintf(file_pointer, "	ld	d, a		");
			break;
		case 0x58:
			fprintf(file_pointer, "	ld	e, b		");
			break;
		case 0x59:
			fprintf(file_pointer, "	ld	e, c		");
			break;
		case 0x5A:
			fprintf(file_pointer, "	ld	e, d		");
			break;
		case 0x5B:
			fprintf(file_pointer, "	ld	e, e		");
			break;
		case 0x5C:
			fprintf(file_pointer, "	ld	e, h		");
			break;
		case 0x5D:
			fprintf(file_pointer, "	ld	e, l		");
			break;
		case 0x5E:
			fprintf(file_pointer, "	ld	e, [hl]		");
			break;
		case 0x5F:
			fprintf(file_pointer, "	ld	e, a		");
			break;
		case 0x60:
			fprintf(file_pointer, "	ld	h, b		");
			break;
		case 0x61:
			fprintf(file_pointer, "	ld	h, c		");
			break;
		case 0x62:
			fprintf(file_pointer, "	ld	h, d		");
			break;
		case 0x63:
			fprintf(file_pointer, "	ld	h, e		");
			break;
		case 0x64:
			fprintf(file_pointer, "	ld	h, h		");
			break;
		case 0x65:
			fprintf(file_pointer, "	ld	h, l		");
			break;
		case 0x66:
			fprintf(file_pointer, "	ld	h, [hl]		");
			break;
		case 0x67:
			fprintf(file_pointer, "	ld	h, a		");
			break;
		case 0x68:
			fprintf(file_pointer, "	ld	l, b		");
			break;
		case 0x69:
			fprintf(file_pointer, "	ld	l, c		");
			break;
		case 0x6A:
			fprintf(file_pointer, "	ld	l, d		");
			break;
		case 0x6B:
			fprintf(file_pointer, "	ld	l, e		");
			break;
		case 0x6C:
			fprintf(file_pointer, "	ld	l, h		");
			break;
		case 0x6D:
			fprintf(file_pointer, "	ld	l, l		");
			break;
		case 0x6E:
			fprintf(file_pointer, "	ld	l, [hl]		");
			break;
		case 0x6F:
			fprintf(file_pointer, "	ld	l, a		");
			break;
		case 0x70:
			fprintf(file_pointer, "	ld	[hl], b		");
			break;
		case 0x71:
			fprintf(file_pointer, "	ld	[hl], c		");
			break;
		case 0x72:
			fprintf(file_pointer, "	ld	[hl], d		");
			break;
		case 0x73:
			fprintf(file_pointer, "	ld	[hl], e		");
			break;
		case 0x74:
			fprintf(file_pointer, "	ld	[hl], h		");
			break;
		case 0x75:
			fprintf(file_pointer, "	ld	[hl], l		");
			break;
		case 0x76:
			fprintf(file_pointer, "	halt			");
			break;
		case 0x77:
			fprintf(file_pointer, "	ld	[hl], a		");
			break;
		case 0x78:
			fprintf(file_pointer, "	ld	a, b		");
			break;
		case 0x79:
			fprintf(file_pointer, "	ld	a, c		");
			break;
		case 0x7A:
			fprintf(file_pointer, "	ld	a, d		");
			break;
		case 0x7B:
			fprintf(file_pointer, "	ld	a, e		");
			break;
		case 0x7C:
			fprintf(file_pointer, "	ld	a, h		");
			break;
		case 0x7D:
			fprintf(file_pointer, "	ld	a, l		");
			break;
		case 0x7E:
			fprintf(file_pointer, "	ld	a, [hl]		");
			break;
		case 0x7F:
			fprintf(file_pointer, "	ld	a, a		");
			break;
		case 0x80:
			fprintf(file_pointer, "	add	a, b		");
			break;
		case 0x81:
			fprintf(file_pointer, "	add	a, c		");
			break;
		case 0x82:
			fprintf(file_pointer, "	add	a, d		");
			break;
		case 0x83:
			fprintf(file_pointer, "	add	a, e		");
			break;
		case 0x84:
			fprintf(file_pointer, "	add	a, h		");
			break;
		case 0x85:
			fprintf(file_pointer, "	add	a, l		");
			break;
		case 0x86:
			fprintf(file_pointer, "	add	a, [hl]		");
			break;
		case 0x87:
			fprintf(file_pointer, "	add	a, a		");
			break;
		case 0x88:
			fprintf(file_pointer, "	adc	a, b		");
			break;
		case 0x89:
			fprintf(file_pointer, "	adc	a, c		");
			break;
		case 0x8A:
			fprintf(file_pointer, "	adc	a, d		");
			break;
		case 0x8B:
			fprintf(file_pointer, "	adc	a, e		");
			break;
		case 0x8C:
			fprintf(file_pointer, "	adc	a, h		");
			break;
		case 0x8D:
			fprintf(file_pointer, "	adc	a, l		");
			break;
		case 0x8E:
			fprintf(file_pointer, "	adc	a, [hl]		");
			break;
		case 0x8F:
			fprintf(file_pointer, "	adc	a, a		");
			break;
		case 0x90:
			fprintf(file_pointer, "	sub	b		");
			break;
		case 0x91:
			fprintf(file_pointer, "	sub	c		");
			break;
		case 0x92:
			fprintf(file_pointer, "	sub	d		");
			break;
		case 0x93:
			fprintf(file_pointer, "	sub	e		");
			break;
		case 0x94:
			fprintf(file_pointer, "	sub	h		");
			break;
		case 0x95:
			fprintf(file_pointer, "	sub	l		");
			break;
		case 0x96:
			fprintf(file_pointer, "	sub	[hl]		");
			break;
		case 0x97:
			fprintf(file_pointer, "	sub	a		");
			break;
		case 0x98:
			fprintf(file_pointer, "	sbc	a, b		");
			break;
		case 0x99:
			fprintf(file_pointer, "	sbc	a, c		");
			break;
		case 0x9A:
			fprintf(file_pointer, "	sbc	a, d		");
			break;
		case 0x9B:
			fprintf(file_pointer, "	sbc	a, e		");
			break;
		case 0x9C:
			fprintf(file_pointer, "	sbc	a, h		");
			break;
		case 0x9D:
			fprintf(file_pointer, "	sbc	a, l		");
			break;
		case 0x9E:
			fprintf(file_pointer, "	sbc	a, [hl]		");
			break;
		case 0x9F:
			fprintf(file_pointer, "	sbc	a, a		");
			break;
		case 0xA0:
			fprintf(file_pointer, "	and	b		");
			break;
		case 0xA1:
			fprintf(file_pointer, "	and	c		");
			break;
		case 0xA2:
			fprintf(file_pointer, "	and	d		");
			break;
		case 0xA3:
			fprintf(file_pointer, "	and	e		");
			break;
		case 0xA4:
			fprintf(file_pointer, "	and	h		");
			break;
		case 0xA5:
			fprintf(file_pointer, "	and	l		");
			break;
		case 0xA6:
			fprintf(file_pointer, "	and	[hl]		");
			break;
		case 0xA7:
			fprintf(file_pointer, "	and	a		");
			break;
		case 0xA8:
			fprintf(file_pointer, "	xor	b		");
			break;
		case 0xA9:
			fprintf(file_pointer, "	xor	c		");
			break;
		case 0xAA:
			fprintf(file_pointer, "	xor	d		");
			break;
		case 0xAB:
			fprintf(file_pointer, "	xor	e		");
			break;
		case 0xAC:
			fprintf(file_pointer, "	xor	h		");
			break;
		case 0xAD:
			fprintf(file_pointer, "	xor	l		");
			break;
		case 0xAE:
			fprintf(file_pointer, "	xor	[hl]		");
			break;
		case 0xAF:
			fprintf(file_pointer, "	xor	a		");
			break;
		case 0xB0:
			fprintf(file_pointer, "	or	b		");
			break;
		case 0xB1:
			fprintf(file_pointer, "	or	c		");
			break;
		case 0xB2:
			fprintf(file_pointer, "	or	d		");
			break;
		case 0xB3:
			fprintf(file_pointer, "	or	e		");
			break;
		case 0xB4:
			fprintf(file_pointer, "	or	h		");
			break;
		case 0xB5:
			fprintf(file_pointer, "	or	l		");
			break;
		case 0xB6:
			fprintf(file_pointer, "	or	[hl]		");
			break;
		case 0xB7:
			fprintf(file_pointer, "	or	a		");
			break;
		case 0xB8:
			fprintf(file_pointer, "	cp	b		");
			break;
		case 0xB9:
			fprintf(file_pointer, "	cp	c		");
			break;
		case 0xBA:
			fprintf(file_pointer, "	cp	d		");
			break;
		case 0xBB:
			fprintf(file_pointer, "	cp	e		");
			break;
		case 0xBC:
			fprintf(file_pointer, "	cp	h		");
			break;
		case 0xBD:
			fprintf(file_pointer, "	cp	l		");
			break;
		case 0xBE:
			fprintf(file_pointer, "	cp	[hl]		");
			break;
		case 0xBF:
			fprintf(file_pointer, "	cp	a		");
			break;
		case 0xC0:
			fprintf(file_pointer, "	ret	nz		");
			break;
		case 0xC1:
			fprintf(file_pointer, "	pop	bc		");
			break;
		case 0xC2:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	jp	nz, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jp	nz, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xC3:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	jp	ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jp	$%02X%02X		", b, a);
			}
			extra_increment += 2;
			break;
		case 0xC4:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	call	nz, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	call	nz, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xC5:
			fprintf(file_pointer, "	push	bc		");
			break;
		case 0xC6:
			fprintf(file_pointer, "	add	a, $%02X		", a);
			extra_increment++;
			break;
		case 0xC7:
			fprintf(file_pointer, "	rst	$00		");
			break;
		case 0xC8:
			fprintf(file_pointer, "	ret	z		");
			break;
		case 0xC9:
			fprintf(file_pointer, "	ret			");
			break;
		case 0xCA:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	jp	z, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jp	z, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xCB: /*TODO: Fill this out properly.*/
			extra_increment++;
			switch(a)
			{
				case 0x00:
					fprintf(file_pointer, "rlc	b			");
					break;
				case 0x01:
					fprintf(file_pointer, "rlc	c			");
					break;
			}
			break;
		case 0xCC:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	call	z, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	call	z, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xCD:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	call	ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	call	$%02X%02X		", b, a);
			}
			extra_increment += 2;
			break;
		case 0xCE:
			fprintf(file_pointer, "	adc	a, $%02X		", a);
			extra_increment++;
			break;
		case 0xCF:
			fprintf(file_pointer, "	rst	$08		");
			break;
		case 0xD0:
			fprintf(file_pointer, "	ret	nc		");
			break;
		case 0xD1:
			fprintf(file_pointer, "	pop	de		");
			break;
		case 0xD2:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	jp	nc, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jp	nc, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xD4:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	call	nc, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	call	nc, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xD5:
			fprintf(file_pointer, "	push	de		");
			break;
		case 0xD6:
			fprintf(file_pointer, "	sub	$%02X		", a);
			extra_increment++;
			break;
		case 0xD7:
			fprintf(file_pointer, "	rst	$10		");
			break;
		case 0xD8:
			fprintf(file_pointer, "	ret	c		");
			break;
		case 0xD9:
			fprintf(file_pointer, "	reti			");
			break;
		case 0xDA:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	jp	c, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	jp	c, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xDC:
			if(options->label_jumps)
			{
				size_t temp_value;
				temp_value = (unsigned short)b << 8;
				temp_value += a;
				if (temp_value >= ROMX_START)
				{
					temp_value += (input->index / 4000) * 4000;
				}
				fprintf(file_pointer, "	call	c, ROM%04zX	", temp_value);
			}
			else
			{
				fprintf(file_pointer, "	call	c, $%02X%02X	", b, a);
			}
			extra_increment += 2;
			break;
		case 0xDE:
			fprintf(file_pointer, "	sbc	a, $%02X		", a);
			extra_increment++;
			break;
		case 0xDF:
			fprintf(file_pointer, "	rst	$18		");
			break;
		case 0xE0:
			fprintf(file_pointer, "	ldh	[$FF00+$%02X], a	", a);
			extra_increment++;
			break;
		case 0xE1:
			fprintf(file_pointer, "	pop	hl		");
			break;
		case 0xE2:
			fprintf(file_pointer, "	ld	[c], a		");
			break;
		case 0xE5:
			fprintf(file_pointer, "	push	hl		");
			break;
		case 0xE6:
			fprintf(file_pointer, "	and	$%02X		", a);
			extra_increment++;
			break;
		case 0xE7:
			fprintf(file_pointer, "	rst	$20		");
			break;
		case 0xE8:
			fprintf(file_pointer, "	add	sp, $%02X		", a);
			extra_increment++;
			break;
		case 0xE9:
			fprintf(file_pointer, "	jp	hl		");
			break;
		case 0xEA:
			fprintf(file_pointer, "	ld	[$%02X%02X], a	", b, a);
			extra_increment += 2;
			break;
		case 0xEE:
			fprintf(file_pointer, "	xor	$%02X		", a);
			extra_increment++;
			break;
		case 0xEF:
			fprintf(file_pointer, "	rst	$28		");
			break;
		case 0xF0:
			fprintf(file_pointer, "	ldh	a, [$FF00+$%02X]	", a);
			extra_increment++;
			break;
		case 0xF1:
			fprintf(file_pointer, "	pop	af		");
			break;
		case 0xF2:
			fprintf(file_pointer, "	ld	a, [c]		");
			break;
		case 0xF3:
			fprintf(file_pointer, "	di			");
			break;
		case 0xF5:
			fprintf(file_pointer, "	push	af		");
			break;
		case 0xF6:
			fprintf(file_pointer, "	or	$%02X		", a);
			extra_increment++;
			break;
		case 0xF7:
			fprintf(file_pointer, "	rst	$30		");
			break;
		case 0xF8:
			fprintf(file_pointer, "	ld	hl, sp+$%02X	", a);
			extra_increment++;
			break;
		case 0xF9:
			fprintf(file_pointer, "	ld	sp, hl		");
			break;
		case 0xFA:
			fprintf(file_pointer, "	ld	a, [$%02X%02X]	", b, a);
			extra_increment += 2;
			break;
		case 0xFB:
			fprintf(file_pointer, "	ei			");
			break;
		case 0xFE:
			fprintf(file_pointer, "	cp	$%02X		", a);
			extra_increment++;
			break;
		case 0xFF:
			fprintf(file_pointer, "	rst	$38		");
			break;
		default:
			fprintf(file_pointer, "	db	$%02X		", op);
	}
	fprintf(file_pointer, "	; %04zX\n", input->index);
	return extra_increment;
}

void saveRelativeJumpMacros(FILE *file_pointer) /*We need these macros, becase RGBDS doesn't properly support relative jumps with pure hex values, only labels.*/
{
	fprintf(file_pointer, "jumpRelative EQUS	\"db	18\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeNZ EQUS	\"db	20\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeZ EQUS	\"db	28\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeNC EQUS	\"db	30\\ndb	\\1\"\n");
	fprintf(file_pointer, "jumpRelativeC EQUS	\"db	38\\ndb	\\1\"\n\n");
}

void simpleDump(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options)
{
	printf("Starting dump...\n");
	fprintf(file_pointer, "SECTION \"Start\"\n");
	for(input->index = VECTORS_START; input->index < input->size; input->index++)
	{
		input->index += decodeOp(file_pointer, input, options, jumps);
	}
	return;
}

void vectorInterruptDump(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options)
{
	printf("Starting vector/interrupt dump...\n");
	fprintf(file_pointer,  "SECTION \"Vectors and interrupts\", ROM0[$%04X]\n", VECTORS_START);
	for(input->index = VECTORS_START; (input->index < HEADER_START && input->index < input->size); input->index++)
	{
		input->index += decodeOp(file_pointer, input, options, jumps);
	}
	fprintf(file_pointer, "\n");
	return;
}

void headerDump(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options)
{
	input->index = HEADER_START;
	printf("Starting header dump...\n");
	fprintf(file_pointer, "SECTION \"Header\", ROM0[$%04X]\n", HEADER_START);
	fprintf(file_pointer, "; Entrypoint:\n");
	for(input->index = 0x100; input->index < 0x104; input->index++)
	{
		input->index += decodeOp(file_pointer, input, options, jumps);
	}
	
	fprintf(file_pointer, "; Scrolling Nintendo graphic:\n");

	unsigned char i;
	unsigned char j;
	for(i = 0; i < 3; i++)
	{
		fprintf(file_pointer, "	db	");
		for(j = 0; j < 15; j++, input->index++)
		{
			fprintf(file_pointer, "$%02X,", input->buffer[input->index]);
		}
		fprintf(file_pointer, "$%02X\n", input->buffer[input->index]);
		input->index++;
	}
	
	fprintf(file_pointer, "	db	\"");
	for(i = 0; i < 15; i++, input->index++)
	{
		fprintf(file_pointer, "%c", input->buffer[input->index]);
	}
	fprintf(file_pointer, "\" ; Game Title\n");
	
	fprintf(file_pointer, "	db	$%02X ; GBC = $80\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X,", input->buffer[input->index]);
	input->index++;
	fprintf(file_pointer, "$%02X", input->buffer[input->index]);
	fprintf(file_pointer, " ; Licensee\n");
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; SGB = 03\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; Cartridge type\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; ROM size\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; RAM size\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; Destination code\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; Licensee code (old)\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; Mask ROM Version\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X ; Header complement check\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "	db	$%02X,", input->buffer[input->index]);
	input->index++;
	fprintf(file_pointer, "%02X ; Checksum\n", input->buffer[input->index]);
	input->index++;
	
	fprintf(file_pointer, "\n");
	return;
}

void ROM0Dump(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options)
{
	/*
	TODO:
	Print jumps as I'm disassembling. Reuse ROMX code?
	*/
	size_t ROM0_end = 0x4000;
	printf("Starting ROM0 dump...\n");
	fprintf(file_pointer, "SECTION \"ROM0 Start\", ROM0[$%04X]\n", ROM0_START);
	if(input->size <= 0x8000)
	{
		ROM0_end = input->size;
	}
	for(input->index = ROM0_START; input->index < ROM0_end; input->index++)
	{
		input->index += decodeOp(file_pointer, input, options, jumps);
	}
	return;
}

void ROMXDump(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options)
{
	/*
	TODO:
	Print jumps as I'm disassembling. Reuse ROM0 code?
	*/
	input->index = ROMX_START;
	while(input->index < input->size)
	{
		size_t address_start = (ROMX_START * ((input->index / ROMX_START)));
		size_t address_end = (ROMX_START * ((input->index / ROMX_START) + 1));
		size_t bank = ((input->index / ROMX_START) - 1);
		printf("Starting ROMX bank[$%02zX] dump...\n", bank);
		fprintf(file_pointer, "SECTION \"ROMX Bank[%02zX] Start\", ROMX[$%04zX]\n", bank, address_start);
		for(input->index = address_start; (input->index < address_end && input->index < input->size); input->index++)
		{
			input->index += decodeOp(file_pointer, input, options, jumps);
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	struct input_struct input;
	struct jumps_struct jumps;
	struct options_struct options = {FALSE};

	/*Get command line options.*/
	int i;
	for(i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		switch(argv[i][1])
		{
			case 'h':
				options.help = TRUE;
				break;
			case 'j':
				options.label_jumps = TRUE;
				break;
			case 's':
				options.simple_mode = TRUE;
				break;
		}
	}

	/*Check if we're flagged to display help, if there aren't enough arguments, and if the last two arguments aren't options.*/
	if(options.help || argc < 3 || argv[argc - 1][0] == '-' || argv[argc - 2][0] == '-')
	{
		printf("Your command:\n");
		for(i = 0; i < argc; i++)
		{
			printf("%s ", argv[i]);
		}
		printf("\n\n");
		printf("Use the following format:\n");
		printf("[filename] [flags] [input] [output]\n\n");
		printf("Flags:\n");
		printf("[-h]elp: Display this help message. Also displayed when there are less than two arguments for the program, or if either of the last two arguments are flags.\n");
		printf("[-j]umps: Label jumps instead of using pure hex values.\n");
		printf("[-s]imple: Dump the header as if it was regular code. If size is less that 0x150, this is enabled automatically.\n");
		printf("\n");
		printf("Examples:\n");
		printf("%s \"Tetris.gb\" \"Tetris.asm\"\n", argv[0]);
		printf("%s \"Tetris.gb\" \"Tetris.asm\"\n", argv[0]);
		printf("%s -j \"Pokemon Silver.gbc\" \"Pokemon Silver.asm\"\n", argv[0]);
		printf("%s -s -j \"Random Code.gb\" \"Random Code.asm\"\n", argv[0]);
		return EXIT_FAILURE;
	}

	/*Read binary input file into memory.*/
	FILE *file_pointer = fopen(argv[argc - 2], "rb");
	if(!file_pointer)
	{
		printf("Failed to open file: %s.\n", argv[argc - 2]);
		return EXIT_FAILURE;
	}
	fseek(file_pointer, 0, SEEK_END);
	input.size = ftell(file_pointer);
	rewind(file_pointer);
	input.buffer = malloc(sizeof(*input.buffer) * input.size);
	if(!input.buffer)
	{
		exit(EXIT_FAILURE);
	}
	fread(input.buffer, input.size, 1, file_pointer);
	fclose(file_pointer);

	/*Open assembly output file.*/
	file_pointer = fopen(argv[argc - 1], "wb");
	if(!file_pointer)
	{
		printf("Failed to open file: %s.\n", argv[argc - 1]);
		exit(EXIT_FAILURE);
	}

	/*Label jumps or use pure hex values?*/
	if(options.label_jumps)
	{
		findPossibleJumps(&input, &jumps);
	}
	else
	{
		saveRelativeJumpMacros(file_pointer);
	}

	/*Simple or normal dump?*/
	if(options.simple_mode || input.size < 0x150)
	{
		simpleDump(file_pointer, &input, &jumps, &options);
	}
	else
	{
		vectorInterruptDump(file_pointer, &input, &jumps, &options);
		headerDump(file_pointer, &input, &jumps, &options);
		ROM0Dump(file_pointer, &input, &jumps, &options);
		if(input.size > 0x8000)
		{
			ROMXDump(file_pointer, &input, &jumps, &options);
		}
	}

	/*Exit.*/
	fprintf(file_pointer, "; End of output.\n");
	fclose(file_pointer);
	free(input.buffer);
	printf("Done.\n");
	system("pause");
	return EXIT_SUCCESS;
}

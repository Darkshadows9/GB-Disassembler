#ifndef DISASSEMBLE_INC
#define DISASSEMBLE_INC

#define VECTORS_START	0x0000
#define HEADER_START	0x0100
#define ROM0_START	0x0150
#define ROMX_START	0x4000 /*Coincidentally, also the multiple for every bank.*/

void simpleDisassemble(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options);
void complexDisassemble(FILE *file_pointer, struct input_struct *input, struct jumps_struct *jumps, struct options_struct *options);

#endif /*DISASSEMBLE_INC*/

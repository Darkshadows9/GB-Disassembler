#ifndef JUMP_INC
#define JUMP_INC

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

#endif /*JUMP_INC*/

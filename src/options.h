#ifndef OPTIONS_INC
#define OPTIONS_INC

#include "bool.h"

struct options_struct
{
	bool help;
	bool simple_mode;
	bool label_jumps;
};

void parseTerminalOptions(int argc, char *argv[], struct options_struct *options);
void printHelp();

#endif /*OPTIONS_INC*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "input.h"
#include "options.h"
#include "jumps.h"
#include "disassemble.h"

int main(int argc, char *argv[])
{
	struct input_struct input;
	struct jumps_struct jumps;
	struct options_struct options = {FALSE};

	/*Get command line options.*/
	parseTerminalOptions(argc, argv, &options);

	/*Check if we're flagged to display help, if there aren't enough arguments, and if the last two arguments aren't options.*/
	if(options.help || argc < 3 || argv[argc - 1][0] == '-' || argv[argc - 2][0] == '-')
	{
		printHelp(argc, argv);
		return EXIT_FAILURE;
	}

	/*Read binary input file into memory.*/
	loadInput(argc, argv, &input);

	/*Open assembly output file.*/
	FILE *file_pointer = fopen(argv[argc - 1], "wb");
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
		simpleDisassemble(file_pointer, &input, &jumps, &options);
	}
	else
	{
		complexDisassemble(file_pointer, &input, &jumps, &options);
	}

	/*Exit.*/
	fprintf(file_pointer, "; End of output.\n");
	fclose(file_pointer);
	free(input.buffer);
	printf("Done.\n");
	system("pause");
	return EXIT_SUCCESS;
}

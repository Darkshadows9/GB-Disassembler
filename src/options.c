#include <stdio.h>
#include <stdlib.h>

#include "bool.h"
#include "options.h"

void parseTerminalOptions(int argc, char *argv[], struct options_struct *options)
{
	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		switch (argv[i][1])
		{
		case 'h':
			options->help = TRUE;
			break;
		case 'j':
			options->label_jumps = TRUE;
			break;
		case 's':
			options->simple_mode = TRUE;
			break;
		}
	}
	return;
}

void printHelp(int argc, char *argv[])
{
	printf("Your command:\n");
	int i;
	for (i = 0; i < argc; i++)
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
	return;
}

# Gameboy Disassembler
This repository contains a dissassembler for the Gameboy's processor, the Sharp LR35902.

The disassembler's output contains Sharp LR35902 assembly capable of being assembled and linked by the RGBDS development suite.

Compile Disassembler.c using Make.bat.

This is the output of the program's -h flag:
Your command:
%s

Use the following format:
[filename] [flags] [input] [output]
Flags:
[-h]elp: Display this help message. Also displayed when there are less than two arguments for the program, or if either of the last two arguments are flags.
[-j]umps: Label jumps instead of using pure hex values.
[-s]imple: Dump the header as if it was regular code. If size is less that 0x150, this is enabled automatically.

Examples:
%[program name] "Tetris.gb" "Tetris.asm"
%[program name] "Tetris.gb" "Tetris.asm"
%[program name] -j "Pokemon Silver.gbc" "Pokemon Silver.asm"
%[program name] -s -j "Random Code.gb" "Random Code.asm"

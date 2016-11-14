## Synopsis

This program reads MIPS machine instructions from a (simplified) binary executable file and prints each assembly language instructionto the screen using the following steps:

1) The MIPS simplified binary file contains 32 bit machine instructions

2) Using bitwise operations, the 32 bit binary field can be decoded to gather the MIPS syntax.


## Example

The program will disassemble the test1.mips binary file and print out the machine instructions in that file like so:

MIPS Disassembler
400000: sll $t1, $zero, 2
400004: unimplemented
400008: lw $t0, 0($t1)
40000c: bne $t0, $s5, 400018
400010: unimplemented
400014: j 400000

To view the output of each MIPS binary file, please refer to the .dis files provided in the src directory to view the output of each MIPS binary file. 

## Compile and Run 

g++ -O3 disassembler.cpp -o disassembler -std=c++11

Run with one of the provided mips binary file 

./disassembler *.mips

## Contributors

Department of Computer Science at Texas State
CS 3339 - Spring 2016

Edgar Jaimes

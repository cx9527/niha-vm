/*
 *	Header file of virtual machine execution
 *	
 *	Dad`
 */

#ifndef __VIRTUALMACHINE_H__
#define __VIRTUALMACHINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>

#include "tools.h"

#define OPCODES_SIZE 12
#define MAX_STRINGS 10

extern u_char MAGIC_BYTES[4];

typedef struct opcode_func
{
	u_char opcode;
	void (*func)(void *);
} opcode_func;

typedef struct processor_t
{
	/* EIP */
	u_char *current;

	/* EBP */
	int base;
	int regA;
	int regB;
	int regC;

	/* Flags
	 *	CompareFlags
	 */
	u_char flags;

	/* Stack */
	u_char *memory;

	/* List of opcodes */
	opcode_func opcodes[OPCODES_SIZE];

	/* List of strings */
	chunk_t strings[MAX_STRINGS];
	int stringslen;
} processor_t;

#endif

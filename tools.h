/*
 *	Usefull functions
 */
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>

typedef unsigned char u_char;

typedef struct chunk_t
{
	u_char *ptr;
	size_t len;
} chunk_t;

typedef struct mnemonic_t
{
	/* Mnemonic's name (e.g SETA) */
	char *name;
	/* Associated function taking regA, regB and regC as parameters */
	void (*function)(chunk_t *, char *, char* , char*);
} mnemonic_t;

void logging(char *fmt __attribute__((unused)), ...);
int isValidMagic(u_char *bytecode);
unsigned int safeRead(u_char *positionStart, u_char *positionEnd, char **word);

#endif

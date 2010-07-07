/*
 *	VM compilator header
 *
 *	Dad
 */
#ifndef __COMPILO_H__
#define __COMPILO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "tools.h"

extern const u_char MAGIC_BYTES[4];
extern const mnemonic_t MNEMONICS[];

unsigned int MNEMONIC_SIZE = 7;

void mnemonicSetA(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicSetB(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicSetC(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicStrA(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicGetS(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicPutT(chunk_t *code, char *regA, char *regB, char *regC);
void mnemonicStrC(chunk_t *code, char *regA, char *regB, char *regC);

#endif

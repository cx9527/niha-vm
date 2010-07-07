/*
 *	VM compilator!
 *
 *	Dad
 */
#include "compilo.h"

const mnemonic_t MNEMONICS[] =
{ { "SETA", &mnemonicSetA }, 
  { "SETB", &mnemonicSetB },
  { "SETC", &mnemonicSetC },
  { "STRA", &mnemonicStrA },
  { "GETS", &mnemonicGetS },
  { "PUTT", &mnemonicPutT },
  { "STRC", &mnemonicStrC }
};
/*
 *	Main usage of this compilator
 */
void usage(char *s)
{
	logging("Usage: %s <source code>\n", s);
}

/*
 *	SETA definition
 */
void mnemonicSetA(chunk_t *code, char *regA, 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
		realloc(code->ptr, sizeof(u_char)*(code->len+1+sizeof(int)))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in SETA\n");
	}

	code->ptr[code->len] = 0xF0;
	*((unsigned int *) (code->ptr + code->len + 1)) = strtoul(regA, NULL, 0);
	code->len += 1 + sizeof(int);
}

/*
 *	SETB definition
 */
void mnemonicSetB(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB,
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
		realloc(code->ptr, sizeof(u_char)*(code->len+1+sizeof(int)))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in SETB\n");
	}

	code->ptr[code->len] = 0xF1;
	*((unsigned int *) (code->ptr + code->len + 1)) = strtoul(regB, NULL, 0);
	code->len += 1 + sizeof(int);
}

/*
 *	SETC definition
 */
void mnemonicSetC(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB __attribute__((unused)),
								char *regC)
{
	if ((code->ptr =
		realloc(code->ptr, sizeof(u_char)*(code->len+1+sizeof(int)))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in SETC\n");
	}

	code->ptr[code->len] = 0xF2;
	*((unsigned int *) (code->ptr + code->len + 1)) = strtoul(regC, NULL, 0);
	code->len += 1 + sizeof(int);
}

/*
 * STRA for String Allocation with buffer pushed in bytecode
 */
void mnemonicStrA(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
			realloc(code->ptr,
			   	sizeof(u_char)*(code->len+1+sizeof(int)+strlen(regA)+1)))
					== NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in STRA\n");
	}

	code->ptr[code->len] = 0xE0;
	*((unsigned int *) (code->ptr + code->len + 1)) = strlen(regA) + 1;
	memcpy(code->ptr + code->len + 1 + sizeof(int), regA, strlen(regA) + 1);
	code->len += 1 + sizeof(int) + strlen(regA) + 1;
}

/*
 * GETS to get reference of string number
 */
void mnemonicGetS(chunk_t *code, char *regA, 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
			realloc(code->ptr,
			   	sizeof(u_char)*(code->len+1))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in GETS\n");
	}

	mnemonicSetA(code, regA, regB, regC);
	code->ptr[code->len] = 0xE1;
	code->len += 1;
}


/*
 * PUTT print current context
 */
void mnemonicPutT(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
			realloc(code->ptr,
			   	sizeof(u_char)*(code->len+1))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in PUTT\n");
	}

	code->ptr[code->len] = 0xF6;
	code->len += 1;
}

/*
 * STRC Allocating a yencoded string
 */
void mnemonicStrC(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	u_char i;

	for (i = 0; i < strlen(regA); i++)
	{
		*(regA + i) -= 42;
	}

	mnemonicStrA(code, regA, regB, regC);
}


/*
 * PUTT print current context
 
void mnemonicPutT(chunk_t *code, char *regA __attribute__((unused)), 
								char *regB __attribute__((unused)),
								char *regC __attribute__((unused)))
{
	if ((code->ptr =
			realloc(code->ptr,
			   	sizeof(u_char)*(code->len+1))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in PUTT\n");
	}

	code->ptr[code->len] = 0xE2;
	code->len += 1;
}*/

/*
 *	Mnemonic parsing
 */
unsigned int readMnemonic(char **mnemonic, 
							char **regA, char **regB, char **regC,
							chunk_t sourcecode, unsigned int index)
{
	u_char *currentPosition, *endPosition;

	currentPosition = sourcecode.ptr + index;
	endPosition = sourcecode.ptr + sourcecode.len;

	logging("[+] Read mnemonics at index %u: %s\n", index, currentPosition);

	currentPosition+= safeRead(currentPosition, endPosition, mnemonic);
	currentPosition+= safeRead(currentPosition, endPosition, regA);
	currentPosition+= safeRead(currentPosition, endPosition, regB);
	currentPosition+= safeRead(currentPosition, endPosition, regC);

	return currentPosition - sourcecode.ptr - index;
}

/*
 *	Code generation
 */
void generateSourcecode(chunk_t sourceCode, chunk_t *generatedCode)
{
	unsigned int index, position;
	char *mnemonic, *regA, *regB, *regC;
	bool mnemonicFound;

	index = 0;
	mnemonic = NULL;
	regA = NULL;
	regB = NULL;
	regC = NULL;

	while (index < sourceCode.len)
	{
		index+= readMnemonic(&mnemonic, &regA, &regB, &regC, sourceCode, index);

		/* Safe ? :) */
		if (mnemonic == NULL) break;

		/* Do Stuff */
		mnemonicFound = false;
		position = 0;
		
		while ((position < MNEMONIC_SIZE) && (!mnemonicFound))
		{
			logging("[+] Comparing <%s:%s> (index:%u)\n"
					, mnemonic
					, MNEMONICS[position].name
					, index);

			/* Should add maximun mnemonic length with strncmp */
			if (!strcmp(mnemonic, MNEMONICS[position].name))
			{
				MNEMONICS[position].function(generatedCode, regA, regB, regC);
				mnemonicFound = true;
			}

			position++;
		}

		/* Free allocated memory */
		free(mnemonic);
		free(regA);
		free(regB);
		free(regC);
		mnemonic = NULL;
		regA = NULL;
		regB = NULL;
		regC = NULL;
	}

	/* Pushing 0xFF indicating the end of bytecode */
	logging("[+] Pushing 0xFF\n");
	if ((generatedCode->ptr =
		realloc(generatedCode->ptr, 
				sizeof(u_char)*(generatedCode->len+1))) == NULL)
	{
		logging("[+] Realloc returned NULL for code->ptr allocation in SETA\n");
	}

	generatedCode->ptr[generatedCode->len] = 0xFF;
	generatedCode->len++;
}

/*
 *	Main routine for source compilation
 */
int main(int argc, char *argv[])
{
	FILE *fileSourcecode, *fileGeneratedcode;
	char *strGeneratedcode;
	struct stat fileStats;
	u_char *sourcecode;
	size_t resultSize;
	chunk_t sourcecodeChunk, generatedCode;

	if (argc < 2)
	{
		usage(argv[0]);
		return 0;
	}

	if ((fileSourcecode = fopen(argv[1], "rb+")) == NULL)
	{
		logging("[-] Unable to open file %s\n", argv[1]);
		return 1;
	}

	if (!(stat(argv[1], &fileStats) == 0))
	{
		logging("[-] Unable to stat file %s\n", argv[1]);
		return 1;
	}

	logging("[+] Allocating %lu bytes for source code\n"
					, (long unsigned int) fileStats.st_size);

	if ((sourcecode = malloc(sizeof(u_char)*fileStats.st_size)) == NULL)
	{
		logging("[-] Failed to allocate space for source code\n");
		return 1;
	}

	resultSize = fread(sourcecode, 1, fileStats.st_size, fileSourcecode);
	logging("[+] Copied %zu bytes for source code\n"
					, resultSize);

	sourcecodeChunk.ptr = sourcecode;
	sourcecodeChunk.len = resultSize;

	logging("[+] Generating sourcecode at 0x%08X\n"
					, (int) sourcecode);

	generatedCode.ptr = NULL;
	generatedCode.len = 0;

	generateSourcecode(sourcecodeChunk, &generatedCode);

	strGeneratedcode = (char*) malloc(sizeof(char)*(strlen(argv[1]) + 4));
	strncpy(strGeneratedcode, argv[1], strlen(argv[1]));
	strncat(strGeneratedcode, ".bc\0", 3);

	logging("[+] Generated code is %u byte(s) long\n", generatedCode.len);
	logging("[+] Opening file %s\n", strGeneratedcode);

	if ((fileGeneratedcode = fopen(strGeneratedcode, "wb+")) == NULL)
	{
		logging("[-] Unable to open file %s\n", strGeneratedcode);
		return 1;
	}

	logging("[+] Writing to file %s\n", strGeneratedcode);
	fwrite(MAGIC_BYTES, 1, sizeof(MAGIC_BYTES), fileGeneratedcode);
	fwrite(generatedCode.ptr, 1, generatedCode.len, fileGeneratedcode);

	logging("[+] Freeing allocated memory\n");

	/* Should make a destroyChunk() */
	free(sourcecode);
	free(strGeneratedcode);
	free(generatedCode.ptr);
	sourcecode = NULL;
	strGeneratedcode = NULL;
	generatedCode.ptr = NULL;

	fclose(fileSourcecode);
	fclose(fileGeneratedcode);

	return 0;
}

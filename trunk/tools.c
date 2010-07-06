/*
 *	Tools sources
 */
#include "tools.h"

const u_char MAGIC_BYTES[] = { 0x21, 0x45, 0x4c, 0x46 };

void logging(char *fmt __attribute__((unused)), ...)
{
	va_list arg;
	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

int isValidMagic(u_char *bytecode)
{
	u_char i;
	for ( i=0; i<(sizeof(MAGIC_BYTES)/sizeof(u_char)); i++)
	{
		if (bytecode[i] != MAGIC_BYTES[i]) return 0;
	}

	return 1;
}

unsigned int safeRead(u_char *positionStart, char **word)
{
	unsigned int bufSize, position;

	bufSize = 0;
	position = 0;

	/* Remove whitespaces */
	while ((*(positionStart++)) == ' ');
	positionStart--;

	while (((*(positionStart + position)) != ' ') &&
		   ((*(positionStart + position)) != '\n'))
	{
		if (position >= bufSize)
		{
			bufSize+= 5;
			if ((*word = realloc(*word, sizeof(u_char)*(bufSize))) == NULL)
			{
				logging("[-] Realloc returned NULL for mnemonic\n");
			}
		}

		logging("[+] Reading character @%u %c\n"
					, position
					, *(positionStart + position));
		(*word)[position] = *(positionStart + position);
		position++;
	}

	/* No need to realloc since position < bufSize */
	if (*word)
	{
		(*word)[position] = '\0';
	}

	logging("[+] Successfully parsed word: %s\n", *word);

	return position;
}

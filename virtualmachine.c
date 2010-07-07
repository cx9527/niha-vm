/*
 *	Code of virtual machine execution
 *	
 *	Dad`
 */

#include "virtualmachine.h"

/*
 *	Print default usage of program
 */

void usage(char *s)
{
	logging("Usage: %s <bytecode>\n", s);
}

void printProc(processor_t proc)
{
	logging("EIP :\n");
	logging("\tcurrent : \t%08X\n", (unsigned int)proc.current);
	logging("Flags :\n");
	logging("\tComp : \t%u (%02X)\n"
					, (proc.flags & 1<<7) != 0, (proc.flags & 1<<7) != 0);
	logging("Registers :\n");
	logging("\tA : \t%u (%08X)\n", proc.regA, proc.regA);
	logging("\tB : \t%u (%08X)\n", proc.regB, proc.regB);
	logging("\tC : \t%u (%08X)\n", proc.regC, proc.regC);
}

void setCompareFlag(processor_t *proc, int boolean)
{
	proc->flags |= boolean<<7;
}

int getCompareFlag(processor_t proc)
{
	return (proc.flags & 1<<7);
}

void funcseta(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->regA = *(int*) (proc->current + 1);
	proc->current += sizeof(int);
}

void funcsetb(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->regB = *(int*) (proc->current + 1);
	proc->current += sizeof(int);
}

void funcsetc(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->regC = *(int*) (proc->current + 1);
	proc->current += sizeof(int);
}

void funccmp(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	setCompareFlag(proc, (int) (proc->regA != proc->regB));
}

void funcjmp(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->current += *(int*) (proc->current + 1);
}

/*
 * : In : A->sizetoread
 * : Out: A->allocatedmem
 */
void funcgettext(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	char *buff = malloc(sizeof(char)*proc->regA);
	read(0, buff, proc->regA);
	proc->regA = (int) buff;
}

/*
 * : In : A->bufftodisplay
 * : In : B->sizeofbuff
 */
void funcputtext(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	write(1, (char*) proc->regA, proc->regB);
}

void stralloc(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->strings[proc->stringslen].len = * (int*) (proc->current + 1);
	proc->strings[proc->stringslen].ptr = proc->current + sizeof(int) + 1;
	proc->current+= proc->strings[proc->stringslen].len + sizeof(int);
	proc->stringslen++;
}

/*
 * : In : A->stringnumber
 */
void uncipher(void *procVoid)
{
	u_char i;
	processor_t *proc = (processor_t*) procVoid;
	
	for (i = 0; i < proc->strings[proc->regA].len; i++)
	{
		*(proc->strings[proc->regA].ptr + i) += 42;
	}
}

/*
 * In : A->stringnumber
 * Out: A->addressofstring
 * Out: B->sizeofstring
 */
void getstr(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->regB = proc->strings[proc->regA].len;
	proc->regA = (int) proc->strings[proc->regA].ptr;
}

/*
 * In: A->addressofstring
 * In: B->sizeofstring
 */
void strstore(void *procVoid)
{
	processor_t *proc = (processor_t*) procVoid;
	proc->strings[proc->stringslen].len = proc->regB;
	proc->strings[proc->stringslen].ptr = (u_char*) proc->regA;
	proc->stringslen++;
}

void initializeProc(processor_t *proc, u_char *start)
{
	proc->current = start;
	proc->regA = 0;
	proc->regB = 0;
	proc->regC = 0;
	proc->flags = 0;
	proc->stringslen = 0;

	proc->opcodes[0].opcode = 0xF0;
	proc->opcodes[0].func = &funcseta;
	proc->opcodes[1].opcode = 0xF1;
	proc->opcodes[1].func = &funcsetb;
	proc->opcodes[2].opcode = 0xF2;
	proc->opcodes[2].func = &funcsetc;
	proc->opcodes[3].opcode = 0xF3;
	proc->opcodes[3].func = &funccmp;
	proc->opcodes[4].opcode = 0xF4;
	proc->opcodes[4].func = &funcjmp;
	proc->opcodes[5].opcode = 0xF5;
	proc->opcodes[5].func = &funcgettext;
	proc->opcodes[6].opcode = 0xF6;
	proc->opcodes[6].func = &funcputtext;
	proc->opcodes[7].opcode = 0xE0;
	proc->opcodes[7].func = &stralloc;
	proc->opcodes[8].opcode = 0xE1;
	proc->opcodes[8].func = &getstr;
	proc->opcodes[9].opcode = 0xE2;
	proc->opcodes[9].func = &uncipher;
	proc->opcodes[10].opcode = 0xE3;
	proc->opcodes[10].func = &strstore;
}

void destroyProc(processor_t *proc __attribute__((unused)))
{
}

void executeOpCode(processor_t *proc)
{
	u_char i, found;
	i = 0;
	found = 0;

	logging("[+] Reading byte 0x%02X\n", *proc->current);

	while ((!found) && (i < OPCODES_SIZE))
	{
		if (*proc->current == proc->opcodes[i].opcode)
		{
			logging("[+] Found OPCODE %02X\n"
							, proc->opcodes[i].opcode);
			found = 1;
			proc->opcodes[i].func((void *) proc);
		}

		i++;
	}

	proc->current++;
}

int executeBytecode(chunk_t bytecode)
{
	processor_t proc;

	if (!isValidMagic(bytecode.ptr))
	{
		logging("[-] Unable to verify magic bytes\n");
		return 1;
	}

	initializeProc(&proc, bytecode.ptr + (sizeof(MAGIC_BYTES)/sizeof(u_char)));

	while(*proc.current != 0xFF)
	{
		printProc(proc);
		executeOpCode(&proc);
	}

	destroyProc(&proc);

	return 0;
}

/*
 *	Main routine verifying arguments
 */
int main(int argc, char *argv[])
{
	FILE *fileBytecode;
	struct stat fileStats;
	u_char *bytecode;
	size_t resultSize;
	chunk_t bytecodeChunk;

	if (argc < 2)
	{
		usage(argv[0]);
		return 0;
	}

	if ((fileBytecode = fopen(argv[1], "rb+")) == NULL)
	{
		logging("[-] Unable to open file %s\n", argv[1]);
		return 1;
	}

	if (!(stat(argv[1], &fileStats) == 0))
	{
		logging("[-] Unable to stat file %s\n", argv[1]);
		return 1;
	}

	logging("[+] Allocating %lu bytes for bytecode\n"
					, (long unsigned int) fileStats.st_size);

	if ((bytecode = malloc(sizeof(u_char)*fileStats.st_size)) == NULL)
	{
		logging("[-] Failed to allocate space for bytecode\n");
		return 1;
	}

	resultSize = fread(bytecode, 1, fileStats.st_size, fileBytecode);
	logging("[+] Copied %zu bytes for bytecode\n"
					, resultSize);

	bytecodeChunk.ptr = bytecode;
	bytecodeChunk.len = resultSize;

	logging("[+] Executing bytecode at 0x%08X\n"
					, (int) bytecode);

	executeBytecode(bytecodeChunk);

	logging("[+] Freeing allocated memory\n");
	free(bytecode);
	bytecode = NULL;

	fclose(fileBytecode);

	return 0;
}

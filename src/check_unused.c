#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <stdlib.h>

void usage(char *s)
{
	printf("Usage: %s\n", s);
}

/*
 * find unused bytes in elf_file
 */
int find_unused_bytes(unsigned char* elf_file, size_t len)
{
    Elf32_Word elf_sz = 0;
    Elf32_Ehdr elf_hdr;
    Elf32_Shdr shdr;
    Elf32_Phdr phdr;
    void* ptr = NULL;
    size_t highest = 0;
    int i = 0;
    int j = 0;
    int unused_count=0;
    int debug = 0;
    unsigned char is_used[len];
    /*
     * 0 -> unused  (X)
     * 1 -> header  (H)
     * 2 -> PHT     (P) 
     * 3 -> SHT     (S)
     * 4 -> section (s)
     */

    /* initialize is_used array */
    for (i=0;i<len;i++)
        is_used[i] = 0;

    ptr = memcpy(&elf_hdr, elf_file, sizeof(Elf32_Ehdr));
    if (ptr == NULL)
    {
        if (debug) printf("[-] memcpy failed\n");
        return -1;
    }

    /* check if elf_file is a elf_file */
    if (elf_hdr.e_ident[EI_MAG0] != 0x7f ||
        elf_hdr.e_ident[EI_MAG1] != 0x45 || 
        elf_hdr.e_ident[EI_MAG2] != 0x4c || 
        elf_hdr.e_ident[EI_MAG3] != 0x46)
    {
         if (debug) printf("[-] input file does not appear to be a ELF file\n");
         return -1;
    }

    /* ELF header size */
    elf_sz += elf_hdr.e_ehsize;

    for (j=0;j<elf_hdr.e_ehsize;j++)
        is_used[j] = 1;

    if (highest < elf_hdr.e_ehsize) 
        highest = elf_hdr.e_ehsize;
    if (debug) printf("[+] ELF header size: 0x%08x\n", elf_hdr.e_ehsize);

    if (elf_hdr.e_phoff != 0)
    {
        if (debug) printf("[+] found program header table: 0x%08x - 0x%08x\n", 
                          elf_hdr.e_phoff, 
                          elf_hdr.e_phoff+((elf_hdr.e_phnum)*(elf_hdr.e_phentsize)));

        for (j=elf_hdr.e_phoff;j<elf_hdr.e_phoff+(elf_hdr.e_phnum)*(elf_hdr.e_phentsize);j++)
            is_used[j] = 2;

        if (highest < elf_hdr.e_phoff+((elf_hdr.e_phnum)*(elf_hdr.e_phentsize))) 
            highest = elf_hdr.e_phoff+((elf_hdr.e_phnum)*(elf_hdr.e_phentsize));
        
        /* PHT size */
        elf_sz += ((elf_hdr.e_phnum)*(elf_hdr.e_phentsize));
        if (debug) printf("PHT size: 0x%08x\n", (elf_hdr.e_phnum)*(elf_hdr.e_phentsize));
        for (i=0; i<elf_hdr.e_phnum; i++)
        {
            /* fseek(infile, elf_hdr.e_phoff+(elf_hdr.e_phentsize)*i, SEEK_SET); */
            /* sz = fread(&phdr, 1, sizeof(Elf32_Phdr), infile); */
            ptr = memcpy(&phdr, elf_file+elf_hdr.e_phoff+(elf_hdr.e_phentsize)*i, sizeof(Elf32_Phdr));
            if (debug) printf("\tsegment[%d] @ 0x%08x: size 0x%08x\n", i, phdr.p_offset, phdr.p_filesz);

            /*
            for (i=phdr.p_offset;i<phdr.p_offset+phdr.p_filesz;i++)
                is_used[i]=1;
            */
        }
    }

    if (elf_hdr.e_shoff != 0)
    {
        if (debug) printf("[+] found section header table @ 0x%08x (%d sections)\n", elf_hdr.e_shoff, elf_hdr.e_shnum);

        for (j=elf_hdr.e_shoff;j<elf_hdr.e_shoff+(elf_hdr.e_shnum)*(elf_hdr.e_shentsize);j++)
            is_used[j] = 3;

        
        if (highest < elf_hdr.e_shoff+((elf_hdr.e_shnum)*(elf_hdr.e_shentsize))) 
            highest = elf_hdr.e_shoff+((elf_hdr.e_shnum)*(elf_hdr.e_shentsize));
        /* SHT size */
        elf_sz += ((elf_hdr.e_shnum)*(elf_hdr.e_shentsize));
        if (debug) printf("[+] SHT size: 0x%08x\n", (elf_hdr.e_shnum)*(elf_hdr.e_shentsize));
        for (i=0; i<elf_hdr.e_shnum; i++)
        {
            /* fseek(infile, elf_hdr.e_shoff+(elf_hdr.e_shentsize)*i, SEEK_SET); */
            /* sz = fread(&shdr, 1, sizeof(Elf32_Shdr), infile); */
            ptr = memcpy(&shdr, elf_file+elf_hdr.e_shoff+(elf_hdr.e_shentsize)*i, sizeof(Elf32_Shdr));
            if (shdr.sh_type == SHT_NOBITS)
            {
                if (debug) printf("\tsection[%d] @ 0x%08x: size 0x%08x (NOBITS)\n", i, shdr.sh_offset, shdr.sh_size);
            }
            else
            {
                if (debug) printf("\tsection[%d] @ 0x%08x: size 0x%08x\n", i, shdr.sh_offset, shdr.sh_size);
                if (highest < shdr.sh_offset+shdr.sh_size) 
                    highest = shdr.sh_offset+shdr.sh_size;
                /* Section size */
                elf_sz += shdr.sh_size;
                
                for (j=shdr.sh_offset;j<shdr.sh_offset+shdr.sh_size;j++)
                    is_used[j]=4;
            }
        }
    }

    if (debug) printf("[+] final size is %d\n", elf_sz);
    if (debug) printf("[+] highest byte referenced: %d\n", highest);

    
    for (i=0;i<len;i++)
    {
        if (is_used[i] == 0)
        {
            printf(".");
            unused_count++;
        }
        else if (is_used[i] == 1)
            printf("H"); 
        else if (is_used[i] == 2)
            printf("P"); 
        else if (is_used[i] == 3)
            printf("S");
        else if (is_used[i] == 4)
            printf("s");
        else
             printf("X"); 
    }
    /* printf("\n"); */
    return unused_count;
}
/*
 *	Main routine verifying arguments
 */
int main(int argc, char *argv[])
{
    int i=0;
    FILE* infile;
    struct stat infile_stats;
    unsigned char* buff;
    int unused_count = 0;
    int total_unused_count = 0;
    size_t sz=0;
    int debug = 1;

    if (argc < 2)
        return 1;

    for (i=1;i<argc;i++)
    {
        unused_count=0;
        if ((infile = fopen(argv[i],"rb")) == NULL)
        {
            printf("[-] unable to open file %s\n", argv[i]);
            return 1;
        }
        if (!(stat(argv[i], &infile_stats) == 0))
        {
            printf("[-] unable to stat file %s\n", argv[i]);
            return 1;
        }
        if (debug) printf("[+] trying to allocate %d bytes for %s\n", 
                           (int) infile_stats.st_size,
                           argv[i]);
        if ((buff = malloc(sizeof(unsigned char)*infile_stats.st_size)) == NULL)
        {
            printf("[-] malloc failed\n");
            return 1;
        }
        sz = fread(buff, 1, infile_stats.st_size, infile);
        unused_count = find_unused_bytes(buff, infile_stats.st_size);
        if (unused_count >=0)
        {
            printf("[+] found %d unused bytes in file %s\n", unused_count, argv[i]);
            total_unused_count += unused_count;
        }
        else
        {
            printf("[-] check_unused failed for file %s\n", argv[i]);
        }
        fclose(infile);
        free(buff);
    }
    printf("\n");
    printf("[+] total number of unused bytes: %d\n", total_unused_count);
	return 0;
}

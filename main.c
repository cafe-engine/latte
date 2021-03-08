#include "src/latte.h"
#include <elf.h>

#if defined(__LP64__)
#define ElfC(type) Elf64_##type
#else
#define ElfC(type) Elf32_##type
#endif

int main(int argc, char ** argv) {
    la_File *f;
    la_Dir *d;

    la_Header h;
    ElfC(Ehdr) header;

    la_init(".");

    la_File *file = la_fopen(argv[0], LA_READ_MODE);

    if (file) {
        la_fread(file, (char*)&header, sizeof(header));
        if (memcmp(header.e_ident, ELFMAG, SELFMAG) == 0) {
            int sz = header.e_shoff + (header.e_shentsize * header.e_shnum);
            int tsize = la_fsize(file) - sz;

            printf("file: %ld elf: %d\n", la_fsize(file), sz);
            printf("cat data size: %d\n\n", tsize);

            la_fseek(file, sz); 
            if (tsize > 0) {
                printf("DATA\n");
                printf("*************************************************\n");
                char buf[tsize];
                la_fread(file, buf, tsize);

                /*printf("%s\n", buf);*/
                int i = 0;
                while (i < tsize) printf("%c", buf[i++]);
                printf("\n*************************************************\n");
                
            }
        }


        la_fclose(file);
    }

    /*FILE *file = fopen("latte", "rb");
    if (file) {
        fread(&header, sizeof(header), 1, file);
        if (memcmp(header.e_ident, ELFMAG, SELFMAG) == 0) {
            printf("deu certo, man\n");
            printf("phoff: %ld shoff: %ld\n", header.e_phoff, header.e_shoff);
            printf("pentsize: %d shentsize: %d\n", header.e_phentsize, header.e_shentsize);
            printf("phnum: %d shnum: %d\n", header.e_phnum, header.e_shnum);
            int table_len = header.e_phentsize * header.e_phnum;
            int sh_table_len = header.e_shentsize * header.e_shnum;
            printf("table: %d section header table: %d\n", table_len, sh_table_len);
            int sz = header.e_shoff + sh_table_len;
            printf("size: %d\n", sz);
            fseek(file, sz, SEEK_SET);
        }

        char r[6];
        fread(r, 5, 1, file); 
        r[5] = '\0';
        printf("%s\n", r);

        fclose(file);
    }*/


    const char *arg = argv[1] != NULL ? argv[1] : ".";

    la_header(arg, &h);
    if (h.type == LA_TREG) {
        f = la_fopen(arg, LA_READ_MODE);

        printf("name: %s size: %ld\n", h.name, h.size);
	la_fheader(f, &h);
	char buf[h.size];
        la_fread(f, buf, h.size);

	/*printf("%*s\n", (int)h.size, buf);*/
        int i = 0;
        while (i < h.size) {
            if (buf[i] == '\n') {
                printf("\n");
            } else {
                printf("%c", buf[i]);
            }
            i++;
        }

	la_fclose(f);
    } else {
	d = la_dopen(arg);

	while (la_dread(d, &h)) {
	    printf("%s\n", h.name);
	}

	la_dclose(d);
    }


    la_deinit();
    printf("\n");

    return 1;
}

/*
 *
 * la_touch("caraio.txt");
 * laFile *f = la_open("main.c", LA_READ | LA_WRITE | LA_BINARY);
 * int sz = la_size(f);
 * char buf[sz+1];
 * la_read(f, buf);
 * la_write(f, "carain, vinhado, foda");
 * la_close(f);
 *
 *
 *
 * */

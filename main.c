#include "src/latte.h"

int main(int argc, char ** argv) {
    la_File *f;
    la_Dir *d;

    la_Header h;

    const char *arg = argv[1] != NULL ? argv[1] : ".";
    la_init(".");

    la_header(arg, &h);
    if (h.type == LA_TREG) {
	f = la_fopen(arg, LA_READ_MODE);

	la_fheader(f, &h);
	char buf[h.size + 1];
	la_fread(f, buf, h.size);
	buf[h.size] = '\0';

	printf("%s\n", buf);

	la_fclose(f);
    } else {
	d = la_dopen(arg);

	while (la_dread(d, &h)) {
	    printf("%s\n", h.name);
	}

	la_dclose(d);
    }


    la_deinit();

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

#define LATTE_IMPLEMENTATION
#include "latte.h"

int main(int argc, char ** argv) {
    la_init(".");

    la_virtdrv_t *drv = la_vopen("teste.tar", LA_READ_MODE);

    la_file_t *fp = la_vfopen(drv, "");

    la_header_t h;

    la_fheader(fp, &h);

    la_log("name: %s size: %ldkb", h.name, h.size/1024); /* bytes to kb */

    la_log("data: ");
    char buf[h.size+1];
    la_fread(fp, buf, h.size);
    buf[h.size] = '\0';

    int i = 0;
    while (i < h.size) printf("%c", buf[i++]);


    return 0;
}

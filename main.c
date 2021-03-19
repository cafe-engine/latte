#include "latte.h"

void _print_file(la_file_t *fp) {
    la_header_t h;
    la_fheader(fp, &h);
    printf("name: %s, size: %ld\n", h.name, h.size);

    char vbuf[h.size+1];
    la_fread(fp, vbuf, h.size);
    vbuf[h.size] = '\0';
    la_log("%s", vbuf);
}

int main(int argc, char ** argv) {
    la_init(".");

    la_file_t *fp;
    la_dir_t *dir;
    la_header_t h;

    la_vdrive_t *drv = la_vopen("teste.tar", LA_READ_MODE);
    
    fp = la_vfopen(drv, "README.md");

    _print_file(fp);
    _print_file(la_vfopen(drv, ".git"));
    /*la_fheader(fp, &h);
    printf("name: %s, size: %ld\n", h.name, h.size);

    char vbuf[h.size+1];
    la_fread(fp, vbuf, h.size);
    vbuf[h.size] = '\0';
    la_log("%s", vbuf);*/

    la_vclose(drv);

    if (argv[1]) {
        if (la_isfile(argv[1])) {
            fp = la_fopen(argv[1], LA_READ_MODE);
            la_fheader(fp, &h); 

            char buf[h.size];

            la_fread(fp, buf, h.size);

            int i = 0;
            while (i < h.size) printf("%c", buf[i++]);

            la_fclose(fp);
        } else {
            dir = la_dopen(argv[1]);
            while (la_dread(dir, &h)) {
                la_log("name: %s size: %dkb", h.name, h.size / 1024);
            }

            la_dclose(dir);
        }
    }

    return 0;
}

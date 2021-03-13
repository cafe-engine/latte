#include "src/latte.h"

int main(int argc, char ** argv) {
    la_init(".");

    la_file_t *fp;
    la_dir_t *dir;
    la_header_t h;

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

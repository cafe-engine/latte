#ifndef LATTE_H
#define LATTE_H

#include <stdio.h>

#define LA_API extern

#define LATTE_VERSION "0.1.0"
#define LA_VERSION "0.1.0"

#define LA_STR(x) #x
#define LA_ASSERT(expr, msg) \
    if (!(expr)) { \
	fprintf(stderr, "Assertion %s failed: %s\n", LA_STR(expr), msg); \
	exit(1); \
    }

enum {
    LA_TREG = 0,
    LA_TLINK,
    LA_TSYM,
    LA_TCHR,
    LA_TBLK,
    LA_TDIR,
    LA_TFIFO
};

enum {
    LA_UNKOWN_MODE        = (     0),
    LA_READ_MODE          = (1 << 0),
    LA_WRITE_MODE         = (1 << 1),
    LA_REWRITE_MODE       = (1 << 2),
};

enum {
    LA_FILE_STREAM = 0,
    LA_FILE_STATIC
};

typedef struct Latte Latte;
typedef struct la_VirtDrive la_VirtDrive;

typedef struct la_Header {
    char name[100];
    char linkname[100];
    int type;
    int mode;
    long size;
    unsigned int uid, gid;
    char uname[32], gname[32];
} la_Header;

typedef struct la_File la_File;
typedef struct la_Dir la_Dir;

/*struct VirtFile {
	char name[100];
	int type;
	unsigned offset;
	long size;

	void *stream;
	void *data;
	void *userdata;
};*/

/*struct VirtDir {
    long size;
    void *stream;
    VirtFile *files;
};*/

LA_API void la_resolve_path(const char *path, char *out);

LA_API void la_get_basedir(char *out);
LA_API void la_set_basedir(const char *basedir);


LA_API int la_init(const char *basedir);
LA_API void la_deinit();

LA_API long la_size(const char *filename, FILE ** out);

LA_API int la_file_iself(la_File *f);
LA_API int la_file_ispe(la_File *f);

LA_API int la_elf_fsize(la_File *f);

LA_API int la_elf_fread(la_File *f, char *out, int bytes);

LA_API int la_elf_fwrite(la_File *f, const char *text, int bytes);
/* FileSystem */

LA_API int la_header(const char *filename, la_Header *out);

LA_API int la_read(const char *filename, char **out, int *sz);
LA_API int la_write(const char *filename, const char *text, int sz);

LA_API int la_touch(const char *filename);
LA_API int la_rm(const char *filename);

LA_API int la_mkdir(const char *path);
LA_API int la_rmdir(const char *path);

LA_API int la_isfile(const char *filename);
LA_API int la_isdir(const char *path);


/* File */

LA_API int la_fheader(la_File *f, la_Header *out);

LA_API la_File* la_fopen(const char *filename, int mode);
LA_API int la_freopen(la_File *f, const char *filename, int mode);
LA_API void la_fclose(la_File *f);

LA_API void la_fclear(la_File *f);

LA_API long la_fsize(la_File *f);
LA_API int la_fseek(la_File *f, int offset);
LA_API long la_ftell(la_File *f);

LA_API int la_fread(la_File *f, char *out, int bytes);
LA_API int la_fwrite(la_File *f, const char *txt, int len);
LA_API int la_frewrite(la_File *f, const char *txt, int len);

LA_API int la_finsert(la_File *f, const char *txt, int len);
LA_API int la_fappend(la_File *f, const char *txt, int len);

/* Directory */

LA_API int la_dheader(la_Dir *dir, la_Header *h);

LA_API la_Dir* la_dopen(const char *path);
LA_API void la_dclose(la_Dir *dir);

LA_API int la_dread(la_Dir *dir, la_Header *out);
LA_API void la_dlist(la_Dir *dir);
LA_API la_File* la_dopen_file(la_Dir *dir, const char *filename);

/* Virtual FileSystem */

LA_API la_VirtDrive* la_virt_create(int mode);
LA_API la_VirtDrive* la_virt_mount(const char *path, int mode, int usage);

LA_API void la_virt_destroy(la_VirtDrive *drv);

LA_API int la_virt_sync(const char *path, int packed);

LA_API void la_virt_store(la_VirtDrive *drv, const char *filename);

LA_API la_File* la_virt_fopen(la_VirtDrive *drv, const char *filename);
LA_API la_Dir* la_virt_dopen(la_VirtDrive *drv, const char *path);

LA_API int la_virt_touch(la_VirtDrive *drv, const char *filename);
LA_API int la_virt_rm(la_VirtDrive *drv, const char *filename);

LA_API void la_virt_mkdir(la_VirtDrive *drv, const char *path);
LA_API void la_virt_rmdir(la_VirtDrive *drv, const char *path);

LA_API int la_virt_isfile(la_VirtDrive *drv, const char *filename);
LA_API int la_virt_isdir(la_VirtDrive *drv, const char *path);

#endif /* LATTE_H */

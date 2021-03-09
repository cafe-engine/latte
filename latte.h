#ifndef LATTE_H
#define LATTE_H

#define LATTE_VERSION "0.1.0"

#define LA_API extern
#define LA_VERSION LATTE_VERSION

enum {
    LA_TREG = 0,
    LA_TDIR
};

enum {
    LA_READ_MODE = 1,
    LA_WRITE_MODE,
    LA_REWRITE_MODE
};

typedef struct la_File la_File;
typedef struct la_Dir la_Dir;
typedef struct la_virtdrv_s la_virtdrv_t;

typedef struct la_Node la_Node;

typedef struct la_posixh_s la_posixh_t;
typedef struct la_header_s la_header_t;
typedef struct la_file_s la_file_t;
typedef struct la_dir_s la_dir_t;
typedef struct la_node_s la_node_t;

struct la_header_s {
    char name[100];
    char linkname[100];
    int type;
    int mode;
    long size;
    unsigned int uid, gid;
    char uname[32], gname[32];
};

/**********************
 *        Core        *
 **********************/
LA_API int la_resolve_path(const char *path, char *out);

LA_API void la_get_basedir(char *out);
LA_API void la_set_basedir(const char *basedir);

LA_API int la_init(const char *basedir);
LA_API void la_deinit();

LA_API int la_header(const char *path, la_header_t *out);

LA_API int la_touch(const char *filename);
LA_API int la_rm(const char *filename);

LA_API int la_mkdir(const char *path);
LA_API int la_rmdir(const char *path);

LA_API int la_isfile(const char *filename);
LA_API int la_isdir(const char *path);

/**********************
 *        File        *
 **********************/

LA_API la_file_t* la_fopen(const char *filename, int mode);
LA_API int la_freopen(la_file_t *fp, const char *filename, int mode);
LA_API void la_fclose(la_file_t *fp);

LA_API int la_fheader(la_file_t *fp, la_header_t *out);
LA_API int la_fseek(la_file_t *fp, long offset);
LA_API long la_ftell(la_file_t *fp);

LA_API int la_fread(la_file_t *fp, char *out, int bytes);
LA_API int la_fwrite(la_file_t *fp, const char *buffer, int bytes);

LA_API int la_fappend(la_file_t *fp, const char *buffer, int bytes);
LA_API int la_finsert(la_file_t *fp, const char *buffer, int bytes);

/**********************
 *        Dir         *
 **********************/

LA_API la_dir_t* la_dopen(const char *path);
LA_API void la_dclose(la_dir_t *dp);

LA_API int la_drewind(la_dir_t *dp);
LA_API int la_dread(la_dir_t *dp, la_header_t *out);

/***********************
 *       Virtual       *
 ***********************/

LA_API la_virtdrv_t* la_vopen(const char *filename, int mode);
LA_API la_virtdrv_t* la_vmount(const char *path, int mode);

LA_API void la_vclose(la_virtdrv_t *drv);

LA_API la_file_t* la_vfopen(la_virtdrv_t *drv, const char *filename);
LA_API la_dir_t* la_vdopen(la_virtdrv_t *drv, const char *path);

/***********************
 *        Utils        *
 ***********************/

LA_API void* la_dlopen(const char *libpath);
LA_API void la_dlclose(void *dl);

LA_API int la_dlcall();

#endif /* LATTE _H */

#if defined(LATTE_IMPLEMENTATION)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define latte() (&_ctx)

struct la_file_t {
    la_header_t h;
    int pos, offset;
    int mode;
    FILE *stream;
};

struct la_dir_t {
    la_header_t h;
    DIR *stream;
};

struct Latte {
    char basedir[64];
    int basedir_len;
};

static Latte _ctx;

/**********************
 *        Core        *
 **********************/

int la_resolve_path(const char *path, char *out) {
    char base[64];
    la_get_basedir(base);
    int len = strlen(base);

    strcpy(out, base);
    if (base[len] != '/' && base[len] != '\\') strcat(out, "/");
    strcat(out, path);
    return 1;
}

void la_set_basedir(const char *basedir) {
    if (!basedir) return;
    strcpy(latte()->basedir, basedir);
}

void la_get_basedir(char *out) {
    if (!out) return;
    strcpy(out, latte()->basedir);
}

int la_init(const char *basedir) {
   la_set_basedir(basedir);
}

void la_deinit() {

}

int la_header(const char *path, la_header_t *out) {
    if (!path) return 0;
    if (!out) return 0;
    char file[100];

    la_resolve_path(path, file);

    struct stat s;
    if (stat(file, &s) != 0) {
        fprintf(stderr, "cannot open %s\n", path);
        return 0;
    }

    out->gid = s.st_gid;
    out->uid = s.st_uid;
    strcpy(out->name, path);
    out->mode = s.st_mode & 0x8f;

    out->type = S_ISDIR(s.st_mode) ? LA_TDIR : LA_TREG;
    out->size = s.st_size;

    return 1;
}

int la_touch(const char *filename) {
    if (!filename) {
        fprintf(stderr, "filename cannot be NULL\n");
        return 0;
    }
    char file[100];
    la_resolve_path(filename, file);

    if (la_isfile(filename)) return 1;
    FILE *fp;
    fp = fopen(file, "wb");
    if (!fp) {
        fprintf(stderr, "failed to create %s\n", filename);
        return 0;
    }
    fclose(fp);

    return 1;
}

int la_rm(const char *filename) {
    if (!filename) return 0;

    return remove(filename) != -1;
}

int la_mkdir(const char *path) {
    if (!path) return 0;
    char dir[100];
    la_resolve_path(path, dir);
    int err;
#ifdef _WIN32
    err = _mkdir(dir);
#else
    err = mkdir(dir, 0733);
#endif
    return err;
}

int la_rmdir(const char *path) {
    if (!path) return 0;

    char dir[100];
    la_resolve_path(path, dir);

#ifdef _WIN32
    _rmdir(dir);
#else
    rmdir(dir);
#endif
    return 1;
}

int la_isfile(const char *filename) {
    la_header_t h;
    la_header(filename, &h);

    return h.type == LA_TREG;
}

int la_isdir(const char *path) {
    la_header h;
    la_header(filename, &h);

    return h.type == LA_TDIR;
}

/**********************
 *        File        *
 **********************/

la_file_t* la_fopen(const char *filename, int mode) {
   la_file_t *fp = (la_file_t*)malloc(sizeof(*fp)); 
   memset(fp, 0, sizeof(*fp));
    if (!fp) {
        fprintf(stderr, "failed to malloc\n");
        exit(1);
    }
    if (filename == NULL) return NULL;

    char file[100];
    la_resolve_path(filename, file);
    if (mode & LA_WRITE_MODE) la_touch(file);
}

#endif /* LATTE_IMPLEMENTATION */

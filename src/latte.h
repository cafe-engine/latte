#define LATTE_IMPLEMENTATION
#ifndef LATTE_H
#define LATTE_H

#define LATTE_VERSION "0.1.0"

#ifndef LA_API
    #if defined(_WIN32)
        #if defined(BUILD_SHARE)
	    #define LA_API __declspec(dllexport)
        #elif defined(USE_SHARED)
            #define LA_API __declspec(dllimport)
        #else
            #define LA_API
        #endif
    #else
        #define LA_API
    #endif
#endif

#define LA_VERSION LATTE_VERSION

#define LA_STR(x) #x
#define la_assert(s) if (!(s)) la_fatal("Assertion '%s' failed", LA_STR((s)))

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
    LA_READ_MODE    = (1 << 0),
    LA_WRITE_MODE   = (1 << 1),
    LA_REWRITE_MODE = (1 << 2)
};

enum {
    LA_RD_ONLY = 0,
    LA_WR_ONLY,
    LA_RDWR,
    LA_WRRD
};

enum {
    LA_FILE_STREAM = 0,
    LA_FILE_STATIC
};


enum {
    LA_LOG = 1,
    LA_ERROR,
    LA_WARNING,
    LA_FATAL,

    LA_TRACE = 8
};

typedef struct Latte Latte;

typedef struct la_virtdrv_s la_virtdrv_t;

typedef struct la_posix_header_s la_posix_header_t;

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
    unsigned mtime;
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

LA_API int la_vreplace(la_virtdrv_t *drv, const char *f1, const char *f2);

/***********************
 *        Utils        *
 ***********************/
typedef void la_lib_t;

typedef struct la_tar_s la_tar_t;

LA_API la_lib_t* la_dlopen(const char *libpath);
LA_API void la_dlclose(la_lib_t *dl);

LA_API int la_dlcall(la_lib_t *dl, const char *fn);

/***********************
 *        Debug        *
 ***********************/

LA_API int la_log_output(void *out);
LA_API int la_log_(int mode, const char *file, int line, const char *function, const char *fmt, ...);

#define la_log(msg...) la_log_(LA_LOG, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_warn(msg...) la_log_(LA_WARNING, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_error(msg...) la_log_(LA_ERROR, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_fatal(msg...) la_log_(LA_FATAL, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_trace(msg...) la_log_(LA_TRACE, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)

#define la_traceerror(msg...) la_log_(LA_ERROR | LA_TRACE, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_tracewarn(msg...) la_log_(LA_WARNING | LA_TRACE, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define la_tracefatal(msg...) la_log_(LA_FATAL | LA_TRACE, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#endif /* LATTE _H */


#if defined(LATTE_IMPLEMENTATION)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define latte() (&_ctx)

/* POSIX header */
struct posix_header_s {
  struct {
    char name[100];       /* 0 */
    char mode[8];         /* 100 */
    char uid[8];          /* 108 */
    char gid[8];          /* 116 */
    char size[12];        /* 124 */
    char mtime[12];       /* 136 */
    char chksum[8];       /* 148 */
    char typeflag;        /* 156 */
    char linkname[100];   /* 157 */
    char magic[6];        /* 257 */
    char version[2];      /* 263 */
    char uname[32];       /* 265 */
    char gname[32];       /* 297 */
    char devmajor[8];     /* 329 */
    char devminor[8];     /* 337 */
    char prefix[155];     /* 345 */
    char _[12];           /* 500 */
  };                      /* 512 */
};

typedef char la_block_t[512];

struct la_file_s {
    char usage;
    la_header_t h;
    int pos, offset;
    int mode;
    union {
        FILE *stream;
        void *data;
    };
};

struct la_dir_s {
    la_header_t h;
    DIR *stream;
};

struct la_virtdrv_s {
    la_file_t stream;
};

struct Latte {
    char basedir[64];
    int basedir_len;
    FILE *out;
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
   return 1;
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
    la_header_t h;
    la_header(path, &h);

    return h.type == LA_TDIR;
}

/**********************
 *        File        *
 **********************/

static int _get_mode(char *out, int mode) {
    if (!out) return 0;
    memset(out, 0, 3);

    if (mode & LA_REWRITE_MODE) out[0] = 'w';
    else if (mode & LA_READ_MODE) out[0] = 'r';
    else if (mode & LA_WRITE_MODE) out[0] = 'w';
    else {
        fprintf(stderr, "invalid file open mode: %d\n", mode);
        exit(1);
    }

    out[1] = 'b';
    
    if (mode & LA_REWRITE_MODE || mode & LA_WRITE_MODE)
        if (mode & LA_READ_MODE) out[2] = '+';

    return 1;
}

static int _header(const char *filepath, la_header_t *out) {
    la_assert(filepath != NULL);
    if (!out) return 0;

    struct stat s;
    const char *file = filepath;

    if (stat(file, &s) != 0) {
        FILE *out = latte()->out;
        la_log_output(stderr);
        la_error("cannot open file %s\n", filepath);
        la_log_output(out);
        return 0;
    }

    out->gid = s.st_gid;
    out->uid = s.st_uid;
    strcpy(out->name, file);
    out->mode = s.st_mode & 0x8f;

    out->type = S_ISDIR(s.st_mode) ? LA_TDIR : LA_TREG;
    out->size = s.st_size;

    return 1;
}

static int _file_init(la_file_t *fp, const char *filename, int mode) {
    la_assert(fp != NULL);

    char m[3];
    _get_mode(m, mode);
    if (!fp->stream) fp->stream = fopen(filename, m);
    else fp->stream = freopen(filename, m, fp->stream);

    if (fp->stream == NULL) la_fatal("cannot open file %s", filename);

    _header(filename, &fp->h);

    fp->mode = mode;
    fp->pos = 0;
    fp->offset = 0;

    return 1;
}

la_file_t* la_fopen(const char *filename, int mode) {
   /*la_file_t *fp = (la_file_t*)malloc(sizeof(*fp)); 
   memset(fp, 0, sizeof(*fp));
    if (!fp) la_fatal("failed to malloc\n");
    if (filename == NULL) return NULL;

    char file[100];
    la_resolve_path(filename, file);
    if (mode & LA_WRITE_MODE) la_touch(file);
    return fp;*/
    la_file_t *fp = (la_file_t*)malloc(sizeof(*fp));
    if (!fp) la_fatal("failed to malloc\n");
    la_assert(filename != NULL);

    memset(fp, 0, sizeof(*fp));

    char file[100];
    la_resolve_path(filename, file);
    if (mode & LA_WRITE_MODE) la_touch(file);

    _file_init(fp, file, mode);

    strcpy(fp->h.name, filename);

    return fp;
}

int la_freopen(la_file_t *fp, const char *filename, int mode) {
    la_assert(fp != NULL);

    char file[100];
    la_resolve_path(filename, file);
    
    _file_init(fp, file, mode);
    strcpy(fp->h.name, filename);

    return 1;
}

void la_fclose(la_file_t *fp) {
    if (!fp) return;
}

int la_fheader(la_file_t *fp, la_header_t *out) {
    la_assert(fp != NULL);
    if (!out) return 0;

    memcpy(out, &fp->h, sizeof(*out));
    return 1;
}

long la_fsize(la_file_t *fp) {
    return fp->h.size;
}

int la_fseek(la_file_t *fp, long offset) {
    la_assert(fp != NULL);
    fp->offset = offset;

    fseek(fp->stream, fp->pos + fp->offset, SEEK_SET);
    return 1;
}

int la_fread(la_file_t *fp, char *out, int bytes) {
    la_assert(fp != NULL);
    if (!out) return 0;

    if (~fp->mode & LA_READ_MODE) {
        la_error("write-only file");
        return 0;
    }
    return fread(out, bytes, 1, fp->stream) == 0;
}

int la_fwrite(la_file_t *fp, const char *text, int len) {
 la_assert(fp != NULL);
    if (!text) return 0;

    if (len < 0) len = strlen(text);

    if (~fp->mode & LA_REWRITE_MODE && ~fp->mode & LA_WRITE_MODE) {
        la_error("read-only file");
        return 0;
    }
    int off = fp->pos + fp->offset;

    fseek(fp->stream, off, SEEK_SET);
    int res = fwrite(text, len, 1, fp->stream);
    int p = ftell(fp->stream);

    fseek(fp->stream, 0, SEEK_END);
    fp->h.size = ftell(fp->stream);
    fseek(fp->stream, p, SEEK_SET);
    fp->offset = p - fp->pos;

    return res;
}

int la_frewrite(la_file_t *fp, const char *text, int len) {
    la_assert(fp != NULL);
    if (!text) return 0;

    if (len < 0) len = strlen(text);

    if (~fp->mode & LA_WRITE_MODE && ~fp->mode & LA_REWRITE_MODE) {
        la_error("read-only file");
        return 0;
    }
    int start_sz = fp->offset - fp->pos;

    char sbuff[start_sz];
    la_fread(fp, sbuff, start_sz);

    int mode = fp->mode;

    la_freopen(fp, NULL, LA_REWRITE_MODE);
    la_freopen(fp, NULL, mode);

    la_fwrite(fp, sbuff, start_sz);
    la_fwrite(fp, text, len);

    return 1;
}

int la_fappend(la_file_t *fp, const char *text, int len) {
    la_assert(fp != NULL);
    if (!text) return 0;

    if (~fp->mode & LA_WRITE_MODE && ~fp->mode & LA_REWRITE_MODE) {
        la_error("read-only file");
        return 0;
    }

    la_fseek(fp, fp->h.size);
    la_fwrite(fp, text, len);

    return 1;
}

/************************
 *      Directory       *
 ************************/

int la_dheader(la_dir_t *dir, la_header_t *out) {
    la_assert(dir != NULL);
    if (!out) return 0;

    memcpy(out, &dir->h, sizeof(*out));
    return 1;
}

la_dir_t* la_dopen(const char *path) {
    la_assert(path != NULL);

    la_dir_t *dir = (la_dir_t*)malloc(sizeof(*dir));

    char p[100];
    la_resolve_path(path, p);

    dir->stream = opendir(p);

    _header(p, &dir->h);
    return dir;
}

void la_dclose(la_dir_t *dir) {
    if (!dir) return;
    if (dir->stream) closedir(dir->stream);

    free(dir);
}

int la_dread(la_dir_t *dir, la_header_t *out) {
    if (!dir) return 0;
    if (!out) return 0;
    struct dirent *d = NULL;

    d = readdir(dir->stream);
    if (!d) return 0;

    char file[100];
    strcpy(file, dir->h.name);
    strcat(file, "/");
    strcat(file, d->d_name);
    _header(file, out);
    return 1;
}

/*************************
 *        Virtual        *
 *************************/

la_virtdrv_t* la_vopen(const char *vhd, int mode) {
    la_assert(vhd != NULL);
    la_virtdrv_t *drv = (la_virtdrv_t*)malloc(sizeof(*drv));
    if (!drv) la_fatal("cannot malloc for %s", vhd);
    memset(drv, 0, sizeof(*drv));

    if (!la_freopen(&drv->stream, vhd, mode)) {
        la_error("cannot open virtual drive");
        free(drv);
        return NULL;
    }
    
    return drv;
}

la_virtdrv_t* la_vmount(const char *path, int mode) {
    return NULL;
}

void la_vclose(la_virtdrv_t *drv) {
    if (!drv) return;
    
    if (drv->stream.stream) fclose(drv->stream.stream);
}

la_file_t* la_vfopen(la_virtdrv_t *drv, const char *filename) {
    la_assert(drv != NULL);
    if (!filename) return NULL;
    
    return &drv->stream;
}

la_dir_t* la_vdopen(la_virtdrv_t *drv, const char *path) {
    return NULL;
}

/***********************
 *        Utils        *
 ***********************/

int la_log_output(void *out) {
    latte()->out = out;
    return 1;
}

int la_log_(int mode, const char *file, int line, const char *function, const char *fmt, ...) {

    if (!latte()->out) latte()->out = stdout;

    time_t t = time(NULL);
    struct tm *tm_now;

    va_list args;

    int m = (mode & 0x7);

    char smode[15] = "";

    switch (m) {
        case LA_LOG: sprintf((char*)smode, "[log]   "); break;
        case LA_ERROR: sprintf((char*)smode, "[error] "); break;
        case LA_WARNING: sprintf((char*)smode, "[warn]  "); break;
        case LA_FATAL: sprintf((char*)smode, "[fatal] "); break;
        default: strcpy((char*)smode, "");
    }
    if (m <= 1 && mode & LA_TRACE) strcpy((char*)smode, "[trace] ");

    tm_now = localtime(&t);
    char tm_buf[10];
    strftime((char*)tm_buf, sizeof(tm_buf), "%H:%M:%S", tm_now);
    fprintf(latte()->out, "%s %s%s:%d: ", tm_buf, smode, file, line); 
    if (mode & LA_TRACE) fprintf(latte()->out, "in function %s(): ", function);

    va_start(args, fmt);
    vfprintf(latte()->out, fmt, args);

    va_end(args);
    fprintf(latte()->out, "\n");
    if (mode == LA_FATAL) exit(-1);
    return 1;
}

#endif /* LATTE_IMPLEMENTATION */

#include "latte.h"

#include "sbtar.h"
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define latte() (&_ctx)
#define _WRITE_MODE(_f) (~(_f)->mode & LA_WRITE_MODE && ~(_f)->mode & LA_REWRITE_MODE) 

struct la_File {
    la_Header h;
    int pos, offset;
    int mode;
    FILE *stream;
};

struct la_Dir {
    la_Header h;
    DIR *stream;
};

typedef struct Node Node;
struct Node {
    posix_header_t h;

	union {
        la_Dir dir;
		la_File file;
	};
	Node *child;
    Node *next;
};

struct la_VirtDrive {
    int mode;
    void *stream;
	Node *root;
};

struct Latte {
    char basedir[64];
    int basedir_len;
};

static Latte _ctx;

void la_resolve_path(const char *path, char *out) {
    const char *base = (const char*)latte()->basedir;
    if (!out) return;

    if (!path) {
	if (!strcmp(base, ".")) out[0] = '\0';
	else strcpy(out, base);
	return;
    }

    long len = strlen(path);
    char res[100];
    res[0] = '\0';
    if (strcmp(base, ".")) {
	len += strlen(base) + 1;
	strcpy(res, base);
	strcat(res, "/");
    }

    strcat(res, path);
    res[len] = '\0';

    strcpy(out, res);
}
void la_get_basedir(char* out) {
    if (!out) return;
    strcpy(out, latte()->basedir);
}

void la_set_basedir(const char *basedir) {
    const char *path = basedir ? basedir : ".";
    strcpy(latte()->basedir, path); 
    latte()->basedir_len = strlen(path);
}

int la_init(const char *basedir) {
    la_set_basedir(basedir);

    return 1;
}

void la_deinit() {

}

static void _get_mode(char *out, int mode) {
    if (!out) return;
    int off = 0;

    memset(out, 0, 3);

    if (mode & LA_REWRITE_MODE) out[0] = 'w';
    else if (mode & LA_READ_MODE) out[0] = 'r';
    else if (mode & LA_WRITE_MODE) out[0] = 'a';
    out[1] = 'b';

    LA_ASSERT(off >= 0, "invalid file open mode");

    /*if (mode & LA_BINARY_MODE) out[off++] = 'b';*/

    if (mode & LA_REWRITE_MODE || mode & LA_WRITE_MODE) 
	if (mode & LA_READ_MODE) out[2] = '+';
}

static int _header(const char *filepath, la_Header *out) {
    if (!filepath) return 0;
    if (!out) return 0;

    struct stat s;
    const char *file = filepath;
    if (stat(file, &s) != 0) {
	fprintf(stderr, "cannot open file %s\n", filepath);
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

static int _file_init(la_File *f, const char *filename, int mode) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!filename) return 0;

    char m[3];
    _get_mode(m, mode);
    if (!f->stream) f->stream = fopen(filename, m);
    else f->stream = freopen(filename, m, f->stream);

    LA_ASSERT(f->stream != NULL, "cannot open file");
    /*fseek(f->stream, 0, SEEK_END);
    f->h.size = ftell(f->stream);
    fseek(f->stream, 0, SEEK_SET);

    strcpy(f->h.name, filename);*/
    _header(filename, &f->h);
    /*printf("%d\n", f->h.size);*/
    f->mode = mode;
    f->pos = 0;
    f->offset = 0;

    return 1;
}


la_File* la_fopen(const char *filename, int mode) {
    la_File *f = (la_File*)malloc(sizeof(*f));
    memset(f, 0, sizeof(*f));
    LA_ASSERT(f != NULL, "failed to malloc");
    LA_ASSERT(filename != NULL, "filename cannot be NULL");
    
    char file[100];
    la_resolve_path(filename, file);
    if (mode & LA_WRITE_MODE) la_touch(file);
    /*_get_mode(m, mode);
    f->stream = fopen(file, m);

    LA_ASSERT(f->stream != NULL, "cannot open file");

    fseek(f->stream, 0, SEEK_END);
    f->h.size = ftell(f->stream);
    fseek(f->stream, 0, SEEK_SET);

    strcpy(f->h.name, file);
    f->mode = mode;
    f->pos = 0;
    f->offset = 0;*/
    _file_init(f, file, mode);

    return f;
}

int la_freopen(la_File *f, const char *filename, int mode) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    /*char m[3];
    _get_mode(m, mode);
     f->stream = freopen(filename, m, f->stream);*/
    _file_init(f, filename, mode);

    return f->stream != NULL;
}

void la_fclose(la_File *fp) {
    if (!fp) return;

    if (fp->stream) fclose(fp->stream);

    free(fp);
}

int la_fheader(la_File *f, la_Header *out) {
    if (!f) return 0;
    if (!out) return 0;

    memcpy(out, &f->h, sizeof(*out));
    return 1;
}

long la_fsize(la_File *fp) {
    return fp->h.size;
}

int la_fseek(la_File *f, int offset) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    f->offset = offset;

    fseek(f->stream, f->pos + f->offset, SEEK_SET);

    return 1;
}

int la_fread(la_File *f, char *out, int bytes) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!out) return 0;

    if (~f->mode & LA_READ_MODE) return 0;

    return fread(out, bytes, 1, f->stream);
}

int la_fwrite(la_File *f, const char *txt, int len) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!txt) return 0;

    if (len < 0) len = strlen(txt);

    if (~f->mode & LA_REWRITE_MODE && ~f->mode & LA_WRITE_MODE) return 0;
    int off = f->pos + f->offset;

    fseek(f->stream, off, SEEK_SET);
    /* int res = fwrite(txt, len, 1, f->stream); */

    /*int res = 1;
    fprintf(f->stream, "%*s", len, txt);*/
    int res = fwrite(txt, len, 1, f->stream);
    int p = ftell(f->stream);

    fseek(f->stream, 0, SEEK_END);
    f->h.size = ftell(f->stream);
    fseek(f->stream, p, SEEK_SET);
    f->offset = p - f->pos;

    return res;
}

int la_frewrite(la_File *f, const char *txt, int len) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!txt) return 0;

    if (len < 0) len = strlen(txt);

    if (~f->mode & LA_WRITE_MODE && ~f->mode & LA_REWRITE_MODE) return 0;
    int off = f->pos + f->offset;
    int start_sz = f->offset - f->pos;

    char sbuff[start_sz];
    la_fread(f, sbuff, start_sz);

    int mode = f->mode;

    la_freopen(f, NULL, LA_REWRITE_MODE);
    la_freopen(f, NULL, mode);

    la_fwrite(f, sbuff, start_sz);
    la_fwrite(f, txt, len);

    /* fseek(f->stream, off, SEEK_SET); */

    return 0;
}

int la_finsert(la_File *f, const char *txt, int len) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!txt) return 0;

    if (~f->mode & LA_WRITE_MODE && ~f->mode & LA_REWRITE_MODE) return 0;

    return 1;
}

int la_fappend(la_File *f, const char *txt, int len) {
    LA_ASSERT(f != NULL, "la_File cannot be NULL");
    if (!txt) return 0;

    if (~f->mode & LA_WRITE_MODE && ~f->mode & LA_REWRITE_MODE) return 0;

    la_fseek(f, f->h.size);
    la_fwrite(f, txt, len);

    return 1;
}

/*laFile* la_open(const char *filename, int mode) {
    int off = 0;
    char smode[3] = "r";
    if (mode & LA_READ_MODE) off += 1;
    if (mode & LA_BINARY_MODE) 

    return NULL;
}*/

long la_size(const char *filename, FILE **out) {
    FILE *fp;
    fp = fopen(filename, "rb");
    if (!fp) {
	fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }

    long sz;

    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (!out) fclose(fp);
    else *out = fp;

    return sz;
}

int la_header(const char *filepath, la_Header *out) {
    if (!filepath) return 0;
    if (!out) return 0;
    char file[100];
    la_resolve_path(filepath, file);
    return _header(file, out);
}

int la_read(const char *filename, char **out, int *len) {
    if (!filename) {
	printf("filename cannot be NULL\n");
        return 0;
    }

    /* int filename_len = latte()->basedir_len + strlen(filename); */

    FILE *fp;
    long buf_len;

    buf_len = la_size(filename, &fp);

    printf("size: %ld\n", buf_len);
    char *buf = (char*)malloc(buf_len+1);
    if (!buf) printf("failed to alloc mem for %s\n", filename); 
    fread(buf, buf_len, 1, fp);
    buf[buf_len] = '\0';
    printf("ok\n");

    if (fp) fclose(fp);
    if (out) *out = buf;

    if (len) *len = buf_len + 1;

    return 1;
}

int la_write(const char *filename, const char *text, int sz) {
    if (!filename) {
	fprintf(stderr, "filename cannot be NULL\n");
	return 0;
    }

    /*FILE *fp;
    fp = fopen(filename, "wb");
    fwrite(text, sz, 1, fp);

    fclose(fp);*/
    la_File *f = la_fopen(filename, LA_WRITE_MODE);
    la_fwrite(f, text, sz);
    la_fclose(f);

   return 1; 
}

int la_touch(const char *filename) {
    if (!filename) {
	fprintf(stderr, "filename cannot be NULL\n");
	return 0;
    }
    if (la_isfile(filename)) return 1;
    FILE *fp;
    fp = fopen(filename, "wb");
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

int la_isfile(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "rb");

    if (!fp) return 0;

    fclose(fp);
    return 1;
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
    return 0;
}

/* Directory */

int la_dheader(la_Dir *dir, la_Header *out) {
    if (!dir) return 0;
    if (!out) return 0;

    memcpy(out, &dir->h, sizeof(*out));

    return 1;
}

la_Dir* la_dopen(const char *path) {
    LA_ASSERT(path != NULL, "cannot open directory");
    la_Dir *dir = (la_Dir*)malloc(sizeof(*dir));
    char p[100];
    la_resolve_path(path, p);

    dir->stream = opendir(p);

    _header(p, &dir->h);
    return dir;
}

void la_dclose(la_Dir *dir) {
    if (!dir) return;
    if (dir->stream) closedir(dir->stream);

    free(dir);
}

void la_dlist(la_Dir *dir) {
    struct dirent *drent;
    while ((drent = readdir(dir->stream))) {
	printf("%s\n", drent->d_name);
    }
}

int la_dread(la_Dir *dir, la_Header *out) {
    if (!dir) return 0;
    if (!out) return 0;
    struct dirent *d;
    
    d = readdir(dir->stream);
    if (!d) return 0;

    char file[100];
    /* la_resolve_path(dir->h.name, file);*/
    strcpy(file, dir->h.name);
    strcat(file, "/");
    strcat(file, d->d_name);
    _header(file, out);
    return 1;
}

/* Virtual FileSystem */

static Node* _create_node();

LA_API la_VirtDrive* la_virt_create(int mode) {
    la_VirtDrive *drv = (la_VirtDrive*)malloc(sizeof(*drv));
    drv->root = NULL; 
    drv->mode = mode;
    drv->stream = NULL;
    return drv;
}

LA_API la_VirtDrive* la_virt_mount(const char *filepath, int mode, int usage) {
    
}

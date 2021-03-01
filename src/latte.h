#ifndef LATTE_H
#define LATTE_H

#define LA_API extern

#define LATTE_VERSION "0.1.0"
#define LA_VERSION "0.1.0"

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
	LA_READ          = (     0),
	LA_WRITE         = (1 << 0),
	LA_APPEND        = (1 << 1),
	LA_BINARY_MODE   = (1 << 2),
};

typedef struct Latte Latte;
typedef struct VirtDrive VirtDrive;
typedef struct VirtFile VirtFile;
typedef struct VirtDir VirtDir;


struct VirtFile {
	char name[100];
	int type;
	unsigned offset;
	long size;

	void *stream;
	void *data;
	void *userdata;
};

struct VirtDir {
	long size;
	void *stream;
	VirtFile *files;
};

LA_API void la_set_basepath(const char *basepath);
LA_API int la_init(const char *basepath);
LA_API void la_deinit();

LA_API long la_size(const char *filename);

/* FileSystem */

LA_API int la_read(const char *filename, char *out, int *sz);
LA_API int la_write(const char *filename, const char *text, int sz);

LA_API int la_touch(const char *filename);
LA_API int la_rm(const char *filename);

LA_API int la_mkdir(const char *path);
LA_API int la_rmdir(const char *path);

LA_API int la_isfile(const char *filename);
LA_API int la_isdir(const char *path);

/* File */

LA_API VirtFile la_file_open(const char *path, int mode);
LA_API void la_file_close(VirtFile *f);

LA_API void la_file_seek(VirtFile *f, unsigned offset);
LA_API void la_file_read(VirtFile *f, char *out, unsigned bytes);
LA_API void la_file_write(VirtFile *f, const char *text, unsigned bytes);

/* Virtual File System */

LA_API int la_virt_mount(VirtDrive *drv, const char *path, int usage);
LA_API void la_virt_close(VirtDrive *drv);

LA_API void la_virt_store(VirtDrive *drv, const char *filename);

LA_API int la_virt_read(VirtDrive *drv, const char *filename, char *out, int *sz);
LA_API int la_virt_write(VirtDrive *drv, const char *filename, const char *text, int *sz);

LA_API int la_virt_touch(VirtDrive *drv, const char *filename);
LA_API int la_virt_rm(VirtDrive *drv, const char *filename);

LA_API void la_virt_mkdir(VirtDrive *drv, const char *path);
LA_API void la_virt_rmdir(VirtDrive *drv, const char *path);

LA_API int la_virt_isfile(VirtDrive *drv, const char *filename);
LA_API int la_virt_isdir(VirtDrive *drv, const char *path);

LA_API VirtDrive* lt_vfs_create(const char *name);
LA_API VirtDrive* lt_vfs_load(const char *filename);
LA_API int lt_vfs_store(VirtDrive *fs);

LA_API int lt_vfs_file_exists(const char *filename);


#endif /* LATTE_H */
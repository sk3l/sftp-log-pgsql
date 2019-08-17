#ifndef SFTP_PGSQL_PLUGIN_STORE_H
#define SFTP_PGSQL_PLUGIN_STORE_H

#include <sys/types.h>

#include "sftp-common.h"

extern "C" int sftp_cf_open_file(u_int32_t, const char *,  u_int32_t, u_int32_t, int *);
extern "C" int sftp_cf_open_dir(u_int32_t, const char*, int *);
extern "C" int sftp_cf_close(u_int32_t, int);
extern "C" int sftp_cf_read(u_int32_t, const char*, int, u_int64_t, u_int32_t, u_char *, int *);
extern "C" int sftp_cf_read_dir(u_int32_t, const char*, StatList *);
extern "C" int sftp_cf_write(u_int32_t, const char*, int, u_int64_t, u_int32_t, u_char *, int *);
extern "C" int sftp_cf_remove(u_int32_t, const char*);
extern "C" int sftp_cf_rename(u_int32_t, const char*, const char *);
extern "C" int sftp_cf_mkdir(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cf_rmdir(u_int32_t, const char*);
extern "C" int sftp_cf_stat(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cf_lstat(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cf_fstat(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cf_setstat(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cf_fsetstat(u_int32_t, const char*, Attrib *);
extern "C" int sftp_cbk_read_link(u_int32_t, const char *, Stat *);
extern "C" int sftp_cbk_symlink(u_int32_t, const char *, const char *);
extern "C" int sftp_cbk_lock(u_int32_t, const char *, u_int64_t, u_int64_t,int);
extern "C" int sftp_cbk_unlock(u_int32_t, const char *, u_int64_t, u_int64_t);
extern "C" int sftp_cbk_realpath(u_int32_t, const char *, Stat *);

#endif

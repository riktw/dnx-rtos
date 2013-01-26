#ifndef VFS_H_
#define VFS_H_
/*=============================================================================================*//**
@file    vfs.h

@author  Daniel Zorychta

@brief   This file support virtual file system

@note    Copyright (C) 2012 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


*//*==============================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                            Include files
==================================================================================================*/
#include "systypes.h"


/*==================================================================================================
                                 Exported symbolic constants/macros
==================================================================================================*/
/* set position equal to offset bytes */
#define VFS_SEEK_SET    0

/* set position to current location plus offset */
#define VFS_SEEK_CUR    1

/* set position to EOF plus offset */
#define VFS_SEEK_END    2


/* translate functions to STDC */
#ifndef SEEK_SET
#define SEEK_SET                          VFS_SEEK_SET
#endif

#ifndef SEEK_CUR
#define SEEK_CUR                          VFS_SEEK_CUR
#endif

#ifndef SEEK_END
#define SEEK_END                          VFS_SEEK_END
#endif


/*==================================================================================================
                                  Exported types, enums definitions
==================================================================================================*/
struct vfs_stat {
     u32_t  st_dev;           /* ID of device containing file */
     u32_t  st_rdev;          /* device ID (if special file) */
     u32_t  st_mode;          /* protection */
     u32_t  st_uid;           /* user ID of owner */
     u32_t  st_gid;           /* group ID of owner */
     size_t st_size;          /* total size, in bytes */
     u32_t  st_mtime;         /* time of last modification */
};

struct vfs_statfs {
      u32_t f_type;           /* file system type */
      u32_t f_blocks;         /* total blocks */
      u32_t f_bfree;          /* free blocks */
      u32_t f_files;          /* total file nodes in FS */
      u32_t f_ffree;          /* free file nodes in FS */
      const ch_t *fsname;     /* FS name */
};

/* Structure describing a mount table entry.  */
struct vfs_mntent {
      ch_t *mnt_fsname;       /* device or server for filesystem.  */
      ch_t *mnt_dir;          /* directory mounted on.  */
      u32_t total;            /* device total size */
      u32_t free;             /* device free */
};

struct vfs_drvcfg {
      devx_t   dev;
      fd_t     part;
      stdRet_t (*f_open )(devx_t dev, fd_t part);
      stdRet_t (*f_close)(devx_t dev, fd_t part);
      size_t   (*f_write)(devx_t dev, fd_t part, void *src, size_t size, size_t nitems, size_t seek);
      size_t   (*f_read )(devx_t dev, fd_t part, void *dst, size_t size, size_t nitems, size_t seek);
      stdRet_t (*f_ioctl)(devx_t dev, fd_t part, IORq_t iorq, void *data);
};

struct vfs_fscfg {
      fsd_t    f_fsd;
      stdRet_t (*f_init   )(const ch_t *path, fsd_t *fsd);
      stdRet_t (*f_open   )(fsd_t fsd, fd_t *fd, size_t *seek, const ch_t *path, const ch_t *mode);
      stdRet_t (*f_close  )(fsd_t fsd, fd_t fd);
      size_t   (*f_write  )(fsd_t fsd, fd_t fd, void *src, size_t size, size_t nitems, size_t seek);
      size_t   (*f_read   )(fsd_t fsd, fd_t fd, void *dst, size_t size, size_t nitems, size_t seek);
      stdRet_t (*f_ioctl  )(fsd_t fsd, fd_t fd, IORq_t iroq, void *data);
      stdRet_t (*f_fstat  )(fsd_t fsd, fd_t fd, struct vfs_stat *stat);
      stdRet_t (*f_mkdir  )(fsd_t fsd, const ch_t *path);
      stdRet_t (*f_mknod  )(fsd_t fsd, const ch_t *path, struct vfs_drvcfg *dcfg);
      stdRet_t (*f_opendir)(fsd_t fsd, const ch_t *path, DIR_t *dir);
      stdRet_t (*f_remove )(fsd_t fsd, const ch_t *path);
      stdRet_t (*f_rename )(fsd_t fsd, const ch_t *oldName, const ch_t *newName);
      stdRet_t (*f_chmod  )(fsd_t fsd, const ch_t *path, u32_t mode);
      stdRet_t (*f_chown  )(fsd_t fsd, const ch_t *path, u16_t owner, u16_t group);
      stdRet_t (*f_stat   )(fsd_t fsd, const ch_t *path, struct vfs_stat *stat);
      stdRet_t (*f_statfs )(fsd_t fsd, struct vfs_statfs *statfs);
      stdRet_t (*f_release)(fsd_t fsd);
};


/*==================================================================================================
                                     Exported function prototypes
==================================================================================================*/
extern stdRet_t vfs_init(void);
extern stdRet_t vfs_mount(const ch_t *srcPath, const ch_t *mntPoint, struct vfs_fscfg *mountcfg);
extern stdRet_t vfs_umount(const ch_t *path);
extern stdRet_t vfs_getmntentry(size_t item, struct vfs_mntent *mntent);
extern stdRet_t vfs_mknod(const ch_t *path, struct vfs_drvcfg *drvcfg);
extern stdRet_t vfs_mkdir(const ch_t *path);
extern DIR_t   *vfs_opendir(const ch_t *path);
extern stdRet_t vfs_closedir(DIR_t *dir);
extern dirent_t vfs_readdir(DIR_t *dir);
extern stdRet_t vfs_remove(const ch_t *path);
extern stdRet_t vfs_rename(const ch_t *oldName, const ch_t *newName);
extern stdRet_t vfs_chmod(const ch_t *path, u16_t mode);
extern stdRet_t vfs_chown(const ch_t *path, u16_t owner, u16_t group);
extern stdRet_t vfs_stat(const ch_t *path, struct vfs_stat *stat);
extern stdRet_t vfs_statfs(const ch_t *path, struct vfs_statfs *statfs);
extern FILE_t  *vfs_fopen(const ch_t *path, const ch_t *mode);
extern stdRet_t vfs_fclose(FILE_t *file);
extern size_t   vfs_fwrite(void *ptr, size_t size, size_t nitems, FILE_t *file);
extern size_t   vfs_fread(void *ptr, size_t size, size_t nitems, FILE_t *file);
extern stdRet_t vfs_fseek(FILE_t *file, i32_t offset, i32_t mode);
extern i32_t    vfs_ftell(FILE_t *file);
extern stdRet_t vfs_ioctl(FILE_t *file, IORq_t rq, void *data);
extern stdRet_t vfs_fstat(FILE_t *file, struct vfs_stat *stat);


#ifdef __cplusplus
}
#endif

#endif /* VFS_H_ */
/*==================================================================================================
                                            End of file
==================================================================================================*/
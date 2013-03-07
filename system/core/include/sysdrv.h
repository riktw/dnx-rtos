#ifndef SYSDRV_H_
#define SYSDRV_H_
/*=========================================================================*//**
@file    sysdrv.h

@author  Daniel Zorychta

@brief   This function provide all required function needed to write drivers.

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


*//*==========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "systypes.h"
#include "memman.h"
#include "oswrap.h"
#include "vfs.h"
#include "taskmoni.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
#ifdef DNX_H_
#error "dnx.h and sysdrv.h shall never included together!"
#endif

#ifndef calloc
#define calloc(nmemb, msize)              memman_calloc(nmemb, msize)
#endif

#ifndef malloc
#define malloc(size)                      memman_malloc(size)
#endif

#ifndef free
#define free(mem)                         memman_free(mem)
#endif

#define mount(path, fs_cfgPtr)            vfs_mount(path, fs_cfgPtr)
#define umount(path)                      vfs_umount(path)
#define getmntentry(item, mntentPtr)      vfs_getmntentry(item, mntentPtr)
#define mknod(path, drv_cfgPtr)           vfs_mknod(path, drv_cfgPtr)
#define mkdir(path)                       vfs_mkdir(path)
#define opendir(path)                     tskm_opendir(path)
#define closedir(dir)                     tskm_closedir(dir)
#define readdir(dir)                      vfs_readdir(dir)
#define remove(path)                      vfs_remove(path)
#define rename(oldName, newName)          vfs_rename(oldName, newName)
#define chmod(path, mode)                 vfs_chmod(path, mode)
#define chown(path, owner, group)         vfs_chown(path, owner, group)
#define stat(path, statPtr)               vfs_stat(path, statPtr)
#define statfs(path, statfsPtr)           vfs_statfs(path, statfsPtr)
#define fopen(path, mode)                 tskm_fopen(path, mode)
#define fclose(file)                      tskm_fclose(file)
#define fwrite(ptr, isize, nitems, file)  vfs_fwrite(ptr, isize, nitems, file)
#define fread(ptr, isize, nitems, file)   vfs_fread(ptr, isize, nitems, file)
#define fseek(file, offset, mode)         vfs_fseek(file, offset, mode)
#define ftell(file)                       vfs_ftell(file)
#define ioctl(file, rq, data)             vfs_ioctl(file, rq, data)
#define fstat(file, statPtr)              vfs_fstat(file, stat)

#define DRIVER_INTERFACE(classname)                                                 \
extern stdRet_t classname##_Init   (devx_t, fd_t);                                  \
extern stdRet_t classname##_Open   (devx_t, fd_t);                                  \
extern stdRet_t classname##_Close  (devx_t, fd_t);                                  \
extern size_t   classname##_Write  (devx_t, fd_t, void*, size_t, size_t, size_t);   \
extern size_t   classname##_Read   (devx_t, fd_t, void*, size_t, size_t, size_t);   \
extern stdRet_t classname##_IOCtl  (devx_t, fd_t, IORq_t, void*);                   \
extern stdRet_t classname##_Release(devx_t, fd_t)

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* SYSDRV_H_ */
/*==============================================================================
  End of file
==============================================================================*/

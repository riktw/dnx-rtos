/*=========================================================================*//**
@file    lfs.c

@author  Daniel Zorychta

@brief   This file support list file system

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
#include "fs/lfs.h"
#include "core/list.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
/* wait time for operation on FS */
#define MTX_BLOCK_TIME                    10

#define force_lock_recursive_mutex(mtx)   while (lock_recursive_mutex(mtx, MTX_BLOCK_TIME) != MUTEX_LOCKED)

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
/** file types */
enum node_type {
        NODE_TYPE_DIR  = FILE_TYPE_DIR,
        NODE_TYPE_FILE = FILE_TYPE_REGULAR,
        NODE_TYPE_DRV  = FILE_TYPE_DRV,
        NODE_TYPE_LINK = FILE_TYPE_LINK
};

/** node structure */
typedef struct node {
        char            *name;                  /* file name                 */
        enum node_type   type;                  /* file type                 */
        fd_t             fd;                    /* file desriptor            */
        u32_t            mode;                  /* protection                */
        u32_t            uid;                   /* user ID of owner          */
        u32_t            gid;                   /* group ID of owner         */
        u64_t            size;                  /* file size                 */
        u32_t            mtime;                 /* time of last modification */
        void            *data;                  /* file type specified data  */
} node_t;

/** info of opened file */
struct opened_file_info {
        node_t          *node;                  /* opened node                    */
        node_t          *base_node;             /* base of opened node            */
        bool             remove_at_close;       /* file to remove after close     */
        u32_t            item_ID;               /* item ID in base directory list */
};

/** main memory structure */
struct LFS_data {
        node_t           root_dir;              /* root dir '/'           */
        mutex_t         *resource_mtx;          /* lock mutex             */
        list_t          *list_of_opended_files; /* list with opened files */
        u32_t            id_counter;            /* list ID counter        */
};

/*==============================================================================
  Local function prototypes
==============================================================================*/
static node_t   *new_node(struct LFS_data*, node_t*, char*, i32_t*);
static stdret_t  delete_node(node_t*, node_t*, u32_t);
static node_t   *get_node(const char*, node_t*, i32_t, i32_t*);
static uint      get_path_deep(const char*);
static dirent_t  lfs_readdir(void*, dir_t*);
static stdret_t  lfs_closedir(void*, dir_t*);
static stdret_t  add_node_to_list_of_open_files(struct LFS_data*, node_t*, node_t*, i32_t*);

/*==============================================================================
  Local object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Initialize file system
 *
 * @param[out] **fshdl          pointer to allocated memory by file system
 * @param[in]  *src_path        file source path
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_init(void **fshdl, const char *src_path)
{
        (void)src_path;

        _stop_if(!fshdl);

        struct LFS_data *lfs;
        if (!(lfs = calloc(1, sizeof(struct LFS_data)))) {
                return STD_RET_ERROR;
        }

        lfs->resource_mtx  = new_mutex();
        lfs->root_dir.data = new_list();
        lfs->list_of_opended_files = new_list();

        if (!lfs->resource_mtx || !lfs->root_dir.data || !lfs->list_of_opended_files) {
                if (lfs->resource_mtx) {
                        delete_mutex(lfs->resource_mtx);
                }

                if (lfs->root_dir.data) {
                        delete_list(lfs->root_dir.data);
                }

                if (lfs->list_of_opended_files) {
                        delete_list(lfs->list_of_opended_files);
                }

                free(lfs);
                return STD_RET_ERROR;
        } else {
                lfs->root_dir.name = "/";
                lfs->root_dir.size = sizeof(node_t);
                lfs->root_dir.type = NODE_TYPE_DIR;

                *fshdl = lfs;
                return STD_RET_OK;
        }
}

//==============================================================================
/**
 * @brief Function release file system
 *
 * @param[in] *fshdl            FS handle
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_release(void *fshdl)
{
        (void)fshdl;

        /*
         * Here the LFS should delete all lists and free all allocated buffers.
         * If will be necessary should be implemented in the future.
         */

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function create node for driver file
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *path             path when driver-file shall be created
 * @param[in] *drv_if           pointer to driver interface
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_mknod(void *fshdl, const char *path, struct vfs_drv_interface *drv_if)
{
        _stop_if(!fshdl);
        _stop_if(!path);
        _stop_if(!drv_if);

        struct LFS_data *lfs = fshdl;

        if (path[0] != '/') {
                return STD_RET_ERROR;
        }

        force_lock_recursive_mutex(lfs->resource_mtx);
        node_t *node     = get_node(path, &lfs->root_dir, -1, NULL);
        node_t *drv_node = get_node(strrchr(path, '/'), node, 0, NULL);

        /* directory must exist and driver's file not */
        if (!node || drv_node) {
                goto error;
        }

        if (node->type != NODE_TYPE_DIR) {
                goto error;
        }

        char *drv_name     = strrchr(path, '/') + 1;
        uint  drv_name_len = strlen(drv_name);

        char *drv_file_name;
        if ((drv_file_name = calloc(drv_name_len + 1, sizeof(char)))) {
                strcpy(drv_file_name, drv_name);

                node_t                   *drv_file      = calloc(1, sizeof(node_t));
                struct vfs_drv_interface *drv_interface = calloc(1, sizeof(struct vfs_drv_interface));

                if (drv_file && drv_interface) {
                        memcpy(drv_interface, drv_if, sizeof(struct vfs_drv_interface));

                        drv_file->name = drv_file_name;
                        drv_file->size = 0;
                        drv_file->type = NODE_TYPE_DRV;
                        drv_file->data = drv_interface;
                        drv_file->fd   = 0;

                        /* add new driver to this folder */
                        if (list_add_item(node->data, lfs->id_counter++, drv_file) >= 0) {
                                unlock_recursive_mutex(lfs->resource_mtx);
                                return STD_RET_OK;
                        }
                }

                /* free memory when error */
                if (drv_file) {
                        free(drv_file);
                }

                if (drv_interface) {
                        free(drv_interface);
                }

                free(drv_file_name);
        }

error:
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Create directory
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *path             path to new directory
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_mkdir(void *fshdl, const char *path)
{
        _stop_if(!fshdl);
        _stop_if(!path);

        struct LFS_data *lfs = fshdl;
        char *new_dir_name   = NULL;

        if (path[0] != '/') {
                return STD_RET_ERROR;
        }

        force_lock_recursive_mutex(lfs->resource_mtx);
        node_t *base_node = get_node(path, &lfs->root_dir, -1, NULL);
        node_t *file_node = get_node(strrchr(path, '/'), base_node, 0, NULL);

        /* base node must exist and created node not */
        if (base_node == NULL || file_node != NULL) {
                goto error;
        }

        if (base_node->type != NODE_TYPE_DIR) {
                goto error;
        }

        char *dir_name_ptr = strrchr(path, '/') + 1;
        uint  dir_name_len = strlen(dir_name_ptr);

        if ((new_dir_name = calloc(dir_name_len + 1, sizeof(char)))) {
                strcpy(new_dir_name, dir_name_ptr);

                node_t *new_dir;
                if (!(new_dir = calloc(1, sizeof(node_t)))) {
                        goto error;
                }

                if ((new_dir->data = new_list())) {
                        new_dir->name = new_dir_name;
                        new_dir->size = sizeof(node_t);
                        new_dir->type = NODE_TYPE_DIR;

                        /* add new folder to this folder */
                        if (list_add_item(base_node->data, lfs->id_counter++, new_dir) >= 0) {
                                unlock_recursive_mutex(lfs->resource_mtx);
                                return STD_RET_OK;
                        } else {
                                delete_list(new_dir->data);
                        }
                } else {
                        free(new_dir);
                }
        }

error:
        if (new_dir_name) {
                free(new_dir_name);
        }

        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function open directory
 *
 * @param[in]  *fshdl           FS handle
 * @param[in]  *path            directory path
 * @param[out] *dir             directory info
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_opendir(void *fshdl, const char *path, dir_t *dir)
{
        _stop_if(!fshdl);
        _stop_if(!path);
        _stop_if(!dir);

        struct LFS_data *lfs = fshdl;
        force_lock_recursive_mutex(lfs->resource_mtx);

        /* go to target dir */
        node_t *node;
        if ((node = get_node(path, &lfs->root_dir, 0, NULL))) {
                if (node->type == NODE_TYPE_DIR) {
                        dir->items = list_get_item_count(node->data);
                        dir->rddir = lfs_readdir;
                        dir->cldir = lfs_closedir;
                        dir->seek  = 0;
                        dir->dd    = node;

                        unlock_recursive_mutex(lfs->resource_mtx);
                        return STD_RET_OK;
                }
        }

        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function close dir
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *dir              directory info
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
static stdret_t lfs_closedir(void *fshdl, dir_t *dir)
{
        (void) fshdl;
        (void) dir;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Function read next item of opened directory
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *dir              directory object
 *
 * @return element attributes
 */
//==============================================================================
static dirent_t lfs_readdir(void *fshdl, dir_t *dir)
{
        _stop_if(!fshdl);
        _stop_if(!dir);

        struct LFS_data *lfs = fshdl;

        dirent_t dirent;
        dirent.name = NULL;
        dirent.size = 0;

        force_lock_recursive_mutex(lfs->resource_mtx);

        node_t *from = dir->dd;
        node_t *node = list_get_nitem_data(from->data, dir->seek++);

        if (node) {
                if (node->type == NODE_TYPE_DRV) {
                        struct vfs_drv_interface *drv_if = node->data;
                        struct vfs_dev_info dev_info;
                        dev_info.st_size = 0;
                        drv_if->drv_info(drv_if->handle, &dev_info);
                        node->size = dev_info.st_size;
                }

                dirent.filetype = node->type;
                dirent.name     = node->name;
                dirent.size     = node->size;
        }

        unlock_recursive_mutex(lfs->resource_mtx);

        return dirent;
}

//==============================================================================
/**
 * @brief Remove file
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *patch            localization of file/directory
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_remove(void *fshdl, const char *path)
{
        _stop_if(!fshdl);
        _stop_if(!path);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);
        i32_t   item;
        bool    remove_file = TRUE;
        node_t *base_node   = get_node(path, &lfs->root_dir, -1, NULL);
        node_t *obj_node    = get_node(path, &lfs->root_dir, 0, &item);

        if (!base_node || !obj_node || obj_node == &lfs->root_dir) {
                goto error;
        }

        /* if path is ending on slash, the object must be DIR */
        if (path[strlen(path) - 1] == '/') {
                if (obj_node->type != NODE_TYPE_DIR) {
                        goto error;
                }
        }

        /* check if file is opened */
        if (obj_node->type != NODE_TYPE_DIR) {
                i32_t n = list_get_item_count(lfs->list_of_opended_files);

                for (int i = 0; i < n; i++) {
                        struct opened_file_info *opened_file = list_get_nitem_data(lfs->list_of_opended_files, i);
                        if (opened_file->node == obj_node) {
                                opened_file->remove_at_close = TRUE;
                                remove_file = FALSE;
                        }
                }
        }

        /* remove node if possible */
        if (remove_file == TRUE) {
                u32_t item_ID;
                if (list_get_nitem_ID(base_node->data, item, &item_ID) == STD_RET_OK) {
                        unlock_recursive_mutex(lfs->resource_mtx);
                        return delete_node(base_node, obj_node, item_ID);
                }
        } else {
                unlock_recursive_mutex(lfs->resource_mtx);
                return STD_RET_OK;
        }

error:
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Rename file name
 *
 * @param[in] *fshdl                FS handle
 * @param[in] *oldName              old file name
 * @param[in] *newName              new file name
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_rename(void *fshdl, const char *old_name, const char *new_name)
{
        _stop_if(!fshdl);
        _stop_if(!old_name);
        _stop_if(!new_name);

        struct LFS_data *lfs = fshdl;
        char *new_node_name  = NULL;

        force_lock_recursive_mutex(lfs->resource_mtx);
        node_t *old_node_base = get_node(old_name, &lfs->root_dir, -1, NULL);
        node_t *new_node_base = get_node(new_name, &lfs->root_dir, -1, NULL);

        if (!old_node_base || !new_node_base) {
                goto error;
        }

        if (old_node_base != new_node_base) {
                goto error;
        }

        if (old_name[0] != '/' || new_name[0] != '/') {
                goto error;
        }

        if (  old_name[strlen(old_name) - 1] == '/'
           || new_name[strlen(new_name) - 1] == '/') {
                goto error;
        }

        new_node_name = calloc(1, strlen(strrchr(new_name, '/') + 1));
        node_t *node  = get_node(old_name, &lfs->root_dir, 0, NULL);

        if (new_node_name && node) {
                strcpy(new_node_name, strrchr(new_name, '/') + 1);

                if (node->name) {
                        free(node->name);
                }

                node->name = new_node_name;

                if (node->type == NODE_TYPE_DIR) {
                        node->size = sizeof(node_t);
                }

                unlock_recursive_mutex(lfs->resource_mtx);
                return STD_RET_OK;
        }

error:
        if (new_node_name) {
                free(new_node_name);
        }

        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function change file mode
 *
 * @param[in] *fshdl                FS handle
 * @param[in] *path                 path
 * @param[in]  mode                 file mode
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_chmod(void *fshdl, const char *path, int mode)
{
        _stop_if(!fshdl);
        _stop_if(!path);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        node_t *node = get_node(path, &lfs->root_dir, 0, NULL);
        if (node) {
                node->mode = mode;
                unlock_recursive_mutex(lfs->resource_mtx);
                return STD_RET_OK;
        }

        unlock_recursive_mutex(lfs->resource_mtx);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function change file owner and group
 *
 * @param[in] *fshdl                FS handle
 * @param[in] *path                 path
 * @param[in]  owner                file owner
 * @param[in]  group                file group
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_chown(void *fshdl, const char *path, int owner, int group)
{
        _stop_if(!fshdl);
        _stop_if(!path);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        node_t *node = get_node(path, &lfs->root_dir, 0, NULL);
        if (node) {
                node->uid = owner;
                node->gid = group;

                unlock_recursive_mutex(lfs->resource_mtx);
                return STD_RET_OK;
        }

        unlock_recursive_mutex(lfs->resource_mtx);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function returns file/dir status
 *
 * @param[in]  *fshdl                FS handle
 * @param[in]  *path                 file/dir path
 * @param[out] *stat                 pointer to stat structure
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_stat(void *fshdl, const char *path, struct vfs_stat *stat)
{
        _stop_if(!fshdl);
        _stop_if(!path);
        _stop_if(!stat);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        node_t *node = get_node(path, &lfs->root_dir, 0, NULL);
        if (node) {
                if ( (path[strlen(path) - 1] == '/' && node->type == NODE_TYPE_DIR)
                   || path[strlen(path) - 1] != '/') {

                        if (node->type == NODE_TYPE_DRV) {
                                struct vfs_drv_interface *drv_if = node->data;
                                struct vfs_dev_info dev_info;
                                dev_info.st_size = 0;
                                drv_if->drv_info(drv_if->handle, &dev_info);
                                node->size = dev_info.st_size;
                        }

                        stat->st_dev   = node->fd;
                        stat->st_gid   = node->gid;
                        stat->st_mode  = node->mode;
                        stat->st_mtime = node->mtime;
                        stat->st_size  = node->size;
                        stat->st_uid   = node->uid;

                        unlock_recursive_mutex(lfs->resource_mtx);
                        return STD_RET_OK;
                }
        }

        unlock_recursive_mutex(lfs->resource_mtx);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function returns file status
 *
 * @param[in]  *fshdl                FS handle
 * @param[in]  *extra                file extra data (useful in FS wrappers)
 * @param[in]   fd                   file descriptor
 * @param[out] *stat                 pointer to status structure
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_fstat(void *fshdl, void *extra, fd_t fd, struct vfs_stat *stat)
{
        (void)extra;

        _stop_if(!fshdl);
        _stop_if(!stat);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (opened_file) {
                if (opened_file->node) {
                        if (opened_file->node->type == NODE_TYPE_DRV) {
                                struct vfs_drv_interface *drv_if = opened_file->node->data;
                                struct vfs_dev_info dev_info;
                                dev_info.st_size = 0;
                                drv_if->drv_info(drv_if->handle, &dev_info);
                                opened_file->node->size = dev_info.st_size;
                        }

                        stat->st_dev   = opened_file->node->fd;
                        stat->st_gid   = opened_file->node->gid;
                        stat->st_mode  = opened_file->node->mode;
                        stat->st_mtime = opened_file->node->mtime;
                        stat->st_size  = opened_file->node->size;
                        stat->st_uid   = opened_file->node->uid;

                        unlock_recursive_mutex(lfs->resource_mtx);
                        return STD_RET_OK;
                }
        }

        unlock_recursive_mutex(lfs->resource_mtx);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function returns FS status
 *
 * @param[in]  *fshdl               FS handle
 * @param[out] *statfs              pointer to status structure
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_statfs(void *fshdl, struct vfs_statfs *statfs)
{
        (void)fshdl;

        _stop_if(!statfs);

        statfs->f_bfree  = 0;
        statfs->f_blocks = 0;
        statfs->f_ffree  = 0;
        statfs->f_files  = 0;
        statfs->f_type   = 0x01;
        statfs->fsname   = "lfs";

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Function open selected file
 *
 * @param[in]  *fshdl           FS handle
 * @param[in]  *extra           file extra data (useful in FS wrappers)
 * @param[out] *fd              file descriptor
 * @param[out] *lseek           file position
 * @param[in]  *path            file path
 * @param[in]  *mode            file mode
 *
 * @retval STD_RET_OK           file opened/created
 * @retval STD_RET_ERROR        file not opened/created
 */
//==============================================================================
stdret_t lfs_open(void *fshdl, void **extra, fd_t *fd, u64_t *lseek, const char *path, const char *mode)
{
        (void)extra;

        _stop_if(!fshdl);
        _stop_if(!fd);
        _stop_if(!lseek);
        _stop_if(!path);
        _stop_if(!mode);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        i32_t   item;
        node_t *node      = get_node(path, &lfs->root_dir, 0, &item);
        node_t *base_node = get_node(path, &lfs->root_dir, -1, NULL);

        /* create new file when necessary */
        if (base_node && node == NULL) {
                if (  (strncmp("w",  mode, 2) != 0)
                   && (strncmp("w+", mode, 2) != 0)
                   && (strncmp("a",  mode, 2) != 0)
                   && (strncmp("a+", mode, 2) != 0) ) {
                        goto error;
                }

                char *file_name = calloc(1, strlen(strrchr(path, '/')));
                if (file_name == NULL) {
                        goto error;
                }

                strcpy(file_name, strrchr(path, '/') + 1);
                node = new_node(lfs, base_node, file_name, &item);

                if (node == NULL) {
                        free(file_name);
                        goto error;
                }
        }

        /* file shall exist */
        if (!node || !base_node || item < 0) {
                goto error;
        }

        /* node must be a file */
        if (node->type == NODE_TYPE_DIR) {
                goto error;
        }

        /* add file to list of open files */
        if (add_node_to_list_of_open_files(lfs, base_node, node, &item) == STD_RET_ERROR) {
                goto error;
        }

        /* set file parameters */
        if (node->type == NODE_TYPE_FILE) {
                /* set seek at begin if selected */
                if (  strncmp("r",  mode, 2) == 0
                   || strncmp("r+", mode, 2) == 0
                   || strncmp("w",  mode, 2) == 0
                   || strncmp("w+", mode, 2) == 0 ) {
                        *lseek = 0;
                }

                /* set file size */
                if (  strncmp("w",  mode, 2) == 0
                   || strncmp("w+", mode, 2) == 0 ) {

                        if (node->data) {
                                free(node->data);
                                node->data = NULL;
                        }

                        node->size = 0;
                }

                /* set seek at file end */
                if (strncmp("a", mode, 2) == 0 || strncmp("a+", mode, 2) == 0) {
                        *lseek = node->size;
                }
        } else if (node->type == NODE_TYPE_DRV) {
                struct vfs_drv_interface *drv = node->data;

                if (!drv->drv_open) {
                        goto error;
                }

                if (drv->drv_open(drv->handle) == STD_RET_OK) {
                        *lseek = 0;
                } else {
                        list_rm_nitem(lfs->list_of_opended_files, item);
                        goto error;
                }
        }

        /* everything success - load FD */
        u32_t cfd;
        list_get_nitem_ID(lfs->list_of_opended_files, item, &cfd);
        *fd = (fd_t)cfd;
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_OK;

error:
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function close file in LFS
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *extra            file extra data (useful in FS wrappers)
 * @param[in]  fd               file descriptor
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_close(void *fshdl, void *extra, fd_t fd)
{
        (void)extra;

        _stop_if(!fshdl);

        struct LFS_data *lfs = fshdl;
        stdret_t status      = STD_RET_ERROR;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (opened_file == NULL) {
                goto exit;
        }

        node_t *node = opened_file->node;
        if (node == NULL) {
                goto exit;
        }

        /* close device if file is driver type */
        if (node->type == NODE_TYPE_DRV) {
                if (node->data == NULL) {
                        goto exit;
                }

                struct vfs_drv_interface *drv_if = node->data;
                if (!drv_if->drv_close) {
                        goto exit;
                }

                if ((status = drv_if->drv_close(drv_if->handle)) != STD_RET_OK) {
                        goto exit;
                }
        }

        /* delete file from open list */
        struct opened_file_info opened_file_data = *opened_file;

        if (list_rm_iditem(lfs->list_of_opended_files, fd) != STD_RET_OK) {
                /* critical error! */
                goto exit;
        }

        /* file to remove, check if other task does not opens this file */
        status = STD_RET_OK;

        if (opened_file_data.remove_at_close == TRUE) {
                i32_t n = list_get_item_count(lfs->list_of_opended_files);

                for (int i = 0; i < n; i++) {
                        opened_file = list_get_nitem_data(lfs->list_of_opended_files, i);

                        if (opened_file->node == node) {
                                goto exit;
                        }
                }

                /* file can be removed */
                status = delete_node(opened_file_data.base_node,
                                     opened_file_data.node,
                                     opened_file_data.item_ID);
        }

        exit:
        unlock_recursive_mutex(lfs->resource_mtx);
        return status;
}

//==============================================================================
/**
 * @brief Function write data to the file
 *
 * @param[in] *fshdl            FS handle
 * @param[in] *extra            file extra data (useful in FS wrappers)
 * @param[in]  fd               file descriptor
 * @param[in] *src              data source
 * @param[in]  size             item size
 * @param[in]  nitems           number of items
 * @param[in]  lseek            position in file
 *
 * @return number of written items
 */
//==============================================================================
size_t lfs_write(void *fshdl, void *extra, fd_t fd, const void *src, size_t size, size_t nitems, u64_t lseek)
{
        (void)extra;

        _stop_if(!fshdl);
        _stop_if(!src);
        _stop_if(!size);
        _stop_if(!nitems);

        struct LFS_data *lfs = fshdl;
        size_t           n   = 0;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (!opened_file) {
                goto exit;
        }

        node_t *node = opened_file->node;
        if (!node) {
                goto exit;
        }

        if (node->type == NODE_TYPE_DRV && node->data) {
                struct vfs_drv_interface *drv_if = node->data;

                if (drv_if->drv_write) {
                        unlock_recursive_mutex(lfs->resource_mtx);

                        return drv_if->drv_write(drv_if->handle, src, size, nitems, lseek);
                }
        } else if (node->type == NODE_TYPE_FILE) {
                size_t write_size  = size * nitems;
                size_t file_length = node->size;
                size_t seek        = lseek > SIZE_MAX ? SIZE_MAX : lseek;

                if (seek > file_length) {
                        seek = file_length;
                }

                if ((seek + write_size) > file_length || node->data == NULL) {
                        char *new_data = malloc(file_length + write_size);
                        if (new_data == NULL) {
                                goto exit;
                        }

                        if (node->data) {
                                memcpy(new_data, node->data, file_length);
                                free(node->data);
                        }

                        memcpy(new_data + seek, src, write_size);

                        node->data  = new_data;
                        node->size += write_size - (file_length - seek);

                        n = nitems;
                } else {
                        memcpy(node->data + seek, src, write_size);
                        n = nitems;
                }
        }

exit:
        unlock_recursive_mutex(lfs->resource_mtx);
        return n;
}

//==============================================================================
/**
 * @brief Function read from file data
 *
 * @param[in]  *fshdl           FS handle
 * @param[in]  *extra           file extra data (useful in FS wrappers)
 * @param[in]   fd              file descriptor
 * @param[out] *dst             data destination
 * @param[in]   size            item size
 * @param[in]   nitems          number of items
 * @param[in]   lseek           position in file
 *
 * @return number of read items
 */
//==============================================================================
size_t lfs_read(void *fshdl, void *extra, fd_t fd, void *dst, size_t size, size_t nitems, u64_t lseek)
{
        (void)extra;

        _stop_if(!fshdl);
        _stop_if(!dst);
        _stop_if(!size);
        _stop_if(!nitems);

        struct LFS_data *lfs = fshdl;
        size_t           n   = 0;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (opened_file == NULL) {
                goto exit;
        }

        node_t *node = opened_file->node;
        if (node == NULL) {
                goto exit;
        }

        if (node->type == NODE_TYPE_DRV && node->data) {
                struct vfs_drv_interface *drv_if = node->data;

                if (drv_if->drv_read) {
                        unlock_recursive_mutex(lfs->resource_mtx);
                        return drv_if->drv_read(drv_if->handle, dst, size, nitems, lseek);
                }
        } else if (node->type == NODE_TYPE_FILE) {
                size_t file_length = node->size;
                size_t seek        = lseek > SIZE_MAX ? SIZE_MAX : lseek;

                /* check if seek is not bigger than file length */
                if (seek > file_length) {
                        seek = file_length;
                }

                /* check how many items to read is on current file position */
                size_t items_to_read;
                if (((file_length - seek) / size) >= nitems) {
                        items_to_read = nitems;
                } else {
                        items_to_read = (file_length - seek) / size;
                }

                /* copy if file buffer exist */
                if (node->data) {
                        if (items_to_read > 0) {
                                memcpy(dst, node->data + seek, items_to_read * size);
                                n = items_to_read;
                        }
                }
        }

exit:
        unlock_recursive_mutex(lfs->resource_mtx);
        return n;
}

//==============================================================================
/**
 * @brief IO operations on files
 *
 * @param[in]     *fshdl        FS handle
 * @param[in]     *extra        file extra data (useful in FS wrappers)
 * @param[in]      fd           file descriptor
 * @param[in]      iorq         request
 * @param[in,out]  args         additional arguments
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_ioctl(void *fshdl, void *extra, fd_t fd, int iorq, va_list args)
{
        (void)extra;

        _stop_if(!fshdl);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (opened_file == NULL) {
                goto error;
        }

        if (opened_file->node == NULL) {
                goto error;
        }

        if (opened_file->node->type == NODE_TYPE_DRV && opened_file->node->data) {
                struct vfs_drv_interface *drv_if = opened_file->node->data;

                if (drv_if->drv_ioctl) {
                        unlock_recursive_mutex(lfs->resource_mtx);
                        return drv_if->drv_ioctl(drv_if->handle, iorq, args);
                }
        }

error:
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function flush file data
 *
 * @param[in]     *fshdl        FS handle
 * @param[in]     *extra        file extra data (useful in FS wrappers)
 * @param[in]      fd           file descriptor
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t lfs_flush(void *fshdl, void *extra, fd_t fd)
{
        (void)extra;

        _stop_if(!fshdl);

        struct LFS_data *lfs = fshdl;

        force_lock_recursive_mutex(lfs->resource_mtx);

        struct opened_file_info *opened_file = list_get_iditem_data(lfs->list_of_opended_files, fd);
        if (opened_file == NULL) {
                goto error;
        }

        if (opened_file->node == NULL) {
                goto error;
        }

        if (opened_file->node->type == NODE_TYPE_DRV && opened_file->node->data) {
                struct vfs_drv_interface *drv_if = opened_file->node->data;

                if (drv_if->drv_flush) {
                        unlock_recursive_mutex(lfs->resource_mtx);
                        return drv_if->drv_flush(drv_if->handle);
                }
        }

error:
        unlock_recursive_mutex(lfs->resource_mtx);
        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Remove selected node
 *
 * @param[in] *base             base node
 * @param[in] *target           target node
 * @param[in]  baseitemid       item in base node that point to target
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
static stdret_t delete_node(node_t *base, node_t *target, u32_t baseitemid)
{
        if (target->type == NODE_TYPE_DIR) {

                if (list_get_item_count(target->data) > 0) {
                        return STD_RET_ERROR;
                } else {
                        delete_list(target->data);
                        target->data = NULL;
                }
        }

        if (target->name) {
                free(target->name);
        }

        if (target->data) {
                free(target->data);
        }

        if (list_rm_iditem(base->data, baseitemid) == STD_RET_OK) {
                return STD_RET_OK;
        }

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Check path deep
 *
 * @param[in] *path             path
 *
 * @return path deep
 */
//==============================================================================
static uint get_path_deep(const char *path)
{
        uint deep = 0;

        if (path[0] == '/') {
                const char *last_path = path++;

                while ((path = strchr(path, '/'))) {
                        last_path = path;
                        path++;
                        deep++;
                }

                if (last_path[1] != '\0') {
                        deep++;
                }
        }

        return deep;
}

//==============================================================================
/**
 * @brief Function find node by path
 *
 * @param[in]  *path          path
 * @param[in]  *startnode     start node
 * @param[out] **extPath      external path begin (pointer from path)
 * @param[in]  deep           deep control
 * @param[out] *item          node is n-item of list which was found
 *
 * @return node
 */
//==============================================================================
static node_t *get_node(const char *path, node_t *startnode, i32_t deep, i32_t *item)
{
        if (!path || !startnode) {
                return NULL;
        }

        if (startnode->type != NODE_TYPE_DIR) {
                return NULL;
        }

        node_t *current_node = startnode;
        int     dir_deep     = get_path_deep(path);

        /* go to selected node -----------------------------------------------*/
        while (dir_deep + deep > 0) {
                /* get element from path */
                if ((path = strchr(path, '/')) == NULL) {
                        break;
                } else {
                        path++;
                }

                uint  path_length;
                char *path_end;
                if ((path_end = strchr(path, '/')) == NULL) {
                        path_length = strlen(path);
                } else {
                        path_length = path_end - path;
                }

                /* get number of list items */
                int list_size = list_get_item_count(current_node->data);

                /* find that object exist ------------------------------------*/
                int i = 0;
                while (list_size > 0) {
                        node_t *next_node = list_get_nitem_data(current_node->data, i++);

                        if (next_node == NULL) {
                                dir_deep = 1 - deep;
                                break;
                        }

                        if (  strlen(next_node->name) == path_length
                           && strncmp(next_node->name, path, path_length) == 0 ) {

                                current_node = next_node;

                                if (item) {
                                        *item = i - 1;
                                }

                                break;
                        }

                        list_size--;
                }

                /* directory does not found or error */
                if (list_size == 0 || current_node == NULL) {
                        current_node = NULL;
                        break;
                }

                dir_deep--;
        }

        return current_node;
}

//==============================================================================
/**
 * @brief Function create new file in selected node
 * Function allocate new node. If node is created successfully then filename
 * cannot be freed!
 *
 * @param [in] *lfs             pointer to current LFS instance
 * @param [in] *nodebase        node base
 * @param [in] *filename        filename (must be earlier allocated)
 * @param[out] *item            new node number in base node
 *
 * @return NULL if error, otherwise new node address
 */
//==============================================================================
static node_t *new_node(struct LFS_data *lfs, node_t *nodebase, char *filename, i32_t *item)
{
        if (!nodebase || !filename) {
                return NULL;
        }

        if (nodebase->type != NODE_TYPE_DIR) {
                return NULL;
        }

        node_t *node;
        if ((node = calloc(1, sizeof(node_t))) == NULL) {
                return NULL;
        }

        node->name  = filename;
        node->data  = NULL;
        node->gid   = 0;
        node->mode  = 0;
        node->mtime = 0;
        node->size  = 0;
        node->type  = NODE_TYPE_FILE;
        node->uid   = 0;

        i32_t   node_number;
        if ((node_number = list_add_item(nodebase->data, lfs->id_counter++, node)) < 0) {
                free(node);
                return NULL;
        }

        *item = node_number;
        return node;
}

//==============================================================================
/**
 * @brief Function add node to list of open files
 *
 * @param [in] *lfs             pointer to current LFS instance
 * @param [in] *base_node       base node
 * @param [in] *node            node data added to list of open files
 * @param [io] *item            in:  node number in base node
 *                              out: open file's number in list of open files
 *
 * @retval STD_RET_OK           file registered in list of open files
 * @retval STD_RET_ERROR        file not registered
 */
//==============================================================================
static stdret_t add_node_to_list_of_open_files(struct LFS_data *lfs, node_t *base_node,
                                               node_t *node, i32_t *item)
{
        struct opened_file_info *opened_file_info;
        if (!(opened_file_info = calloc(1, sizeof(struct opened_file_info)))) {
                return STD_RET_ERROR;
        }

        opened_file_info->remove_at_close = FALSE;
        opened_file_info->node      = node;
        opened_file_info->base_node = base_node;

        if (list_get_nitem_ID(base_node->data, *item, &opened_file_info->item_ID) != STD_RET_OK) {
                goto error;
        }

        /* find if file shall be removed */
        i32_t open_file_count = list_get_item_count(lfs->list_of_opended_files);

        for (i32_t i = 0; i < open_file_count; i++) {
                struct opened_file_info *opened_file = list_get_nitem_data(lfs->list_of_opended_files, i);

                if (opened_file->node != node) {
                        continue;
                }

                if (opened_file->remove_at_close == TRUE) {
                        opened_file_info->remove_at_close = TRUE;
                        break;
                }
        }

        /* add open file info to list */
        i32_t open_file_list_position = list_add_item(lfs->list_of_opended_files,
                                                      lfs->id_counter++,
                                                      opened_file_info);

        if (open_file_list_position >= 0) {
                *item = open_file_list_position;
                return STD_RET_OK;
        }

error:
        free(opened_file_info);
        return STD_RET_ERROR;
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
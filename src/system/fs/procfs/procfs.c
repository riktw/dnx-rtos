#include "system/thread.h"
static stdret_t    procfs_closedir_freedd  (void *fs_handle, DIR *dir);
static stdret_t    procfs_closedir_generic (void *fs_handle, DIR *dir);
static dirent_t    procfs_readdir_root     (void *fs_handle, DIR *dir);
static dirent_t    procfs_readdir_taskname (void *fs_handle, DIR *dir);
static dirent_t    procfs_readdir_bin      (void *fs_handle, DIR *dir);
static dirent_t    procfs_readdir_taskid   (void *fs_handle, DIR *dir);
static dirent_t    procfs_readdir_taskid_n (void *fs_handle, DIR *dir);
static inline void mutex_force_lock        (mutex_t *mtx);
        list_t        *file_list = list_new();
        mutex_t       *mtx       = mutex_new(MUTEX_NORMAL);
                list_delete(file_list);
                mutex_delete(mtx);
        if (mutex_lock(procfs->resource_mtx, 100)) {
                        mutex_unlock(procfs->resource_mtx);
                        errno = EBUSY;
                critical_section_begin();
                mutex_unlock(procfs->resource_mtx);
                mutex_delete(procfs->resource_mtx);
                list_delete(procfs->file_list);
                critical_section_end();
        } else {
                errno = EBUSY;
                errno = EROFS;
                        errno = ENOENT;
                        errno = ENOENT;
                        errno = ENOENT;
                mutex_force_lock(procmem->resource_mtx);
                        mutex_unlock(procmem->resource_mtx);
                mutex_unlock(procmem->resource_mtx);
                        errno = ENOENT;
                        mutex_force_lock(procmem->resource_mtx);
                                mutex_unlock(procmem->resource_mtx);
                        mutex_unlock(procmem->resource_mtx);
        errno = ENOENT;

                mutex_force_lock(procmem->resource_mtx);
                        mutex_unlock(procmem->resource_mtx);
                mutex_unlock(procmem->resource_mtx);
 * @return number of written bytes, -1 if error
        errno = EROFS;

 * @return number of read bytes, -1 if error
        mutex_force_lock(procmem->resource_mtx);
        mutex_unlock(procmem->resource_mtx);
                errno = ENOENT;
                errno = ENOENT;
        errno = EPERM;

        errno = EROFS;

API_FS_FSTAT(procfs, void *fs_handle, void *extra, fd_t fd, struct stat *stat)
        mutex_force_lock(procmem->resource_mtx);
        mutex_unlock(procmem->resource_mtx);
                errno = ENOENT;
                errno = ENOENT;
        stat->st_type  = FILE_TYPE_REGULAR;
 * @param[in ]           mode                   dir mode
API_FS_MKDIR(procfs, void *fs_handle, const char *path, mode_t mode)
        UNUSED_ARG(mode);

        errno = EROFS;

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Create pipe
 *
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of created pipe
 * @param[in ]           mode                   pipe mode
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_FS_MKFIFO(procfs, void *fs_handle, const char *path, mode_t mode)
{
        STOP_IF(!fs_handle);
        STOP_IF(!path);
        UNUSED_ARG(mode);

        /* not supported by this file system */
        errno = EPERM;
        errno = EROFS;

                        errno = ENOENT;
                        errno = ENOENT;
        errno = ENOENT;

        errno = EROFS;

        errno = EROFS;

        errno = EROFS;

        errno = EROFS;

API_FS_STAT(procfs, void *fs_handle, const char *path, struct stat *stat)
        stat->st_type  = FILE_TYPE_REGULAR;
//==============================================================================
/**
 * @brief Force lock mutex
 *
 * @param mtx           mutex
 */
//==============================================================================
static inline void mutex_force_lock(mutex_t *mtx)
{
        while (mutex_lock(mtx, MTX_BLOCK_TIME) != true);
}

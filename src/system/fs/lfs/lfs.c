#include "system/dnxfs.h"
static dirent_t  lfs_readdir                    (void *fs_handle, DIR *dir);
static stdret_t  lfs_closedir                   (void *fs_handle, DIR *dir);
 * @param[out]          **fs_handle             file system allocated memory
 * @param[in ]           *src_path              file source path
API_FS_INIT(lfs, void **fs_handle, const char *src_path)
 * @brief Release file system
 * @param[in ]          *fs_handle              file system allocated memory
API_FS_RELEASE(lfs, void *fs_handle)
 * @brief Create node for driver file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of created node
 * @param[in ]          *drv_if                 driver interface
API_FS_MKNOD(lfs, void *fs_handle, const char *path, const struct vfs_drv_interface *drv_if)
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of created directory
API_FS_MKDIR(lfs, void *fs_handle, const char *path)
 * @brief Open directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of opened directory
 * @param[in ]          *dir                    directory object
API_FS_OPENDIR(lfs, void *fs_handle, const char *path, DIR *dir)
static stdret_t lfs_closedir(void *fs_handle, DIR *dir)
static dirent_t lfs_readdir(void *fs_handle, DIR *dir)
                        struct vfs_dev_stat dev_stat;
                        dev_stat.st_size = 0;
                        drv_if->drv_stat(drv_if->handle, &dev_stat);
                        node->size = dev_stat.st_size;
 * @brief Remove file/directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of removed file/directory
API_FS_REMOVE(lfs, void *fs_handle, const char *path)
 * @brief Rename file/directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *old_name               old object name
 * @param[in ]          *new_name               new object name
API_FS_RENAME(lfs, void *fs_handle, const char *old_name, const char *new_name)
 * @brief Change file's mode
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[in ]           mode                   new file mode
API_FS_CHMOD(lfs, void *fs_handle, const char *path, int mode)
 * @brief Change file's owner and group
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[in ]           owner                  new file owner
 * @param[in ]           group                  new file group
API_FS_CHOWN(lfs, void *fs_handle, const char *path, int owner, int group)
 * @brief Return file/dir status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[out]          *stat                   file status
API_FS_STAT(lfs, void *fs_handle, const char *path, struct vfs_stat *stat)
                                struct vfs_dev_stat dev_stat;
                                dev_stat.st_size = 0;
                                drv_if->drv_stat(drv_if->handle, &dev_stat);
                                node->size = dev_stat.st_size;
                                stat->st_dev = dev_stat.st_major;
                        } else {
                                stat->st_dev = node->fd;
 * @brief Return file status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[out]          *stat                   file status
API_FS_FSTAT(lfs, void *fs_handle, void *extra, fd_t fd, struct vfs_stat *stat)
                                struct vfs_dev_stat dev_stat;
                                dev_stat.st_size = 0;
                                drv_if->drv_stat(drv_if->handle, &dev_stat);
                                opened_file->node->size = dev_stat.st_size;
                                stat->st_dev = dev_stat.st_major;
                        } else {
                                stat->st_dev = opened_file->node->fd;
 * @brief Return file system status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[out]          *statfs                 file system status
API_FS_STATFS(lfs, void *fs_handle, struct vfs_statfs *statfs)
        statfs->f_fsname = "lfs";
 * @brief Open file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[out]          *extra                  file extra data
 * @param[out]          *fd                     file descriptor
 * @param[out]          *fpos                   file position
 * @param[in]           *path                   file path
 * @param[in]            flags                  file open flags (see vfs.h)
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
API_FS_OPEN(lfs, void *fs_handle, void **extra, fd_t *fd, u64_t *fpos, const char *path, int flags)
        STOP_IF(!fpos);
                if (!(flags & O_CREAT)) {
                if ((flags & O_CREAT) && !(flags & O_APPEND)) {
                if (!(flags & O_APPEND)) {
                        *fpos = 0;
                } else {
                        *fpos = node->size;
                if (drv->drv_open(drv->handle, O_DEV_FLAGS(flags)) == STD_RET_OK) {
                        *fpos = 0;
 * @brief Close file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]           force                  force close
 * @param[in ]          *file_owner             task which opened file (valid if force is true)
API_FS_CLOSE(lfs, void *fs_handle, void *extra, fd_t fd, bool force, const task_t *file_owner)
                if ((status = drv_if->drv_close(drv_if->handle, force, file_owner)) != STD_RET_OK) {
exit:
 * @brief Write data to the file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]          *src                    data source
 * @param[in ]           count                  number of bytes to write
 * @param[in ]          *fpos                   position in file

 * @return number of written bytes
API_FS_WRITE(lfs, void *fs_handle, void *extra, fd_t fd, const u8_t *src, size_t count, u64_t *fpos)
        STOP_IF(!fpos);
        STOP_IF(!count);
                        return drv_if->drv_write(drv_if->handle, src, count, fpos);
                size_t write_size  = count;
                size_t seek        = *fpos > SIZE_MAX ? SIZE_MAX : *fpos;
                        n = count;
                        n = count;
 * @brief Read data from file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[out]          *dst                    data destination
 * @param[in ]           count                  number of bytes to read
 * @param[in ]          *fpos                   position in file

 * @return number of read bytes
API_FS_READ(lfs, void *fs_handle, void *extra, fd_t fd, u8_t *dst, size_t count, u64_t *fpos)
        STOP_IF(!fpos);
        STOP_IF(!count);
                        return drv_if->drv_read(drv_if->handle, dst, count, fpos);
                size_t seek        = *fpos > SIZE_MAX ? SIZE_MAX : *fpos;
                if ((file_length - seek) >= count) {
                        items_to_read = count;
                        items_to_read = file_length - seek;
                                memcpy(dst, node->data + seek, items_to_read);
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]           request                request
 * @param[in ][out]     *arg                    request's argument
 * @retval ...
API_FS_IOCTL(lfs, void *fs_handle, void *extra, fd_t fd, int request, void *arg)
                        return drv_if->drv_ioctl(drv_if->handle, request, arg);
 * @brief Flush file data
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
API_FS_FLUSH(lfs, void *fs_handle, void *extra, fd_t fd)
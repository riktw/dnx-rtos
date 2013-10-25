#include "system/dnxfs.h"
static stdret_t fatfs_closedir(void *fs_handle, DIR *dir);
static dirent_t fatfs_readdir (void *fs_handle, DIR *dir);
 * @param[out]          **fs_handle             file system allocated memory
 * @param[in ]           *src_path              file source path
API_FS_INIT(fatfs, void **fs_handle, const char *src_path)
 * @brief Release file system
 * @param[in ]          *fs_handle              file system allocated memory
API_FS_RELEASE(fatfs, void *fs_handle)
 * @brief Open file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[out]          *extra                  file extra data
 * @param[out]          *fd                     file descriptor
 * @param[out]          *fpos                   file position
 * @param[in]           *path                   file path
 * @param[in]            flags                  file open flags (see vfs.h)
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
API_FS_OPEN(fatfs, void *fs_handle, void **extra, fd_t *fd, u64_t *fpos, const char *path, int flags)
        STOP_IF(!fpos);
        if (flags == O_RDONLY) {
        } else if (flags == O_RDWR) {
        } else if (flags == (O_WRONLY | O_CREAT)) {
        } else if (flags == (O_RDWR | O_CREAT)) {
        } else if (flags == (O_WRONLY | O_APPEND | O_CREAT)) {
        } else if (flags == (O_RDWR | O_APPEND | O_CREAT)) {
        } else {
                free(fat_file);
                return STD_RET_ERROR;
        if (flags & O_APPEND) {
                *fpos = libfat_size(fat_file);
                *fpos = 0;
 * @brief Close file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]           force                  force close
 * @param[in ]          *file_owner             task which opened file (valid if force is true)
API_FS_CLOSE(fatfs, void *fs_handle, void *extra, fd_t fd, bool force, task_t *file_owner)
        UNUSED_ARG(force);
        UNUSED_ARG(file_owner);
 * @brief Write data to the file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]          *src                    data source
 * @param[in ]           count                  number of bytes to write
 * @param[in ]          *fpos                   position in file

 * @return number of written bytes
API_FS_WRITE(fatfs, void *fs_handle, void *extra, fd_t fd, const u8_t *src, size_t count, u64_t *fpos)
        STOP_IF(!count);
        STOP_IF(!fpos);
        if (libfat_tell(fat_file) != (u32_t)*fpos) {
                libfat_lseek(fat_file, (u32_t)*fpos);
        libfat_write(fat_file, src, count, &n);
        return n;
 * @brief Read data from file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[out]          *dst                    data destination
 * @param[in ]           count                  number of bytes to read
 * @param[in ]          *fpos                   position in file

 * @return number of read bytes
API_FS_READ(fatfs, void *fs_handle, void *extra, fd_t fd, void *dst, size_t count, u64_t *fpos)
        STOP_IF(!count);
        STOP_IF(!fpos);
        if (libfat_tell(fat_file) != (u32_t)*fpos) {
                libfat_lseek(fat_file, (u32_t)*fpos);
        libfat_read(fat_file, dst, count, &n);
        return n;
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[in ]           request                request
 * @param[in ][out]     *arg                    request's argument
 * @retval ...
API_FS_IOCTL(fatfs, void *fs_handle, void *extra, fd_t fd, int request, void *arg)
        UNUSED_ARG(request);
        UNUSED_ARG(arg);
 * @brief Flush file data
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
API_FS_FLUSH(fatfs, void *fs_handle, void *extra, fd_t fd)
 * @brief Return file status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *extra                  file extra data
 * @param[in ]           fd                     file descriptor
 * @param[out]          *stat                   file status
API_FS_FSTAT(fatfs, void *fs_handle, void *extra, fd_t fd, struct vfs_stat *stat)
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of created directory
API_FS_MKDIR(fatfs, void *fs_handle, const char *path)
 * @brief Create node for driver file
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of created node
 * @param[in ]          *drv_if                 driver interface
API_FS_MKNOD(fatfs, void *fs_handle, const char *path, const struct vfs_drv_interface *drv_if)
 * @brief Open directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of opened directory
 * @param[in ]          *dir                    directory object
API_FS_OPENDIR(fatfs, void *fs_handle, const char *path, DIR *dir)
static stdret_t fatfs_closedir(void *fs_handle, DIR *dir)
static dirent_t fatfs_readdir(void *fs_handle, DIR *dir)
 * @brief Remove file/directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   name of removed file/directory
API_FS_REMOVE(fatfs, void *fs_handle, const char *path)
 * @brief Rename file/directory
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *old_name               old object name
 * @param[in ]          *new_name               new object name
API_FS_RENAME(fatfs, void *fs_handle, const char *old_name, const char *new_name)
 * @brief Change file's mode
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[in ]           mode                   new file mode
API_FS_CHMOD(fatfs, void *fs_handle, const char *path, int mode)
        uint8_t dosmode = mode & S_IWUSR ? 0 : LIBFAT_AM_RDO;
 * @brief Change file's owner and group
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[in ]           owner                  new file owner
 * @param[in ]           group                  new file group
API_FS_CHOWN(fatfs, void *fs_handle, const char *path, int owner, int group)
 * @brief Return file/dir status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[in ]          *path                   file path
 * @param[out]          *stat                   file status
API_FS_STAT(fatfs, void *fs_handle, const char *path, struct vfs_stat *stat)
 * @brief Return file system status
 * @param[in ]          *fs_handle              file system allocated memory
 * @param[out]          *statfs                 file system status
API_FS_STATFS(fatfs, void *fs_handle, struct vfs_statfs *statfs)
                        statfs->f_fsname = "fatfs (FAT12)";
                        statfs->f_fsname = "fatfs (FAT16)";
                        statfs->f_fsname = "fatfs (FAT32)";
                        statfs->f_fsname = "fatfs (FAT\?\?)";
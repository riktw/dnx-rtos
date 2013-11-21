#include "system/thread.h"
#define MTX_BLOCK_TIME                  10
#define PIPE_LENGTH                     CONFIG_FSCANF_STREAM_BUFFER_SIZE
#define PIPE_WRITE_TIMEOUT              MAX_DELAY
#define PIPE_READ_TIMEOUT               MAX_DELAY
        NODE_TYPE_LINK = FILE_TYPE_LINK,
        NODE_TYPE_PIPE = FILE_TYPE_PIPE
        mode_t           mode;                  /* protection                */
static inline char      get_first_char                  (const char *str);
static inline char      get_last_char                   (const char *str);
static void             mutex_force_lock                (mutex_t *mtx);
static node_t          *new_node                        (struct LFS_data *lfs, node_t *nodebase, char *filename, i32_t *item);
static stdret_t         delete_node                     (node_t *base, node_t *target, u32_t baseitemid);
static node_t          *get_node                        (const char *path, node_t *startnode, i32_t deep, i32_t *item);
static uint             get_path_deep                   (const char *path);
static dirent_t         lfs_readdir                     (void *fs_handle, DIR *dir);
static stdret_t         lfs_closedir                    (void *fs_handle, DIR *dir);
static stdret_t         add_node_to_list_of_open_files  (struct LFS_data *lfs, node_t *base_node, node_t *node, i32_t *item);
        lfs->resource_mtx  = mutex_new(MUTEX_RECURSIVE);
        lfs->root_dir.data = list_new();
        lfs->list_of_opended_files = list_new();
                        mutex_delete(lfs->resource_mtx);
                        list_delete(lfs->root_dir.data);
                        list_delete(lfs->list_of_opended_files);
        errno = EPERM;

        mutex_force_lock(lfs->resource_mtx);
                                mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
 * @param[in ]           mode                   dir mode
API_FS_MKDIR(lfs, void *fs_handle, const char *path, mode_t mode)
        mutex_force_lock(lfs->resource_mtx);
        if (base_node == NULL) {
                goto error;
        }

        if (file_node != NULL) {
                errno = EEXIST;
                if ((new_dir->data = list_new())) {
                        new_dir->mode = mode;
                                mutex_unlock(lfs->resource_mtx);
                                list_delete(new_dir->data);
        mutex_unlock(lfs->resource_mtx);
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
API_FS_MKFIFO(lfs, void *fs_handle, const char *path, mode_t mode)
{
        STOP_IF(!fs_handle);
        STOP_IF(!path);

        struct LFS_data *lfs = fs_handle;

        if (FIRST_CHARACTER(path) != '/') {
                errno = ENOENT;
                return STD_RET_ERROR;
        }

        mutex_force_lock(lfs->resource_mtx);
        node_t *dir_node  = get_node(path, &lfs->root_dir, -1, NULL);
        node_t *fifo_node = get_node(strrchr(path, '/'), dir_node, 0, NULL);

        /* directory must exist and driver's file not */
        if (!dir_node) {
                goto error;
        }

        if (fifo_node) {
                errno = EEXIST;
                goto error;
        }

        if (dir_node->type != NODE_TYPE_DIR) {
                goto error;
        }

        char *fifo_name     = strrchr(path, '/') + 1;
        uint  fifo_name_len = strlen(fifo_name);

        char *fifo_file_name = calloc(fifo_name_len + 1, sizeof(char));
        if (fifo_file_name) {
                strcpy(fifo_file_name, fifo_name);

                node_t  *fifo_file  = calloc(1, sizeof(node_t));
                queue_t *fifo_queue = queue_new(PIPE_LENGTH, sizeof(u8_t));

                if (fifo_file && fifo_queue) {

                        fifo_file->name = fifo_file_name;
                        fifo_file->size = 0;
                        fifo_file->type = NODE_TYPE_PIPE;
                        fifo_file->data = fifo_queue;
                        fifo_file->fd   = 0;
                        fifo_file->mode = mode;

                        /* add pipe to folder */
                        if (list_add_item(dir_node->data, lfs->id_counter++, fifo_file) >= 0) {
                                mutex_unlock(lfs->resource_mtx);
                                return STD_RET_OK;
                        }
                }

                /* free memory when error */
                if (fifo_file) {
                        free(fifo_file);
                }

                if (fifo_queue) {
                        queue_delete(fifo_queue);
                }

                free(fifo_file_name);
        }

error:
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
                        mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
        } else {
                errno = ENOENT;
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
        if (get_last_char(path) == '/') {
                        errno = ENOTDIR;
                        mutex_unlock(lfs->resource_mtx);
                } else {
                        errno = ENOENT;
                mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
        if (get_first_char(old_name) != '/' || get_first_char(new_name) != '/') {
        if (get_last_char(old_name) == '/' || get_last_char(new_name) == '/') {
                mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
                mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
                mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
API_FS_STAT(lfs, void *fs_handle, const char *path, struct stat *stat)
        mutex_force_lock(lfs->resource_mtx);
                if ( (get_last_char(path) == '/' && node->type == NODE_TYPE_DIR)
                   || get_last_char(path) != '/') {
                                node->size   = dev_stat.st_size;
                        stat->st_type  = node->type;
                        mutex_unlock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
API_FS_FSTAT(lfs, void *fs_handle, void *extra, fd_t fd, struct stat *stat)
        mutex_force_lock(lfs->resource_mtx);
                        stat->st_type  = opened_file->node->type;
                        mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;

        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
        mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
        errno = ENOENT;

        mutex_unlock(lfs->resource_mtx);
 * @return number of written bytes, -1 if error
        mutex_force_lock(lfs->resource_mtx);
                errno = ENOENT;
                errno = ENOENT;
                        mutex_unlock(lfs->resource_mtx);
                                errno = ENOSPC;
        } else if (node->type == NODE_TYPE_PIPE) {
                mutex_unlock(lfs->resource_mtx);
                if (node->data) {
                        for (uint i = 0; i < count; i++) {
                                if (queue_send(node->data, src + i, PIPE_WRITE_TIMEOUT)) {
                                        n++;
                                } else {
                                        i--;
                                }
                        }

                        critical_section_begin();
                        node->size += n;
                        critical_section_end();
                        return n;
                } else {
                        errno = EIO;
                }
        mutex_unlock(lfs->resource_mtx);
 * @return number of read bytes, -1 if error
        mutex_force_lock(lfs->resource_mtx);
                        mutex_unlock(lfs->resource_mtx);
        } else if (node->type == NODE_TYPE_PIPE) {
                mutex_unlock(lfs->resource_mtx);
                if (node->data) {
                        for (uint i = 0; i < count; i++) {
                                if (queue_receive(node->data, dst + i, PIPE_READ_TIMEOUT)) {
                                        n++;
                                }
                        }

                        critical_section_begin();
                        node->size -= n;
                        critical_section_end();
                        return n;
                } else {
                        errno = EIO;
                }
        errno = ENOENT;
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
                        mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        mutex_unlock(lfs->resource_mtx);
        mutex_force_lock(lfs->resource_mtx);
                        mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        mutex_unlock(lfs->resource_mtx);
//==============================================================================
/**
 * @brief Return last character of selected string
 */
//==============================================================================
static inline char get_last_char(const char *str)
{
        return LAST_CHARACTER(str);
}

//==============================================================================
/**
 * @brief Return first character of selected string
 */
//==============================================================================
static inline char get_first_char(const char *str)
{
        return FIRST_CHARACTER(str);
}

//==============================================================================
/**
 * @brief Function force lock mutex
 *
 * @param mtx           mutex
 */
//==============================================================================
static void mutex_force_lock(mutex_t *mtx)
{
        while (mutex_lock(mtx, MTX_BLOCK_TIME) != true);
}

                        list_delete(target->data);
                        target->data = NULL;
                }
        } else if (target->type == NODE_TYPE_PIPE) {

                if (target->data) {
                        queue_delete(target->data);
 *        ERRNO: ENOENT
 * @param[in]  path             path
 * @param[in]  startnode        start node
 * @param[out] extPath          external path begin (pointer from path)
 * @param[in]  deep             deep control
 * @param[out] item             node is n-item of list which was found
                errno = ENOENT;
                errno = ENOENT;
                        errno        = ENOENT;
 * ERRNO: ENOENT, ENOTDIR, ENOMEM
                errno = ENOENT;
                errno = ENOTDIR;
                errno = ENOENT;
 *        ERRNO: ENOMEM
        errno = ENOMEM;
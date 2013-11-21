#include "system/thread.h"
        struct devfs_chain *next;
static stdret_t            closedir                     (void *fs_handle, DIR *dir);
static dirent_t            readdir                      (void *fs_handle, DIR *dir);
static struct devfs_chain *chain_new                    (void);
static void                chain_delete                 (struct devfs_chain *chain);
static struct devnode     *chain_get_node_by_path       (struct devfs_chain *chain, const char *path);
static struct devnode     *chain_get_empty_node         (struct devfs_chain *chain);
static struct devnode     *chain_get_n_node             (struct devfs_chain *chain, int n);
        mutex_t            *mtx   = mutex_new(MUTEX_NORMAL);
        struct devfs_chain *chain = chain_new();
                mutex_delete(mtx);
                chain_delete(chain);
        if (mutex_lock(devfs->mutex, 100)) {
                        mutex_unlock(devfs->mutex);
                        errno = EBUSY;
                critical_section_begin();
                mutex_unlock(devfs->mutex);
                chain_delete(devfs->root_chain);
                mutex_delete(devfs->mutex);
                critical_section_end();
        errno = EBUSY;

        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, path);
                mutex_unlock(devfs->mutex);
        } else {
                errno = EBUSY;
                if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                        mutex_unlock(devfs->mutex);
 * @return number of written bytes, -1 if error
 * @return number of read bytes, -1 if error
API_FS_FSTAT(devfs, void *fs_handle, void *extra, fd_t fd, struct stat *stat)
                stat->st_type  = FILE_TYPE_DRV;
 * @param[in ]           mode                   dir mode
API_FS_MKDIR(devfs, void *fs_handle, const char *path, mode_t mode)
        UNUSED_ARG(mode);

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
API_FS_MKFIFO(devfs, void *fs_handle, const char *path, mode_t mode)
{
        STOP_IF(!fs_handle);
        STOP_IF(!path);
        UNUSED_ARG(mode);

        errno = EPERM;
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                        while (chain->next != NULL) {
                                chain = chain->next;
                        chain->next = chain_new();
                        if (!chain->next) {
                                errno = ENOSPC;
                        }
                struct devnode *node = chain_get_empty_node(devfs->root_chain);

                        errno = ENOSPC;
                mutex_unlock(devfs->mutex);
        } else {
                errno = ENOENT;
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_n_node(devfs->root_chain, dir->f_seek);
                mutex_unlock(devfs->mutex);
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, path);
                        } else {
                                errno = EBUSY;
                mutex_unlock(devfs->mutex);
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, old_name);
                        } else {
                                errno = ENOMEM;
                mutex_unlock(devfs->mutex);
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, path);
                mutex_unlock(devfs->mutex);
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, path);
                mutex_unlock(devfs->mutex);
API_FS_STAT(devfs, void *fs_handle, const char *path, struct stat *stat)
        if (mutex_lock(devfs->mutex, TIMEOUT_MS)) {
                struct devnode *node = chain_get_node_by_path(devfs->root_chain, path);
                        stat->st_type  = FILE_TYPE_DRV;
                mutex_unlock(devfs->mutex);
 *        Errno: ENOENT
 * @param[in] chain             chain
 * @param[in] path              node's path
static struct devnode *chain_get_node_by_path(struct devfs_chain *chain, const char *path)
        for (struct devfs_chain *nchain = chain; nchain != NULL; nchain = nchain->next) {
                        if (nchain->devnode[i].drvif == NULL)
                        if (strcmp(nchain->devnode[i].path, path + 1) == 0)
                                return &nchain->devnode[i];
        errno = ENOENT;

 *        Errno: ENOENT
 * @param[in] chain             chain
static struct devnode *chain_get_empty_node(struct devfs_chain *chain)
        for (struct devfs_chain *nchain = chain; nchain != NULL; nchain = nchain->next) {
                        if (nchain->devnode[i].drvif == NULL)
                                return &nchain->devnode[i];
        errno = ENOENT;

 *        Errno: ENOENT
static struct devnode *chain_get_n_node(struct devfs_chain *chain, int n)
        for (struct devfs_chain *nchain = chain; nchain != NULL; nchain = nchain->next) {
                        if (nchain->devnode[i].drvif != NULL) {
                                        return &nchain->devnode[i];
        errno = ENOENT;

static struct devfs_chain *chain_new(void)
static void chain_delete(struct devfs_chain *chain)
        if (chain->next) {
                chain_delete(chain->next);
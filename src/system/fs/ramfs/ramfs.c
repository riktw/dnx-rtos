static int  new_node                    (struct RAMFS *hdl, node_t *parent, char *filename, mode_t mode, i32_t *item, node_t **child);
                hdl->root_dir.mode = (S_IRWXU | S_IRWXG | S_IRWXO) | S_IFDIR;
                                err = new_node(hdl, parent, child_name,
                                               S_IRWXU | S_IRGRP | S_IROTH | S_IFDEV,
                                               NULL, &child);
                                err = new_node(hdl, parent, child_name,
                                               S_IPMT(mode) | S_IFDIR,
                                               NULL, &child);
                                if (err) {
                                err = new_node(hdl, parent, child_name,
                                               S_IPMT(mode) | S_IFIFO,
                                               NULL, &child);
                                if (err) {
                        if (S_ISDIR(parent->mode)) {
                        dir->dirent.d_name = child->name;
                        dir->dirent.mode   = child->mode;
                        dir->dirent.size   = child->size;
                        if (S_ISDEV(child->mode)) {
                if (strlch(path) == '/' && !S_ISDIR(child->mode)) {
                if (!S_ISDIR(child->mode)) {
                        target->mode = S_IFMT(target->mode) | S_IPMT(mode);
 * @brief Return file status
 * @param[in ]          *fhdl                   file handle
API_FS_FSTAT(ramfs, void *fs_handle, void *extra, struct stat *stat)
        UNUSED_ARG1(fs_handle);
        int err = ENOENT;
        struct opened_file_info *opened_file = extra;
        if (opened_file && opened_file->child) {
                stat->st_gid   = opened_file->child->gid;
                stat->st_mode  = opened_file->child->mode;
                stat->st_mtime = opened_file->child->mtime;
                stat->st_ctime = opened_file->child->ctime;
                stat->st_size  = opened_file->child->size;
                stat->st_uid   = opened_file->child->uid;
                if (S_ISDEV(opened_file->child->mode)) {
                        stat->st_dev = opened_file->child->data.dev_t;
                        struct vfs_dev_stat dev_stat;
                        err = sys_driver_stat(opened_file->child->data.dev_t,
                                              &dev_stat);
                        if (!err) {
                                stat->st_size = dev_stat.st_size;
                        } else if (err == ENODEV) {
                                stat->st_size = 0;
                                err = ESUCC;
                } else {
                        stat->st_dev = 0;
                        err          = ESUCC;
                }
 * @brief Return file/dir status
 * @param[in ]          *path                   file path
API_FS_STAT(ramfs, void *fs_handle, const char *path, struct stat *stat)
                node_t *target;
                err = get_node(path, &hdl->root_dir, 0, NULL, &target);
                if (!err) {
                        if ( (strlch(path) == '/' && S_ISDIR(target->mode))
                           || strlch(path) != '/') {
                                struct opened_file_info file;
                                file.child           = target;
                                file.parent          = NULL;
                                file.remove_at_close = false;
                                err = _ramfs_fstat(fs_handle, &file, stat);
                                err = EINVAL;
                        err = new_node(hdl, parent, file_name, 0666 | S_IFREG,
                                       &item, &child);
                        if (S_ISDIR(child->mode)) {
                // load pointer to file descriptor
                *fhdl = sys_llist_back(hdl->opended_files);

                if (S_ISREG(child->mode)) {
                } else if (S_ISDEV(child->mode)) {
                                int pos = sys_llist_find_end(hdl->opended_files, *fhdl);
                                sys_llist_erase(hdl->opended_files, pos);
                        if (S_ISDEV(target->mode)) {
                        } else if (S_ISFIFO(target->mode)) {
                        sys_gettime(&node->mtime);
                        if (S_ISDEV(node->mode)) {
                        } else if (S_ISFIFO(node->mode)) {
                       } else if (S_ISREG(node->mode)) {
                        if (S_ISDEV(node->mode)) {
                        } else if (S_ISFIFO(node->mode)) {
                        } else if (S_ISREG(node->mode)) {
                        if (S_ISDEV(opened_file->child->mode)) {
                        } else if (S_ISFIFO(opened_file->child->mode)) {
                                case IOCTL_PIPE__PERMANENT:
                                        sys_mutex_unlock(hdl->resource_mtx);
                                        return sys_pipe_permanent(opened_file->child->data.pipe_t);

                        if (S_ISDEV(opened_file->child->mode)) {
        if (S_ISDIR(target->mode)) {
        } else if (S_ISFIFO(target->mode)) {
        } else if (S_ISREG(target->mode)) {
        if (!S_ISDIR(startnode->mode)) {
 * @param[in]  mode             mode (permissions, file type)
                    mode_t        mode,
        if (!S_ISDIR(parent->mode)) {
                sys_gettime(&tm);
                node->mode         = mode;
                if (S_ISDIR(mode)) {
                } else if (S_ISFIFO(mode)) {
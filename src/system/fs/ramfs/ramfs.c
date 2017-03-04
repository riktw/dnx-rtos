        char            *name;                  //!< file name
        tfile_t          type;                  //!< file type
        mode_t           mode;                  //!< protection
        uid_t            uid;                   //!< user ID of owner
        gid_t            gid;                   //!< group ID of owner
        size_t           size;                  //!< file size
        time_t           mtime;                 //!< time of last modification
        time_t           ctime;                 //!< time of creation
        void            *data;                  //!< file type specified data
        node_t          *child;                 //!< opened node
        node_t          *parent;                //!< base of opened node
        bool             remove_at_close;       //!< file to remove after close
        node_t           root_dir;              //!< root dir '/'
        mutex_t         *resource_mtx;          //!< lock mutex
        llist_t         *opended_files;         //!< list with opened files
        size_t           file_count;            //!< number of files
 * @param[in ]           *opts                  file system options (can be NULL)
API_FS_INIT(ramfs, void **fs_handle, const char *src_path, const char *opts)
        UNUSED_ARG2(src_path, opts);
        int err = sys_zalloc(sizeof(struct RAMFS), fs_handle);
        if (err == ESUCC) {
                err = sys_mutex_create(MUTEX_TYPE_RECURSIVE, &hdl->resource_mtx);
                if (err != ESUCC)
                err = sys_llist_create(NULL, NULL, cast(llist_t**, &hdl->root_dir.data));
                if (err != ESUCC)
                err = sys_llist_create(sys_llist_functor_cmp_pointers, NULL, &hdl->opended_files);
                if (err != ESUCC)
                if (err != ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, -1, NULL, &parent);
                if (err == ESUCC) {
                        err = sys_zalloc(strsize(basename), cast(void**, &child_name));
                        if (err == ESUCC) {
                                err = new_node(hdl, parent, child_name, FILE_TYPE_DRV, NULL, &child);
                                if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, -1, NULL, &parent);
                if (err == ESUCC) {
                        err = sys_zalloc(strsize(basename), cast(void**, &child_name));
                        if (err == ESUCC) {
                                err = new_node(hdl, parent, child_name, FILE_TYPE_DIR, NULL, &child);
                                if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, -1, NULL, &parent);
                if (err == ESUCC) {
                        err = sys_zalloc(strsize(basename), cast(void**, &child_name));
                        if (err == ESUCC) {
                                err = new_node(hdl, parent, child_name, FILE_TYPE_PIPE, NULL, &child);
                                if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, 0, NULL, &parent);
                if (err == ESUCC) {
                                dir->d_items    = sys_llist_size(parent->data);
                                dir->d_seek     = 0;
                                dir->d_hdl      = parent;
                                err = ENOTDIR;
        return err;
API_FS_CLOSEDIR(ramfs, void *fs_handle, DIR *dir)
API_FS_READDIR(ramfs, void *fs_handle, DIR *dir)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                node_t *parent = dir->d_hdl;
                node_t *child  = sys_llist_at(parent->data, dir->d_seek++);
                                err = sys_driver_stat((dev_t)child->data, &dev_stat);
                                if (err == ESUCC) {
                                } else if (err == ENODEV) {
                                        dir->dirent.size = 0;
                                        err = ESUCC;
                                return err;
                                err = ESUCC;
                        err = ENOENT;
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, -1, NULL, &parent);
                if (err != ESUCC){
                err = get_node(path, &hdl->root_dir, 0, &item, &child);
                if (err != ESUCC) {
                        err = EPERM;
                        err = ENOTDIR;
                        err = delete_node(hdl, parent, child, item);
                        err = ESUCC;
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(old_name, &hdl->root_dir, 0, NULL, &target);
                if (err == ESUCC) {
                        err = sys_zalloc(strsize(basename), cast(void**, &newname));
                        if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, 0, NULL, &target);
                if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, 0, NULL, &target);
                if (err == ESUCC) {
        return err;
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, 0, NULL, &target);
                if (err == ESUCC) {
                                stat->st_ctime = target->ctime;
                                        stat->st_dev = cast(dev_t, target->data);
                                        
                                        err = sys_driver_stat(cast(dev_t, target->data),
                                        if (err == ESUCC) {
                                                
                                        } else if (err == ENODEV) {
                                                stat->st_size = 0;
                                                err = ESUCC;
                                        return err;
                                        stat->st_dev = 0;
                                err = EINVAL;
        return err;
 * @param[in ]          *fhdl                   file handle
API_FS_FSTAT(ramfs, void *fs_handle, void *extra, struct stat *stat)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                        stat->st_ctime = opened_file->child->ctime;
                                stat->st_dev = cast(dev_t, opened_file->child->data);
                                
                                err = sys_driver_stat(cast(dev_t, opened_file->child->data),
                                if (err == ESUCC) {
                                        
                                } else if (err == ENODEV) {
                                        stat->st_size = 0;
                                        err = ESUCC;
                                return err;
                                stat->st_dev = 0;
                                err          = ESUCC;
                        err = ENOENT;
        return err;
        statfs->f_type   = SYS_FS_TYPE__RAM;
 * @param[out]          *fhdl                   file handle
API_FS_OPEN(ramfs, void *fs_handle, void **fhdl, fpos_t *fpos, const char *path, u32_t flags)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = get_node(path, &hdl->root_dir, -1, NULL, &parent);
                if (err != ESUCC) {
                err = get_node(path, &hdl->root_dir, 0, &item, &child);
                if (err == ENOENT) {
                        err = sys_zalloc(strsize(basename), cast(void**, &file_name));
                        if (err != ESUCC) {
                        err = new_node(hdl, parent, file_name, FILE_TYPE_REGULAR, &item, &child);
                        if (err != ESUCC) {
                                err = EISDIR;
                err = add_node_to_open_files_list(hdl, parent, child);
                if (err != ESUCC) {
                        err = sys_driver_open((dev_t)child->data, flags);
                        if (err == ESUCC) {
                *fhdl = sys_llist_back(hdl->opended_files);
        return err;
 * @param[in ]          *fhdl                   file handle
API_FS_CLOSE(ramfs, void *fs_handle, void *fhdl, bool force)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                struct opened_file_info *opened_file = fhdl;
                                err = sys_driver_close((dev_t)target->data, force);
                                err = sys_pipe_close(target->data);
                                                err = delete_node(hdl,
                                        err = ESUCC;
                        if (err == ESUCC) {
                        err = ENOENT;
        return err;
 * @param[in ]          *fhdl                   file handle
             void            *fhdl,
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = ENOENT;
                struct opened_file_info *opened_file = fhdl;
                        sys_get_time(&target->mtime);
                               err = sys_pipe_write(target->data, src, count, wrcnt, fattr.non_blocking_wr);
                               if (err == ESUCC) {
                                               err = sys_pipe_get_length(target->data, &size);
                               return err;
                                       err = sys_malloc(file_length + write_size,
                                       if (err == ESUCC) {
                                               err = ENOSPC;
                                       err = ESUCC;
        return err;
 * @param[in ]          *fhdl                   file handle
            void            *fhdl,
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = ENOENT;
                struct opened_file_info *opened_file = fhdl;
                                err = sys_pipe_read(target->data, dst, count, rdcnt, fattr.non_blocking_rd);
                                if (err == ESUCC) {
                                                err = sys_pipe_get_length(target->data, &size);
                                return err;
                                        err = ESUCC;
                                        err = EIO;
        return err;
 * @param[in ]          *fhdl                   file handle
API_FS_IOCTL(ramfs, void *fs_handle, void *fhdl, int request, void *arg)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                err = ENOENT;
                struct opened_file_info *opened_file = fhdl;
                                        err = EBADRQC;
        return err;
 * @param[in ]          *fhdl                   file handle
API_FS_FLUSH(ramfs, void *fs_handle, void *fhdl)
        int err = sys_mutex_lock(hdl->resource_mtx, MTX_TIMEOUT);
        if (err == ESUCC) {
                struct opened_file_info *opened_file = fhdl;
                                err = ESUCC;
                        err = ENOENT;
        return err;
        int     err          = ENOENT;
                err = ESUCC;
        return err;
        int err = sys_zalloc(sizeof(node_t), cast(void**, &node));
        if (err == ESUCC) {

                time_t tm = 0;
                sys_get_time(&tm);

                node->uid   = 0;
                node->mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
                node->mtime = tm;
                node->ctime = tm;
                        err = sys_llist_create(NULL, NULL, cast(llist_t**, &node->data));
                        err = sys_pipe_create(cast(pipe_t**, &node->data));
                if (err == ESUCC) {
                                err = ENOMEM;
                if (err != ESUCC) {
        return err;
        int err = sys_zalloc(sizeof(struct opened_file_info),
        if (err == ESUCC) {
                        err = ENOMEM;
        return err;
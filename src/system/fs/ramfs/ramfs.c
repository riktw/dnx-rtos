        time_t           mtime;                 /* time of last modification */
                        char *newname;
                        result = sys_zalloc(strsize(basename), cast(void**, &newname));
                                strcpy(newname, basename);

                                target->name = newname;
                        sys_gettime(&target->mtime);

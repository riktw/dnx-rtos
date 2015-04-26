/*=========================================================================*//**
@file    process.h

@author  Daniel Zorychta

@brief   This file support programs layer

@note    Copyright (C) 2012, 2013 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "config.h"
#include "fs/vfs.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
#define _GVAR_STRUCT_NAME               global_variables
#define GLOBAL_VARIABLES                struct _GVAR_STRUCT_NAME  // compability with older versions
#define GLOBAL_VARIABLES_SECTION        GLOBAL_VARIABLES
#define GLOBAL_VARIABLES_SECTION_BEGIN  GLOBAL_VARIABLES {
#define GLOBAL_VARIABLES_SECTION_END    };

#ifdef __cplusplus
        inline void* operator new     (size_t size) {return malloc(size);}
        inline void* operator new[]   (size_t size) {return malloc(size);}
        inline void  operator delete  (void* ptr  ) {free(ptr);}
        inline void  operator delete[](void* ptr  ) {free(ptr);}
#       define _PROGMAN_CXX extern "C"
#       define _PROGMAN_EXTERN_C extern "C"
#else
#       define _PROGMAN_CXX
#       define _PROGMAN_EXTERN_C extern
#endif

#define _IMPORT_PROGRAM(_name_)\
        _PROGMAN_EXTERN_C const size_t __builtin_app_##_name_##_gs__;\
        _PROGMAN_EXTERN_C const size_t __builtin_app_##_name_##_ss__;\
        _PROGMAN_EXTERN_C int __builtin_app_##_name_##_main(int, char**)

#define int_main(_name_, stack_depth, argc, argv)\
        _PROGMAN_CXX const size_t __builtin_app_##_name_##_gs__ = sizeof(struct _GVAR_STRUCT_NAME);\
        _PROGMAN_CXX const size_t __builtin_app_##_name_##_ss__ = stack_depth;\
        _PROGMAN_CXX int __builtin_app_##_name_##_main(argc, argv)

#define _PROGRAM_CONFIG(_name_) \
        {.name          = #_name_,\
         .main          = __builtin_app_##_name_##_main,\
         .globals_size  = &__builtin_app_##_name_##_gs__,\
         .stack_depth   = &__builtin_app_##_name_##_ss__}

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/
typedef int (*process_func_t)(int, char**);

struct _prog_data {
        const char     *name;
        const size_t   *globals_size;
        const size_t   *stack_depth;
        process_func_t  main;
};

typedef struct {
        FILE       *f_stdin;
        FILE       *f_stdout;
        FILE       *f_stderr;
        const char *cwd;
} process_attr_t;

typedef struct {
        size_t stack_depth;
} thread_attr_t;

//typedef struct thread thread_t;

//typedef struct prog prog_t;

/*==============================================================================
  Exported object declarations
==============================================================================*/
extern FILE                     *stdin;
extern FILE                     *stdout;
extern FILE                     *stderr;
extern struct _GVAR_STRUCT_NAME *global;
extern int                      _errno;

/*==============================================================================
  Exported function prototypes
==============================================================================*/
extern int         _process_create                              (pid_t*, process_attr_t*, const char*, const char*);
extern const char *_process_get_CWD                             (void);
extern void        _copy_task_context_to_standard_variables     (void);
extern void        _copy_standard_variables_to_task_context     (void);

/*==============================================================================
  Exported inline functions
==============================================================================*/
static inline const struct _prog_data *_get_programs_table(void)
{
        extern const struct _prog_data _prog_table[];
        return _prog_table;
}

static inline int _get_programs_table_size(void)
{
        extern const int _prog_table_size;
        return _prog_table_size;
}

#ifdef __cplusplus
}
#endif

#endif /* _PROCESS_H_ */
/*==============================================================================
  End of file
==============================================================================*/

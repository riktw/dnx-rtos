/*=========================================================================*//**
@file    oswrap.c

@author  Daniel Zorychta

@brief   Operating system wrapper

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
#include "oswrap.h"
#include "taskmoni.h"
#include "memman.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define calloc(nitems, itemsize)        memman_calloc(nitems, itemsize)
#define free(mem)                       memman_free(mem)

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local object definitions
==============================================================================*/

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Function create new task and if enabled add to monitor list
 *
 * Function by default allocate memory for task data (localized in task tag)
 * which is used to cpu load calculation and standard IO and etc.
 *
 * @param[in ]  func            task code
 * @param[in ] *name            task name
 * @param[in ]  stack_depth     stack deep
 * @param[in ] *argv            argument pointer
 *
 * @return task object pointer or NULL if error
 */
//==============================================================================
task_t *osw_new_task(taskCode_t func, const char *name, u16_t stack_depth, void *argv)
{
        task_t           *task = NULL;
        struct task_data *data;

        data = calloc(1, sizeof(struct task_data));
        if (data == NULL) {
                return NULL;
        }

        data->f_parent_task = get_task_handle();

        vTaskSuspendAll();
        if (xTaskCreate(func, (signed char*)name, stack_depth, argv, PRIORITY(0), &task) == OS_OK) {

                set_task_tag(task, (void*)data);
                tskm_add_task(task);
        } else {
                free(data);
        }
        xTaskResumeAll();

        return task;
}

//==============================================================================
/**
 * @brief Function delete task and if enabled remove from monitor list
 *
 * @param *taskHdl       task handle
 */
//==============================================================================
void osw_delete_task(task_t *taskHdl)
{
        struct task_data *data = get_task_tag(taskHdl);
        if (data) {
                free(data);
        }

        tskm_remove_task(taskHdl);
        vTaskDelete(THIS_TASK);
}

//==============================================================================
/**
 * @brief Function create binary semaphore
 *
 * @return binary semaphore object
 */
//==============================================================================
sem_t *osw_create_binary_semaphore(void)
{
        sem_t *sem = NULL;

        vSemaphoreCreateBinary(sem);

        return sem;
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/

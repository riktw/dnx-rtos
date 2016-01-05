/*=========================================================================*//**
@file    mntent.h

@author  Daniel Zorychta

@brief   Mount entry information.

@note    Copyright (C) 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

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

/**
\defgroup mntent-h <mntent.h>

The library provides information about file system mount entry.

*/
/**@{*/

#ifndef _MNTENT_H_
#define _MNTENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include <sys/types.h>
#include <kernel/syscall.h>

/*==============================================================================
  Exported macros
==============================================================================*/

/*==============================================================================
  Exported object types
==============================================================================*/
#ifdef DOXYGEN
/** @brief Structure that describes a mount table entry. */
struct mntent {
        const char *mnt_fsname; //!< Device or server for file system
        const char *mnt_dir;    //!< Directory mounted on
        u64_t       mnt_total;  //!< Device total size in bytes
        u64_t       mnt_free;   //!< Device free space in bytes
};
#endif

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Exported functions
==============================================================================*/

/*==============================================================================
  Exported inline functions
==============================================================================*/
//==============================================================================
/**
 * @brief Function returns next mount entries.
 *
 * Function returns next file system entry according to file system order
 * <i>seek</i> (determined on mount time) and write information to mount entry
 * container pointed by <i>mntent</i>.
 *
 * @param seek          n-item to read
 * @param mntent        pointer to container
 *
 * @exception | @ref EINVAL
 * @exception | @ref ENOENT
 *
 * @return Returns \b 0 on success. Returns \b 1 if all items was read.
 * On error \b -1 is returned and <b>errno</b> is set appropriately.
 *
 * @b Example
 * @code
        #include <mntent.h>

        // ...

        int i = 0;

        struct mntent entry;
        while (getmntentry(i++, &entry) == 0) {
               // ...
        }

        // ...
   @endcode
 *
 * @see mount(), umount()
 */
//==============================================================================
static inline int getmntentry(int seek, struct mntent *mntent)
{
        int r = -1;
        syscall(SYSCALL_GETMNTENTRY, &r, &seek, mntent);
        return r;
}

#ifdef __cplusplus
}
#endif

#endif /* _MNTENT_H_ */

/**@}*/
/*==============================================================================
  End of file
==============================================================================*/

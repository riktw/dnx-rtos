/*=========================================================================*//**
@file    tty.c

@author  Daniel Zorychta

@brief   This file support virtual terminal.

@note    Copyright (C) 2012, 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

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

/*==============================================================================
  Include files
==============================================================================*/
#include "drivers/driver.h"
#include "tty_cfg.h"
#include "tty_def.h"
#include "tty.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
enum cmd {
        CMD_INPUT,
        CMD_SWITCH_TTY,
        CMD_CLEAR_TTY,
        CMD_LINE_ADDED,
        CMD_REFRESH_LAST_LINE
};

typedef struct {
        enum cmd        cmd : 8;
        u8_t            arg;
} tty_cmd_t;

typedef struct {
       queue_t         *queue_out;
       mutex_t         *secure_mtx;
       ttybfr_t        *screen;
       ttyedit_t       *editline;
       ttycmd_t        *vtcmd;
       u8_t             major;
} tty_t;

struct module {
        FILE           *infile;
        FILE           *outfile;
        task_t         *service_in;
        task_t         *service_out;
        queue_t        *queue_cmd;
        tty_t          *tty[_TTY_NUMBER];
        int             current_tty;
};

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void     service_out             (void *arg);
static void     service_in              (void *arg);
static void     send_cmd                (enum cmd cmd, u8_t arg);
static void     vt100_init              ();
static void     vt100_analyze           (const char c);
static void     copy_string_to_queue    (const char *str, queue_t *queue, bool lfend, uint timeout);
static void     switch_terminal         (int term_no);

/*==============================================================================
  Local object definitions
==============================================================================*/
MODULE_NAME(TTY);

static struct module *tty_module;
static const char    *service_in_name           = "tty-in";
static const char    *service_out_name          = "tty-out";
static const uint     service_in_stack_depth    = STACK_DEPTH_VERY_LOW - 60;
static const uint     service_out_stack_depth   = STACK_DEPTH_VERY_LOW - 45;
static const int      service_in_priority       = PRIORITY_NORMAL;
static const int      service_out_priority      = PRIORITY_NORMAL;
static const uint     queue_cmd_len             = _TTY_TERMINAL_ROWS;

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Initialize device
 *
 * @param[out]          **device_handle        device allocated memory
 * @param[in ]            major                major device number
 * @param[in ]            minor                minor device number
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_INIT(TTY, void **device_handle, u8_t major, u8_t minor)
{
        UNUSED_ARG(minor);

        int result = ENODEV;

        if (major >= _TTY_NUMBER || minor != 0) {
                return result;
        }

        /* initialize module base */
        if (!tty_module) {
                result = _sys_calloc(1 ,sizeof(struct module), reinterpret_cast(void**, &tty_module));
                if (result != ESUCC)
                        return result;

                result = _sys_fopen(_TTY_IN_FILE, "r", &tty_module->infile);
                if (result != ESUCC)
                        goto module_alloc_finish;

                result = _sys_fopen(_TTY_OUT_FILE, "w", &tty_module->outfile);
                if (result != ESUCC)
                        goto module_alloc_finish;

                result = _sys_task_create(service_in,
                                          service_in_name,
                                          service_in_stack_depth,
                                          NULL,
                                          NULL,
                                          &tty_module->service_in);
                if (result != ESUCC)
                        goto module_alloc_finish;

                result = _sys_task_create(service_out,
                                          service_out_name,
                                          service_out_stack_depth,
                                          NULL,
                                          NULL,
                                          &tty_module->service_out);
                if (result != ESUCC)
                        goto module_alloc_finish;

                result = _sys_queue_create(queue_cmd_len, sizeof(tty_cmd_t), &tty_module->queue_cmd);

                module_alloc_finish:
                if (result != ESUCC) {
                        if (tty_module->infile)
                                _sys_fclose(tty_module->infile);

                        if (tty_module->outfile)
                                _sys_fclose(tty_module->outfile);

                        if (tty_module->service_in)
                                _sys_task_destroy(tty_module->service_in);

                        if (tty_module->service_out)
                                _sys_task_destroy(tty_module->service_out);

                        if (tty_module->queue_cmd)
                                _sys_queue_destroy(tty_module->queue_cmd);

                        if (tty_module)
                                _sys_free(reinterpret_cast(void**, &tty_module));
                }
        } else {
                result = ESUCC;
        }

        /* initialize selected TTY */
        if (result == ESUCC) {
                result = _sys_calloc(1, sizeof(tty_t), device_handle);
                if (result != ESUCC)
                        return result;

                tty_t *tty = *device_handle;

                result = _sys_queue_create(_TTY_STREAM_SIZE, sizeof(char), &tty->queue_out);
                if (result != ESUCC)
                        goto tty_alloc_finish;

                result = _sys_mutex_create(MUTEX_TYPE_NORMAL, &tty->secure_mtx);
                if (result != ESUCC)
                        goto tty_alloc_finish;

                result = ttybfr_create(&tty->screen);
                if (result != ESUCC)
                        goto tty_alloc_finish;

                result = ttyedit_create(tty_module->outfile, &tty->editline);
                if (result != ESUCC)
                        goto tty_alloc_finish;

                result = ttycmd_create(&tty->vtcmd);
                if (result != ESUCC)
                        goto tty_alloc_finish;

                tty->major             = major;
                tty_module->tty[major] = tty;

                tty_alloc_finish:
                if (result != ESUCC) {
                        if (tty->vtcmd)
                                ttycmd_destroy(tty->vtcmd);

                        if (tty->editline)
                                ttyedit_destroy(tty->editline);

                        if (tty->screen)
                                ttybfr_destroy(tty->screen);

                        if (tty->secure_mtx)
                                _sys_mutex_destroy(tty->secure_mtx);

                        if (tty->queue_out)
                                _sys_queue_destroy(tty->queue_out);

                        if (device_handle)
                                _sys_free(device_handle);
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief Release device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_RELEASE(TTY, void *device_handle)
{
        tty_t *tty = device_handle;

        if (_sys_mutex_trylock(tty->secure_mtx) == ESUCC) {
                _sys_critical_section_begin();
                _sys_mutex_destroy(tty->secure_mtx);
                _sys_queue_destroy(tty->queue_out);
                ttybfr_destroy(tty->screen);
                ttyedit_destroy(tty->editline);
                ttycmd_destroy(tty->vtcmd);
                tty_module->tty[tty->major] = NULL;
                _sys_free(device_handle);

                /* de-initialize entire module if all TTYs are released */
                bool release_TTY = true;
                for (int i = 0; i < _TTY_NUMBER && release_TTY; i++) {
                        release_TTY = !tty_module->tty[i];
                }

                if (release_TTY) {
                        _sys_task_destroy(tty_module->service_in);
                        _sys_task_destroy(tty_module->service_out);
                        _sys_fclose(tty_module->infile);
                        _sys_fclose(tty_module->outfile);
                        _sys_queue_destroy(tty_module->queue_cmd);
                        _sys_free(reinterpret_cast(void**, &tty_module));
                }

                _sys_critical_section_end();
                return ESUCC;
        } else {
                return EBUSY;
        }
}

//==============================================================================
/**
 * @brief Open device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           flags                  file operation flags (O_RDONLY, O_WRONLY, O_RDWR)
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_OPEN(TTY, void *device_handle, u32_t flags)
{
        UNUSED_ARG(device_handle);
        UNUSED_ARG(flags);

        return ESUCC;
}

//==============================================================================
/**
 * @brief Close device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           force                  device force close (true)
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_CLOSE(TTY, void *device_handle, bool force)
{
        UNUSED_ARG(device_handle);
        UNUSED_ARG(force);

        return ESUCC;
}

//==============================================================================
/**
 * @brief Write data to device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]          *src                    data source
 * @param[in ]           count                  number of bytes to write
 * @param[in ][out]     *fpos                   file position
 * @param[out]          *wrcnt                  number of written bytes
 * @param[in ]           fattr                  file attributes
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_WRITE(TTY,
              void             *device_handle,
              const u8_t       *src,
              size_t            count,
              fpos_t           *fpos,
              size_t           *wrcnt,
              struct vfs_fattr  fattr)
{
        UNUSED_ARG(fpos);
        UNUSED_ARG(fattr);

        tty_t *tty = device_handle;

        if (_sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                ttybfr_put(tty->screen, reinterpret_cast(const char *, src), count);
                _sys_mutex_unlock(tty->secure_mtx);
                send_cmd(CMD_LINE_ADDED, tty->major);

                *wrcnt = count;

                return ESUCC;
        } else {
                return ETIME;
        }
}

//==============================================================================
/**
 * @brief Read data from device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *dst                    data destination
 * @param[in ]           count                  number of bytes to read
 * @param[in ][out]     *fpos                   file position
 * @param[out]          *rdcnt                  number of read bytes
 * @param[in ]           fattr                  file attributes
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_READ(TTY,
             void            *device_handle,
             u8_t            *dst,
             size_t           count,
             fpos_t          *fpos,
             size_t          *rdcnt,
             struct vfs_fattr fattr)
{
        UNUSED_ARG(fattr);

        tty_t *tty = device_handle;

        *rdcnt = 0;

        while (count--) {
                if (fattr.non_blocking_rd) {
                        if (_sys_mutex_lock(tty->secure_mtx, 100)) {
                                const char *str = ttyedit_get_value(tty->editline);
                                copy_string_to_queue(str, tty->queue_out, false, MAX_DELAY_MS);
                                ttyedit_clear(tty->editline);
                                _sys_mutex_unlock(tty->secure_mtx);
                        } else {
                                return ETIME;
                        }
                }

                if (_sys_queue_receive(tty->queue_out, dst, fattr.non_blocking_rd ? 0 : MAX_DELAY_MS)) {
                        (*rdcnt)++;

                        if (*dst == '\n')
                                break;

                        dst++;
                }
        }

        *fpos = 0;

        return ESUCC;
}

//==============================================================================
/**
 * @brief IO control
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           request                request
 * @param[in ][out]     *arg                    request's argument
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_IOCTL(TTY, void *device_handle, int request, void *arg)
{
        tty_t *tty    = device_handle;
        int    status = EINVAL;

        switch (request) {
        case IOCTL_TTY__GET_CURRENT_TTY:
                if (arg) {
                        *reinterpret_cast(int*, arg) = tty_module->current_tty;
                        status = ESUCC;
                }
                break;

        case IOCTL_TTY__GET_COL:
                if (arg) {
                        *reinterpret_cast(int*, arg) = _TTY_TERMINAL_COLUMNS;
                        status = ESUCC;
                }
                break;

        case IOCTL_TTY__GET_ROW:
                if (arg) {
                        *reinterpret_cast(int*, arg) = _TTY_TERMINAL_ROWS;
                        status = ESUCC;
                }
                break;

        case IOCTL_TTY__SET_EDITLINE:
                if (arg) {
                        if (_sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                                ttyedit_set_value(tty->editline, arg, tty_module->current_tty == tty->major);
                                _sys_mutex_unlock(tty->secure_mtx);
                                status = ESUCC;
                        } else {
                                status = ETIME;
                        }
                }
                break;

        case IOCTL_TTY__SWITCH_TTY_TO:
                send_cmd(CMD_SWITCH_TTY, reinterpret_cast(int, arg));
                status = ESUCC;
                break;

        case IOCTL_TTY__CLEAR_SCR:
                send_cmd(CMD_CLEAR_TTY, tty->major);
                status = ESUCC;
                break;

        case IOCTL_TTY__ECHO_ON:
                ttyedit_enable_echo(tty->editline);
                status = ESUCC;
                break;

        case IOCTL_TTY__ECHO_OFF:
                ttyedit_disable_echo(tty->editline);
                status = ESUCC;
                break;

        case IOCTL_TTY__GET_NUMBER_OF_TTYS:
                if (arg) {
                        *reinterpret_cast(int*, arg) = _TTY_NUMBER;
                }
                break;

        case IOCTL_TTY__REFRESH_LAST_LINE:
                send_cmd(CMD_REFRESH_LAST_LINE, tty->major);
                status = ESUCC;
                break;

        default:
                status = EBADRQC;
                break;
        }

        return status;
}

//==============================================================================
/**
 * @brief Flush device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_FLUSH(TTY, void *device_handle)
{
        tty_t *tty = device_handle;

        if (_sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                ttybfr_flush(tty->screen);
                send_cmd(CMD_REFRESH_LAST_LINE, tty->major);
                _sys_mutex_unlock(tty->secure_mtx);
        }

        return ESUCC;
}

//==============================================================================
/**
 * @brief Device information
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *device_stat            device status
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_STAT(TTY, void *device_handle, struct vfs_dev_stat *device_stat)
{
        tty_t *tty = device_handle;

        device_stat->st_size  = 0;
        device_stat->st_major = tty->major;
        device_stat->st_minor = _TTY_MINOR_NUMBER;

        return ESUCC;
}

//==============================================================================
/**
 * @brief TTY input service (helper task)
 */
//==============================================================================
static void service_in(void *arg)
{
        UNUSED_ARG(arg);

        _sys_task_set_priority(service_in_priority);

        for (;;) {
                char c = '\0'; size_t rdcnt;
                if (_sys_fread(&c, 1, &rdcnt, tty_module->infile) > 0) {
                        _sys_task_set_priority(PRIORITY_HIGHEST);
                        send_cmd(CMD_INPUT, c);
                        _sys_task_set_priority(service_in_priority);
                }
        }
}

//==============================================================================
/**
 * @brief TTY output service (main task)
 */
//==============================================================================
static void service_out(void *arg)
{
        UNUSED_ARG(arg);

        _sys_task_set_priority(service_out_priority);

        vt100_init();

        for (;;) {
                tty_cmd_t rq;

                if (_sys_queue_receive(tty_module->queue_cmd, &rq, MAX_DELAY_MS)) {
                        switch (rq.cmd) {
                        case CMD_INPUT: {
                                vt100_analyze(rq.arg);
                                break;
                        }

                        case CMD_CLEAR_TTY: {
                                if (rq.arg < _TTY_NUMBER && tty_module->tty[rq.arg]) {
                                        tty_t *tty = tty_module->tty[rq.arg];

                                        if (_sys_mutex_lock(tty->secure_mtx, 100)) {
                                                ttybfr_clear(tty->screen);

                                                if (tty_module->current_tty == tty->major) {
                                                        vt100_init();
                                                }

                                                _sys_mutex_unlock(tty->secure_mtx);
                                        }
                                }
                                break;
                        }

                        case CMD_SWITCH_TTY: {
                                switch_terminal(rq.arg);
                                break;
                        }

                        case CMD_LINE_ADDED: {
                                if (rq.arg < _TTY_NUMBER && tty_module->tty[rq.arg] && rq.arg == tty_module->current_tty) {
                                        tty_t *tty = tty_module->tty[rq.arg];

                                        if (_sys_mutex_lock(tty->secure_mtx, 100)) {
                                                const char *str;
                                                while ((str = ttybfr_get_fresh_line(tty->screen))) {
                                                        size_t wrcnt;

                                                        _sys_fwrite(VT100_CLEAR_LINE,
                                                                    strlen(VT100_CLEAR_LINE),
                                                                    &wrcnt,
                                                                    tty_module->outfile);

                                                        _sys_fwrite(str,
                                                                    strlen(str),
                                                                    &wrcnt,
                                                                    tty_module->outfile);
                                                }

                                                _sys_mutex_unlock(tty->secure_mtx);
                                        }
                                }
                                break;
                        }

                        case CMD_REFRESH_LAST_LINE: {
                                tty_t *tty = tty_module->tty[rq.arg];

                                if (rq.arg == tty_module->current_tty && _sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                                        size_t wrcnt;

                                        const char *cmd = VT100_SHIFT_CURSOR_LEFT(999) VT100_ERASE_LINE;
                                        _sys_fwrite(cmd,
                                                    strlen(cmd),
                                                    &wrcnt,
                                                    tty_module->outfile);

                                        const char *last_line = ttybfr_get_line(tty->screen, 0);
                                        _sys_fwrite(last_line,
                                                    strlen(last_line),
                                                    &wrcnt,
                                                    tty_module->outfile);

                                        const char *editline = ttyedit_get_value(tty->editline);
                                        _sys_fwrite(editline,
                                                    strlen(editline),
                                                    &wrcnt,
                                                    tty_module->outfile);

                                        _sys_mutex_unlock(tty->secure_mtx);
                                }
                                break;
                        }

                        default:
                                break;
                        }
                }
        }
}

//==============================================================================
/**
 * @brief Send command to main service
 *
 * @param cmd           a command
 * @param arg           an argument
 */
//==============================================================================
static void send_cmd(enum cmd cmd, u8_t arg)
{
        tty_cmd_t rq;
        rq.cmd = cmd;
        rq.arg = arg;

        _sys_queue_send(tty_module->queue_cmd, &rq, MAX_DELAY_MS);
}

//==============================================================================
/**
 * @brief Configure VT100 terminal
 */
//==============================================================================
static void vt100_init()
{
        const char *cmd = VT100_RESET_ATTRIBUTES
                          VT100_CLEAR_SCREEN
                          VT100_DISABLE_LINE_WRAP
                          VT100_CURSOR_HOME;

        size_t wrcnt;
        _sys_fwrite(cmd, strlen(cmd), &wrcnt, tty_module->outfile);
}

//==============================================================================
/**
 * @brief Control analyzis of VT100 input stream
 *
 * @param c             input character
 */
//==============================================================================
static void vt100_analyze(const char c)
{
        tty_t *tty = tty_module->tty[tty_module->current_tty];
        if (!tty)
                return;

        ttycmd_resp_t resp = ttycmd_analyze(tty->vtcmd, c);
        switch (resp) {
        case TTYCMD_KEY_ENTER:
                if (_sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                        const char *str  = ttyedit_get_value(tty->editline);
                        const char *lf   = "\n";

                        ttybfr_put(tty->screen, str, strlen(str));
                        ttybfr_put(tty->screen, lf, strlen(lf));
                        ttybfr_clear_fresh_line_counter(tty->screen);

                        if (ttyedit_is_echo_enabled(tty->editline)) {
                                const char *crlf = "\r\n";
                                size_t      wrcnt;
                                _sys_fwrite(crlf, strlen(crlf), &wrcnt, tty_module->outfile);
                        }

                        copy_string_to_queue(str, tty->queue_out, true, 0);
                        ttyedit_clear(tty->editline);

                        _sys_mutex_unlock(tty->secure_mtx);
                }
                break;

        case TTYCMD_KEY_CHAR:
                ttyedit_insert_char(tty->editline, c);
                break;

        case TTYCMD_KEY_BACKSPACE:
                ttyedit_remove_char(tty->editline);
                break;

        case TTYCMD_KEY_DELETE:
                ttyedit_delete_char(tty->editline);
                break;

        case TTYCMD_KEY_ARROW_LEFT:
                ttyedit_move_cursor_left(tty->editline);
                break;

        case TTYCMD_KEY_ARROW_RIGHT:
                ttyedit_move_cursor_right(tty->editline);
                break;

        case TTYCMD_KEY_ARROW_UP:
                copy_string_to_queue(VT100_ARROW_UP_STDOUT, tty->queue_out, true, 0);
                break;

        case TTYCMD_KEY_ARROW_DOWN:
                copy_string_to_queue(VT100_ARROW_DOWN_STDOUT, tty->queue_out, true, 0);
                break;

        case TTYCMD_KEY_TAB:
                copy_string_to_queue(ttyedit_get_value(tty->editline), tty->queue_out, false, 0);
                copy_string_to_queue(VT100_TAB, tty->queue_out, true, 0);
                break;

        case TTYCMD_KEY_HOME:
                ttyedit_move_cursor_home(tty->editline);
                break;

        case TTYCMD_KEY_END:
                ttyedit_move_cursor_end(tty->editline);
                break;

        case TTYCMD_KEY_F1...TTYCMD_KEY_F12:
                switch_terminal(resp - TTYCMD_KEY_F1);
                break;

        default:
                break;
        }
}

//==============================================================================
/**
 * @brief Copy string to queue
 *
 * @param str           string
 * @param queue         queue
 * @param lfend         true: adds LF, false: without LF
 * @param timeout       operation timeout [ms]
 */
//==============================================================================
static void copy_string_to_queue(const char *str, queue_t *queue, bool lfend, uint timeout)
{
        for (uint i = 0; i < strlen(str); i++) {
                _sys_queue_send(queue, &str[i], timeout);
        }

        if (lfend) {
                const char lf = '\n';
                _sys_queue_send(queue, &lf, timeout);
        }
}

//==============================================================================
/**
 * @brief Switch terminal
 *
 * @param term_no       terminal number
 */
//==============================================================================
static void switch_terminal(int term_no)
{
        if (term_no < _TTY_NUMBER && tty_module->tty[term_no] && tty_module->current_tty != term_no) {
                tty_t *tty = tty_module->tty[tty_module->current_tty];

                if (ttycmd_is_idle(tty->vtcmd)) {
                        tty_module->current_tty = term_no;

                        tty_t *tty = tty_module->tty[tty_module->current_tty];

                        vt100_init();

                        if (_sys_mutex_lock(tty->secure_mtx, MAX_DELAY_MS)) {
                                size_t      wrcnt;
                                const char *str;

                                for (int i = _TTY_TERMINAL_ROWS - 1; i >= 0; i--) {
                                        str = ttybfr_get_line(tty->screen, i);

                                        if (str) {
                                                _sys_fwrite(str,
                                                            strlen(str),
                                                            &wrcnt,
                                                            tty_module->outfile);
                                        }
                                }

                                str = ttyedit_get_value(tty->editline);
                                _sys_fwrite(str, strlen(str), &wrcnt, tty_module->outfile);
                                ttybfr_clear_fresh_line_counter(tty->screen);

                                _sys_mutex_unlock(tty->secure_mtx);
                        }
                } else {
                        /* try later */
                        send_cmd(CMD_SWITCH_TTY, term_no);
                }
        }
}

/*==============================================================================
  End of file
==============================================================================*/

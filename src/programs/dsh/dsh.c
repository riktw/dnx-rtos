/*=========================================================================*//**
@file    dsh.c

@author  Daniel Zorychta

@brief   dnx RTOS Shell interpreter

@note    Copyright (C) 2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dnx/os.h>
#include <dnx/thread.h>
#include <dnx/misc.h>
#include <dnx/vt100.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define PROMPT_LINE_LEN                 100
#define CWD_PATH_LEN                    128
#define HISTORY_NEXT_KEY                "\033^[A"
#define HISTORY_PREV_KEY                "\033^[B"
#define COMMAND_HINT_KEY                "\033^[T"

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void  clear_prompt         ();
static void  print_prompt         ();
static bool  read_input           ();
static bool  history_request      ();
static bool  command_hint         ();
static char *find_cmd_begin       ();
static bool  is_cd_cmd            (const char *cmd);
static bool  is_exit_cmd          (const char *cmd);
static char *find_arg             (char *cmd);
static void  change_directory     (char *str);
static char *remove_leading_spaces(char *str);
static void  print_fail_message   (char *cmd);
static bool  start_program        (char *master, char *slave, char *file);
static bool  analyze_line         (char *cmd);

/*==============================================================================
  Local object definitions
==============================================================================*/
GLOBAL_VARIABLES_SECTION {
        char            line[PROMPT_LINE_LEN];
        char            history[PROMPT_LINE_LEN];
        char            cwd[CWD_PATH_LEN];
        bool            prompt_enable;
        FILE           *input;
        bool            stream_closed;
        process_attr_t  pidmaster_attr;
        process_attr_t  pidslave_attr;
};

const char *pipe_file = "/tmp/dsh-";

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Clear prompt line
 */
//==============================================================================
static void clear_prompt()
{
        memset(global->line, '\0', PROMPT_LINE_LEN);
}

//==============================================================================
/**
 * @brief Function print line prompt
 */
//==============================================================================
static void print_prompt(void)
{
        if (global->prompt_enable && global->input == stdin) {
                printf(VT100_FONT_COLOR_GREEN"%s@%s:%s"VT100_RESET_ATTRIBUTES"\n",
                       get_user_name(), get_host_name(), global->cwd);

                printf(VT100_FONT_COLOR_GREEN"$ "VT100_RESET_ATTRIBUTES);

                fflush(stdout);
        }
}

//==============================================================================
/**
 * @brief Read command from selected file
 *
 * @return true if string read, false if not
 */
//==============================================================================
static bool read_input()
{
        if (global->stream_closed) {
                strcpy(global->line, "exit");
                return true;
        }

        if (fgets(global->line, PROMPT_LINE_LEN, global->input)) {

                if (feof(global->input)) {
                        global->stream_closed = true;
                }

                /* remove LF at the end of line */
                LAST_CHARACTER(global->line) = '\0';
                return true;
        } else {
                return false;
        }
}

//==============================================================================
/**
 * @brief Function handle history. Check if command line contain AUP, ADN keys
 *        and set prompt to historical value
 *
 * @return true if history got, false is new command inserted
 */
//==============================================================================
static bool history_request()
{
        if (strcmp(global->line, HISTORY_NEXT_KEY) == 0 || strcmp(global->line, HISTORY_PREV_KEY) == 0) {
                if (strlen(global->history)) {
                        ioctl(global->input, IOCTL_TTY__SET_EDITLINE, global->history);
                }

                global->prompt_enable = false;

                return true;
        } else {
                global->prompt_enable = true;

                if (strlen(global->line)) {
                        if (global->line[0] != '\033') {
                                strcpy(global->history, global->line);
                        }
                }

                return false;
        }
}

//==============================================================================
/**
 * @brief Finds hint for typed command
 * @param None
 * @return true if hint key was recognized, otherwise false
 */
//==============================================================================
static bool command_hint()
{
        char *tabstart = strchr(global->line, '\033');
        if (tabstart) {
                if (strcmp(tabstart, COMMAND_HINT_KEY) == 0) {
                        *tabstart = '\0';

                        if (strlen(global->line) != 0) {
                                dirent_t *dirent;
                                int cnt  = 0;

                                DIR *dir = opendir("/proc/bin");
                                if (dir) {
                                        while ((dirent = readdir(dir))) {
                                                if (strncmp(dirent->name, global->line, strlen(global->line)) == 0) {
                                                        cnt++;
                                                }
                                        }

                                        closedir(dir);
                                }

                                dir = opendir("/proc/bin");
                                if (dir) {
                                        if (cnt > 1) {
                                                puts("");
                                        }

                                        while ((dirent = readdir(dir))) {

                                                if (strncmp(dirent->name, global->line, strlen(global->line)) == 0) {
                                                        if (cnt > 1) {
                                                                printf("%s ", dirent->name);
                                                        } else  {
                                                                ioctl(global->input, IOCTL_TTY__SET_EDITLINE, dirent->name);
                                                                break;
                                                        }
                                                }
                                        }

                                        closedir(dir);

                                        if (cnt > 1) {
                                                puts(" ");
                                                print_prompt();
                                                ioctl(global->input, IOCTL_TTY__REFRESH_LAST_LINE);
                                        }
                                }
                        }

                        global->prompt_enable = false;
                        return true;
                }
        }

        return false;
}

//==============================================================================
/**
 * @brief Finds command start (ignore first spaces if exist)
 *
 * @return pointer to commnad in command line
 */
//==============================================================================
static char *find_cmd_begin()
{
        char *cmd  = global->line;
        cmd       += strspn(global->line, " ");

        return cmd;
}

//==============================================================================
/**
 * @brief Check if command is CD
 *
 * @param cmd
 *
 * @return true if CD command, otherwise false
 */
//==============================================================================
static bool is_cd_cmd(const char *cmd)
{
        return strncmp(cmd, "cd ", 3) == 0 || strcmp(cmd, "cd") == 0;
}

//==============================================================================
/**
 * @brief Check if command is EXIT
 *
 * @param cmd
 *
 * @return true if EXIT command, otherwise false
 */
//==============================================================================
static bool is_exit_cmd(const char *cmd)
{
        return strncmp(cmd, "exit ", 5) == 0 || strcmp(cmd, "exit") == 0;
}

//==============================================================================
/**
 * @brief Finds arguments after command
 *
 * @param cmd           command line begin
 *
 * @return pointer to first argument
 */
//==============================================================================
static char *find_arg(char *cmd)
{
        char *arg;
        if ((arg = strchr(cmd, ' ')) != NULL) {
                *(arg++) = '\0';
                arg += strspn(arg, " ");
        } else {
                arg = strchr(cmd, '\0');
        }

        return arg;
}

//==============================================================================
/**
 * @brief Function change current working path
 *
 * @param str          command name with arguments
 */
//==============================================================================
static void change_directory(char *str)
{
        char  *newpath   = NULL;
        bool   free_path = false;

        char *arg = find_arg(str);

        if (strcmp(arg, "..") == 0) {

                char *lastslash = strrchr(global->cwd, '/');
                if (lastslash) {
                        if (lastslash != global->cwd) {
                                *lastslash = '\0';
                         } else {
                                 *(lastslash + 1) = '\0';
                         }
                }

        } else if (strcmp(arg, ".") == 0) {
                /* do nothing */

        } else if (FIRST_CHARACTER(arg) != '/') {

                newpath = calloc(strlen(arg) + strlen(global->cwd) + 2, ARRAY_ITEM_SIZE(global->cwd));
                if (newpath) {
                        strcpy(newpath, global->cwd);

                        if (newpath[strlen(newpath) - 1] != '/') {
                                newpath[strlen(newpath)] = '/';
                        }

                        strcat(newpath, arg);

                        free_path = true;
                } else {
                        perror(NULL);
                }

        } else if (FIRST_CHARACTER(arg) == '/') {

                newpath = arg;

        } else {

                puts(strerror(ENOENT));
        }

        if (newpath) {
                DIR *dir = opendir(newpath);
                if (dir) {
                        closedir(dir);
                        strncpy(global->cwd, newpath, CWD_PATH_LEN);
                } else {
                        perror(arg);
                }

                if (free_path) {
                        free(newpath);
                }
        }
}

//==============================================================================
/**
 * @brief Remove leading spaces
 *
 * @param str           string with leading spaces
 *
 * @return string without leading spaces (move pointer to space-free index)
 */
//==============================================================================
static char *remove_leading_spaces(char *str)
{
        if (FIRST_CHARACTER(str) == ' ')
                while (*(++str) == ' ');

        return str;
}

//==============================================================================
/**
 * @brief Function print command find failure
 *
 * @param cmd           cmd string
 */
//==============================================================================
static void print_fail_message(char *cmd)
{
        cmd = remove_leading_spaces(cmd);

        if (strchr(cmd, ' '))
                *strchr(cmd, ' ') = '\0';

        if (errno == ENOMEM) {
                perror(cmd);
        } else {
                printf("\'%s\' is unknown command.\n", cmd);
        }
}

//==============================================================================
/**
 * @brief Function start defined program group
 *
 * @param cmd           command line
 *
 * @return true if command executed, false if error
 */
//==============================================================================
static bool start_program(char *master, char *slave, char *file)
{
        errno              = 0;
        FILE    *pipe      = NULL;
        FILE    *fout      = NULL;
        char    *pipe_name = NULL;
        bool     status    = true;
        pid_t    pidmaster = 0;
        pid_t    pidslave  = 0;

        if (master) {
                master = remove_leading_spaces(master);
        }

        if (slave) {
                slave = remove_leading_spaces(slave);
        }

        if (file) {
                file = remove_leading_spaces(file);
        }

        if (file) {
                fout = fopen(file, "a");
                if (!fout) {
                        perror(file);
                        goto free_resources;
                }
        }

        if (master && slave) {
                pipe_name = calloc(sizeof(char), strlen(pipe_file) + 7);
                if (pipe_name) {
                        u32_t uptime = get_time_ms();
                        snprintf(pipe_name, strlen(pipe_file) + 7, "%s%x", pipe_file, uptime);

                        if (mkfifo(pipe_name, 0666)) {
                                perror("sh");
                                goto free_resources;
                        }

                        pipe = fopen(pipe_name, "r+");
                        if (!pipe) {
                                perror("sh");
                                goto free_resources;
                        }
                } else {
                        perror("sh");
                        goto free_resources;
                }
        }

        if (master && slave && file) {
                global->pidmaster_attr.f_stdin    = stdin;
                global->pidmaster_attr.f_stdout   = pipe;
                global->pidmaster_attr.f_stderr   = pipe;
                global->pidmaster_attr.cwd        = global->cwd;
                global->pidmaster_attr.has_parent = true;
                global->pidmaster_attr.priority   = PRIORITY_NORMAL;
                pidmaster = process_create(master, &global->pidmaster_attr);
                if (pidmaster == 0) {
                        print_fail_message(master);
                        goto free_resources;
                }

                global->pidslave_attr.f_stdin    = pipe;
                global->pidslave_attr.f_stdout   = fout;
                global->pidslave_attr.f_stderr   = fout;
                global->pidslave_attr.cwd        = global->cwd;
                global->pidslave_attr.has_parent = true;
                global->pidslave_attr.priority   = PRIORITY_NORMAL;
                pidslave = process_create(slave, &global->pidslave_attr);
                if (pidslave == 0) {
                        process_kill(pidmaster, NULL);
                        print_fail_message(slave);
                        goto free_resources;
                }

                process_wait(pidmaster, NULL, MAX_DELAY_MS);
                ioctl(pipe, IOCTL_PIPE__CLOSE);
                process_wait(pidslave, NULL, MAX_DELAY_MS);

        } else if (master && slave) {
                global->pidmaster_attr.f_stdin    = stdin;
                global->pidmaster_attr.f_stdout   = pipe;
                global->pidmaster_attr.f_stderr   = pipe;
                global->pidmaster_attr.cwd        = global->cwd;
                global->pidmaster_attr.has_parent = true;
                global->pidmaster_attr.priority   = PRIORITY_NORMAL;
                pidmaster = process_create(master, &global->pidmaster_attr);
                if (pidmaster == 0) {
                        print_fail_message(master);
                        goto free_resources;
                }

                global->pidslave_attr.f_stdin    = pipe;
                global->pidslave_attr.f_stdout   = stdout;
                global->pidslave_attr.f_stderr   = stderr;
                global->pidslave_attr.cwd        = global->cwd;
                global->pidslave_attr.has_parent = true;
                global->pidslave_attr.priority   = PRIORITY_NORMAL;
                pidslave = process_create(slave, &global->pidslave_attr);
                if (pidslave == 0) {
                        process_kill(pidmaster, NULL);
                        print_fail_message(slave);
                        goto free_resources;
                }

                process_wait(pidmaster, NULL, MAX_DELAY_MS);
                ioctl(pipe, IOCTL_PIPE__CLOSE);
                process_wait(pidslave, NULL, MAX_DELAY_MS);

        } else if (master && file) {
                global->pidmaster_attr.f_stdin    = stdin;
                global->pidmaster_attr.f_stdout   = fout;
                global->pidmaster_attr.f_stderr   = fout;
                global->pidmaster_attr.cwd        = global->cwd;
                global->pidmaster_attr.has_parent = true;
                global->pidmaster_attr.priority   = PRIORITY_NORMAL;
                pidmaster = process_create(master, &global->pidmaster_attr);
                if (pidmaster == 0) {
                        print_fail_message(master);
                        goto free_resources;
                }

                process_wait(pidmaster, NULL, MAX_DELAY_MS);

        } else if (master) {
                global->pidmaster_attr.f_stdin    = stdin;
                global->pidmaster_attr.f_stdout   = stdout;
                global->pidmaster_attr.f_stderr   = stderr;
                global->pidmaster_attr.cwd        = global->cwd;
                global->pidmaster_attr.has_parent = true;
                global->pidmaster_attr.priority   = PRIORITY_NORMAL;
                pidmaster = process_create(master, &global->pidmaster_attr);
                if (pidmaster == 0) {
                        print_fail_message(master);
                        goto free_resources;
                }

                process_wait(pidmaster, NULL, MAX_DELAY_MS);

        } else {
                status = false;
        }

free_resources:
        if (fout) {
                fclose(fout);
        }

        if (pipe) {
                fclose(pipe);
                remove(pipe_name);
        }

        if (pipe_name) {
                free(pipe_name);
        }

        return status;
}

//==============================================================================
/**
 * @brief Function analyze line
 *
 * @param cmd           command line
 *
 * @return true if command executed, false if error
 */
//==============================================================================
static bool analyze_line(char *cmd)
{
        int pipe_number = 0;
        int out_number  = 0;

        for (size_t i = 0; i < strlen(cmd); i++) {
                if (cmd[i] == '|')
                        pipe_number++;

                if (cmd[i] == '>')
                        out_number++;
        }

        if (pipe_number > 1 || out_number > 1) {
                puts("sh: syntax error");
                return true;
        }

        if (out_number && pipe_number) {
                if ((strchr(cmd, '|') > strchr(cmd, '>')) || strchr(cmd, '|') == cmd) {
                        puts("sh: syntax error");
                        return true;
                } else {
                        char *master = cmd;

                        char *slave = strchr(master, '|');
                        *slave++ = '\0';

                        char *file = strchr(slave, '>');
                        *file++ = '\0';

                        return start_program(master, slave, file);
                }
        }

        if (out_number) {
                if (strchr(cmd, '>') == cmd) {
                        puts("sh: syntax error");
                        return true;
                } else {
                        char *master = cmd;

                        char *file = strchr(master, '>');
                        *file++ = '\0';

                        return start_program(master, NULL, file);
                }
        }

        if (pipe_number) {
                if (strchr(cmd, '|') == cmd) {
                        puts("sh: syntax error");
                        return true;
                } else {
                        char *master = cmd;

                        char *slave = strchr(master, '|');
                        *slave++ = '\0';

                        return start_program(master, slave, NULL);
                }
        }

        if (strlen(cmd)) {
                return start_program(cmd, NULL, NULL);
        } else {
                return false;
        }
}

//==============================================================================
/**
 * @brief Terminal main function
 */
//==============================================================================
int_main(dsh, STACK_DEPTH_MEDIUM, int argc, char *argv[])
{
        global->prompt_enable = true;
        global->input         = stdin;

        if (argc == 2) {
                global->input = fopen(argv[1], "r");
                if (!global->input) {
                        perror(argv[1]);
                        return EXIT_FAILURE;
                }
        }

        getcwd(global->cwd, CWD_PATH_LEN);

        for (;;) {
                clear_prompt();

                print_prompt();

                if (!read_input())
                        break;

                if (history_request())
                        continue;

                if (command_hint())
                        continue;

                char *cmd = find_cmd_begin();

                if (strlen(cmd) == 0)
                        continue;

                if (is_cd_cmd(cmd)) {
                        change_directory(cmd);
                        continue;
                }

                if (is_exit_cmd(cmd)) {
                        break;
                }

                if (analyze_line(cmd)) {
                        continue;
                }

                print_fail_message(cmd);
        }

        if (global->input != stdin) {
                fclose(global->input);
        }

        return 0;
}

/*==============================================================================
  End of file
==============================================================================*/

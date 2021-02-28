/*==============================================================================
File    run.c

Author  

Brief   

        Copyright (C) 2021  <>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation and modified by the dnx RTOS exception.

        NOTE: The modification  to the GPL is  included to allow you to
              distribute a combined work that includes dnx RTOS without
              being obliged to provide the source  code for proprietary
              components outside of the dnx RTOS.

        The dnx RTOS  is  distributed  in the hope  that  it will be useful,
        but WITHOUT  ANY  WARRANTY;  without  even  the implied  warranty of
        MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
        GNU General Public License for more details.

        Full license text is available on the following file: doc/license.txt.

==============================================================================*/

/*==============================================================================
  Include files
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "loader.h"
#include "loader_config.h"
#include <malloc.h>

/*==============================================================================
  Local macros
==============================================================================*/


/*==============================================================================
  Local object types
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local objects
==============================================================================*/
GLOBAL_VARIABLES_SECTION {
	bool elf_inited;
	ELFSymbol_t exports[10];
	ELFEnv_t env;
	char* sp;
};

static const ELFSymbol_t appendedExports[] = {
		{ "printf", (void*)printf },
		{ "_sleep_ms", (void*)_sleep_ms }
};

/*==============================================================================
  Exported objects
==============================================================================*/
PROGRAM_PARAMS(run, STACK_DEPTH_LARGE);

/*==============================================================================
  External objects
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/
typedef void (*fnc_ptr)(void);

void init_runner()
{
	global->elf_inited = false;
}

void init_elf()
{
	if(!global->elf_inited)
	{
		int counter = 0;
		global->elf_inited = true;


		for(unsigned int i = 0; i < (sizeof(appendedExports) / sizeof(*appendedExports)); ++i)
		{
			strcpy(global->exports[counter].name, appendedExports[i].name);
			global->exports[counter].ptr = appendedExports[i].ptr;
			++counter;
		}

		//stdout is not const.
		strcpy(global->exports[counter].name, "stdout");
		global->exports[counter].ptr = (void*)stdout;
		++counter;

		global->env.exported = global->exports;
		global->env.exported_size = counter;

	}
}

void *do_alloc(size_t size, size_t align, ELFSecPerm_t perm) {
	(void) perm;
	(void) align;
	return malloc(size);
}

void *do_alloc_sdram(size_t size, size_t align, ELFSecPerm_t perm) {
	(void) perm;
	(void) align;
	return malloc(size);
}

static int exec_elf(const char *path, const ELFEnv_t *env) {
	ELFExec_t *exec;
	loader_env_t loader_env;
	loader_env.env = env;
	if(load_elf(path, loader_env, &exec) == 0)
	{
		jumpTo(exec);
		void (*doit)(void) = get_func(exec, "doit");
		if (doit) {
			(doit)();
		}
		unload_elf(exec);
	}
	else
	{
		return -1;
	}

	return 0;
}

void arch_jumpTo(entry_t entry) {
	entry();
}

int is_streq(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2)
			return 0;
		s1++;
		s2++;
	}
	return *s1 == *s2;
}


//==============================================================================
/**
 * Main program function.
 *
 * Note: Please adjust stack size according to programs needs.
 *
 * @param argc      argument count
 * @param argv      arguments
 */
//==============================================================================
int main(int argc, char *argv[])
{
	init_elf();
	if (argc >= 2)
	{
		if(exec_elf(argv[1], &global->env) == -1)
		{
			printf("This file does not exist or is invalid\n");
		}
	} else
	{
		printf("Runs elf programs. Usage: run <programname.elf>\n");
	}

	return EXIT_SUCCESS;
}

char* strtok1(char* str, const char* delimiters)
{

	int i = 0;
	int len = strlen(delimiters);

	/* check in the delimiters */
	if(len == 0)
		printf("delimiters are empty\n");

	/* if the original string has nothing left */
	if(!str && !global->sp)
		return NULL;

	/* initialize the sp during the first call */
	if(str && !global->sp)
		global->sp = str;

	/* find the start of the substring, skip delimiters */
	char* p_start = global->sp;
	while(true) {
		for(i = 0; i < len; i ++) {
			if(*p_start == delimiters[i]) {
				p_start ++;
				break;
			}
		}

		if(i == len) {
			global->sp = p_start;
			break;
		}
	}

	/* return NULL if nothing left */
	if(*global->sp == '\0') {
		global->sp = NULL;
		return global->sp;
	}

	/* find the end of the substring, and
        replace the delimiter with null */
	while(*global->sp != '\0') {
		for(i = 0; i < len; i ++) {
			if(*global->sp == delimiters[i]) {
				*global->sp = '\0';
				break;
			}
		}

		global->sp ++;
		if (i < len)
			break;
	}

	return p_start;
}

/*==============================================================================
  End of file
==============================================================================*/


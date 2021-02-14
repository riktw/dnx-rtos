/*
  driver file for MiniBasic.
  by Malcolm Mclean
  Leeds University
*/

#include <stdio.h>
#include <stdlib.h>

#include "basic.h"



char *loadfile(char *path);

/*==============================================================================
  Local objects
==============================================================================*/
GLOBAL_VARIABLES_SECTION {
	FORLOOP forstack[MAXFORS];   /* stack for for loop conrol */
	int nfors;					/* number of fors on stack */

	VARIABLE *variables;			/* the script's variables */
	int nvariables;				/* number of variables */

	DIMVAR *dimvariables;		/* dimensioned arrays */
	int ndimvariables;			/* number of dimensioned arrays */

	LINE *lines;					/* list of line starts */
	int nlines;					/* number of BASIC lines in program */

	FILE *fpin;				/* input stream */
	FILE *fpout;				/* output strem */
	FILE *fperr;				/* error stream */

	const char *string;        /* string we are parsing */
	int token;                 /* current token (lookahead) */
	int errorflag;             /* set when error in input encountered */
};


/*==============================================================================
  Exported objects
==============================================================================*/
PROGRAM_PARAMS(basic, STACK_DEPTH_MEDIUM);


void usage(void)
{
  printf("MiniBasic: a BASIC interpreter\n");
  printf("usage:\n");
  printf("Basic <script>\n");
  printf("See documentation for BASIC syntax.\n");
  exit(EXIT_FAILURE);
}

/*
  call with the name of the Minibasic script file
*/
int main(int argc, char *argv[])
{
  char *scr;

  if(argc == 1)
  {
	usage();
  }
  else
  {
	scr = loadfile(argv[1]);
	if(scr)
	{
	  basic(scr, stdin, stdout, stderr);
	  free(scr);
	}
  }

  return 0;
}

/*
  function to slurp in an ASCII file
  Params: path - path to file
  Returns: malloced string containing whole file
*/
char *loadfile(char *path)
{
  FILE *fp;
  int ch;
  long i = 0;
  long size = 0;
  char *answer;
  
  fp = fopen(path, "r");
  if(!fp)
  {
    printf("Can't open %s\n", path);
	return 0;
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  answer = malloc(size + 100);
  if(!answer)
  {
    printf("Out of memory\n");
    fclose(fp);
	return 0;
  }

  while( (ch = fgetc(fp)) != EOF)
    answer[i++] = ch;

  answer[i++] = 0;

  fclose(fp);

  return answer;
}

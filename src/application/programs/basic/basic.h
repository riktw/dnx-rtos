#ifndef basic_h
#define basic_h
/*
  Minibasic header file
  By Malcolm Mclean
*/


#define MAXFORS 32    /* maximum number of nested fors */

typedef struct
{
  int no;                 /* line number */
  const char *str;		  /* points to start of line */
}LINE;

typedef struct
{
  char id[32];			/* id of variable */
  double dval;			/* its value if a real */
  char *sval;			/* its value if a string (malloced) */
} VARIABLE;

typedef struct
{
  char id[32];			/* id of dimensioned variable */
  int type;				/* its type, STRID or FLTID */
  int ndims;			/* number of dimensions */
  int dim[5];			/* dimensions in x y order */
  char **str;			/* pointer to string data */
  double *dval;			/* pointer to real data */
} DIMVAR;

typedef struct
{
  int type;				/* type of variable (STRID or FLTID or ERROR) */
  char **sval;			/* pointer to string data */
  double *dval;			/* pointer to real data */
} LVALUE;

typedef struct
{
  char id[32];			/* id of control variable */
  int nextline;			/* line below FOR to which control passes */
  double toval;			/* terminal value */
  double step;			/* step size */
} FORLOOP;

int basic(const char *script, FILE *in, FILE *out, FILE *err);

#endif

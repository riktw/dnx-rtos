/*==============================================================================
File    editor.c

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
	int quit_times;
};

/*==============================================================================
  Exported objects
==============================================================================*/
PROGRAM_PARAMS(editor, STACK_DEPTH_MEDIUM);

/*==============================================================================
  External objects
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

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
/**** Includes *****/ 

#define _DEFAULT_SOURCE  //feature test macro
#define _BSD_SOURCE    //feature test macro
#define _GNU_SOURCE   //feature test macro

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dnx/vt100.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>


/****** defines ******/

#define CTRL_KEY(k) ((k) & 0x1f)  //Defining CTRL_KEY(k) as ctrl+ some character. Changing the meaning of the control charcters.

#define ABUF_INIT {NULL,0}  //works as constructor for the structure.

#define EDITOR_VERSION "0.0.1" //Editor version which is displayed in the welcome message.

#define EDITOR_TAB_STOP 8  //the maximum number of spaces a tab keypress can have.

#define EDITOR_QUIT_TIMES 3 //The number of times one has to press Ctrl-Q to quit the editor  if the displayed file is not saved.

enum editorKey {          //Used to map arrow keys to movement of cursor function and preventing it from clashing with other charch
	//ter inputs.
	BackSpace = 127,
	Arrow_left=1000,
	Arrow_right,
	Arrow_up ,
	Arrow_down,
	Page_up,
	Page_down,
	Home,
	End,
	Del,
	Enter =13     //ASCII value of enter keypress

};

/**** Data *****/

typedef struct erow{                //structure defined to store the characters to be printed on a line in the editor.
	int size;
	char* chars;

	char* render;   //Copies the content of chars and replaces each tab with appropiate number of spaces.

	int rsize;     //Stores the siz eof render string.

} erow;


struct editorConfig{

	//struct termios orig_termios; //struct termios belong to the <termios.h> header file. contains fields which
	//store flag bits controlling various attributes of the terminal.

	int screenrows;
	int screencols;

	int cursorX;
	int cursorY;

	int numrows;   //counts the number of rows of text the file has
	erow *row;      //structure which acts as a dynamic array to store a string which is a single line.

	int fileopen_flag;    //flag to indicate whether file is opened or not.

	int rowoff;     //Stores the row number of the file from which the text should be displayed.

	int coloff;    //Stores the column number of the file from which the text should be displayed.

	int renderX;   //Stores the horizontal position of the cursor in the render string

	char* filename;  //String to store the filename of the file which is being currently edited.

	char statusmsg[80];  //String to store the message which is to be displayed to the user.

	time_t statusmsg_time; // Time for which statusmsg is to be displayed.

	int dirty_flag;  //it is a flag variable which indicates if a file has been edited and not saved (i.e; the file has been modified
	//and the content displayed is different from the content that is saved in the file).

};

struct editorConfig E;

/****** Prototypes **********/

void editorSetStatusMsg(const char* fmt, ...);

void editorFreeRow(erow* row);

void editorDelRow(int at);

void editorRowAppendString(erow* row, char* s, size_t len);

void disableRawMode();
/**** Terminal ******/

void die(const char* s)      
{
	write(fileno(stdout), "\x1b[2J", 4);
	write(fileno(stdout), "\x1b[H", 3);


	perror(s);
	disableRawMode();
	exit(1);
}

void disableRawMode() {

	/*if(tcsetattr(fileno(stdin), TCSAFLUSH, &E.orig_termios)==-1) //belong to <termios.h>
		die("tcsetattr");
	 */
	ioctl(fileno(stdin), IOCTL_TTY__ECHO_ON);
	ioctl(fileno(stdin), IOCTL_TTY__RAW_MODE_OFF);
	ioctl(fileno(stdin), IOCTL_VFS__DEFAULT_RD_MODE);
}


void enableRawMode() {

	ioctl(fileno(stdin), IOCTL_TTY__ECHO_OFF);
	ioctl(fileno(stdin), IOCTL_TTY__RAW_MODE_ON);
	ioctl(fileno(stdout), IOCTL_TTY__CLEAR_SCR);
	ioctl(fileno(stdin), IOCTL_VFS__NON_BLOCKING_RD_MODE);
	/*
	if(tcgetattr(fileno(stdin), &E.orig_termios)==-1) //belong to <termios.h>
		die("tcgetattr");

	atexit(disableRawMode); //belong to <stdio.h> : tells the program to call disableRawMode() whenever				   // the program exits.

	struct termios raw;

	if(tcgetattr(fileno(stdin), &raw)==-1)
	{
		die("tcgetattr");
	}

	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);//To change the flag in terminal controlling the ctrl-s and ctr-q  //so as to print only the ascii values when such control character are typed. 

	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); //To change the flags controlling various attrinbutes of						   //terminal, thus changing the terminal to raw mode,turning off echo, and disabling signal  controls				       // such as ctrl-c a ctrl-z.

	raw.c_oflag &= ~(OPOST); //To turn off any output processing in terminal

	raw.c_cflag |= (CS8);

	raw.c_cc[VMIN]=	0;  //VMIN is a index of c_cc byte array,(cc- control character), c_cc[VMIN] decides how many minimum charc-
	//ters must read() must take before returning.

	raw.c_cc[VTIME]= 1; //VTIME is a index of c_cc byte array, c_cc[VTIME] decides time read() has to wait before returning

	//the above VMIN and VTIME doesn't work for WSL (bash in windows).

	if(tcsetattr(fileno(stdin), TCSAFLUSH, &raw)==-1)
	{
		die("tcsetattr");
	}
	 */

}


int editorReadKey()
{
	int nread;
	char c;
	while((nread=read(fileno(stdin), &c,1) != 1))
	{
		if(nread == -1 && errno != EAGAIN)
		{
			die("read");
		}

	}

	if(c == 27)        //Condition for reading arrow keys for moving the cursor; Arrow keys are multiple bytes escape
	{                    //sequence such as \x1b[? , where ? is replaced by A,B,C,D to associate with the arrow keys.
		char seq[4];

		if((nread=read(fileno(stdin), &seq[0], 1))!=1)
		{
			if(nread== -1 && errno != EAGAIN)
			{
				die("read");
			}
			else
			{
				return '\x1b';
			}
		}

		if((nread=read(fileno(stdin), &seq[1], 1)) !=1)
		{
			if(nread == -1 && errno != EAGAIN)
			{
				die("read");
			}
			else
			{
				return '\x1b';
			}
		}



		if(seq[0] == '^')
		{
			if((nread=read(fileno(stdin), &seq[2], 1)) !=1)
			{
				if(nread == -1 && errno != EAGAIN)
				{
					die("read");
				}
				else
				{
					return '\x1b';
				}
			}

			if(seq[1] == '[')
			{
				switch(seq[2])
				{
				case 'A': return Arrow_up;
				case 'B': return Arrow_down;
				case 'C': return Arrow_right;
				case 'D': return Arrow_left;
				case 'H': return Home;
				case 'F': return End;
				default: return '\x1b';
				}
			}
		}

		else if(seq[0] == '[')
		{
			if(seq[1]>= '0' && seq[1] <= '9')
			{
				if((nread=read(fileno(stdin), &seq[2], 1)) != 1)
				{
					if(nread == -1 && errno != EAGAIN)
					{
						die("read");
					}
					else
					{
						return '\x1b';
					}
				}

				if(seq[2]=='~')
				{
					switch(seq[1])
					{
					case '5': return Page_up;


					case '6': return Page_down;


					case '1':
					case '7': return Home;

					case '4':
					case '8': return End;

					case '3': return Del;
					}
				}
				else
				{
					return '\x1b';
				}


			}
			else
			{
				switch(seq[1])
				{
				case 'A': return Arrow_up;
				case 'B': return Arrow_down;
				case 'C': return Arrow_right;
				case 'D': return Arrow_left;
				case 'H': return Home;
				case 'F': return End;
				default: return '\x1b';
				}
			}
		}
		else
		{
			if(seq[0] == 'O')
			{
				switch(seq[1])
				{
				case 'H': return Home;
				case 'F' : return End;
				default: return '\x1b';
				}
			}
			else
			{
				return '\x1b';
			}
		}

	}
	else
	{
		return c;
	}
	return c;
}


int getCursorPosition(int* rows,int* cols)       //Isn't working!! The escape sequence command \x1b[6N is not replying with the 
{						// cursor position, checked in bash in windows(WSL).
	char buf[32];
	unsigned int i=0;


	if(write(fileno(stdout), "\x1b[6N",4)!=4)
	{
		return -1;
	}


	while(i < sizeof(buf) - 1)
	{
		if(read(fileno(stdin), &buf[i], 1) != 1)
		{
			break;
		}

		if(buf[i]== 'R')
		{
			break;
		}

		i++;
	}

	buf[i] = '\0';

	if(buf[0] != '\x1b' || buf[1] != '[')
	{
		return -1;
	}

	if(sscanf(&buf[2], "%d;%d", rows,cols) != 2)
	{
		return -1;
	}

	return 0;

}	


int getWindowSize(int* row, int* col)  //getting the size of the terminal in which current editor is being displayed.
{
	//struct winsize ws;   //defined by <sys/ioctl.h>; structure having two fields, one row and other one is column

	/*
	if( ioctl(fileno(stdout), TIOCGWINSZ, &ws)==-1 || ws.ws_col==0)  //ioctl() with TIOCGWINSZ returns the terminal size
	{
		if(write(fileno(stdout), "\x1b[999C\x1b[999B", 12) !=12)  //Moving the cursor to right bottom position, used in fall
		{							 //back method for finding the terminal size in which the 
			return -1;					 //editor is represented.
		}

		return getCursorPosition(row,col); //Not working
	}
	else
	{
	 *row = ws.ws_row;
	 *col= ws.ws_col;

		return 0;
	}
	 */
	*row = 23;
	*col = 80;
	return 0;
}

/******** Row Operations *********/


/*
int editorRowCxtoRx(erow* row, int cx)
{
	int rX =0;
	int j;

	for(j=0;j < cx;j++)
	{
		if(row->chars[j] == '\t')
		{
			rX += (EDITOR_TAB_STOP - 1) - (rX % EDITOR_TAB_STOP);
		}

		rX++;
	}

	return rX;
}

 */


void editorUpdateRow(erow *row)    //function to parse chars and replace tabs with spaces and store them in render string.
{
	int tab =0;
	int j;

	for(j=0;j<(row->size);j++)
	{
		if(row->chars[j] == '\t')
		{
			tab++;
		}
	}



	free(row->render);
	row->render = (char*) malloc(row->size+ tab * ((EDITOR_TAB_STOP)-1) + 1);

	int idx=0;

	for(j=0;j<row->size;j++)
	{
		if(row->chars[j] != '\t')
		{
			row->render[idx++] = row->chars[j];
		}
		else
		{
			//	for(int i=0;i<8;i++)

			row->render[idx++] = ' ';



			while(idx % EDITOR_TAB_STOP != 0)
			{
				row->render[idx++] = ' ';
			}

		}
	}

	row->render[idx] = '\0';

	row->rsize = idx;
}

/*

void editorAppendRow(char* s, size_t len)
{
	E.row=realloc(E.row,sizeof(erow)*(E.numrows+1));

	int at=E.numrows;
	E.row[at].size=len;

	E.row[at].chars=(char*)malloc(len+1);

	memcpy(E.row[at].chars,s,len+1);
	E.row[at].render=NULL;
	E.row[at].rsize=0;

	editorUpdateRow(&E.row[at]);

	E.numrows++;

	E.dirty_flag++;
}

 */



void editorInsertRow(int at,char *s, size_t len)  //Function to insert a string at the current line number and shift current
{						  //present strings downward.
	if(at < 0 && at > E.numrows)
	{
		return;
	}


	E.row = realloc(E.row,sizeof(erow) * (E.numrows+1));

	memmove(&E.row[at+1],&E.row[at],sizeof(erow) * (E.numrows - at));



	//	int at=E.numrows;


	E.row[at].size = len;
	E.row[at].chars = (char*) malloc(len+1);
	memcpy(E.row[at].chars,s,len+1);
	E.row[at].chars[len] = '\0';

	E.row[at].render = NULL;
	E.row[at].rsize = 0;

	editorUpdateRow(&E.row[at]);

	E.numrows++;
	E.dirty_flag++;
}




void editorRowInsertChar(erow* row,int at,int c)  //Function to insert a single character in a line.
{
	if(at < 0 || at > row->size)
	{
		at = row->size;
	}

	row->chars = (char*) realloc(row->chars ,row->size+2);

	memmove(&row->chars[at+1],&row->chars[at],(row->size+1)-at);  //memmove is same as memcopy(which is to copy one string to
	//another but it is preferred when both source and destinatio
	//-n memory overlap.
	row->size++;

	row->chars[at] = c;
	editorUpdateRow(row);

	E.dirty_flag++;

}

void editorRowDelChars(erow* row, int at)  //Method which deltes a single character from a string by overwriting it with part of
//after the character using memmove() method.
{
	if(at < 0  || at >= row->size)    //condition to check if something to be deleted lies within the start and end of the
		//string (both ends inclusive).
	{
		return ;
	}

	memmove(&row->chars[at], &row->chars[at+1], row->size-at); //Shift the characters to the left to overrite the character 
	//to be deleted.
	row->size--;

	editorUpdateRow(row);

	E.dirty_flag++;  //Updating dirty_flag so as to infer a change in the file which is unsaved.

}


void editorDelChars()   //Method to delete a character preceding the X coordinate of the current position of the cursor. 
{
	if(E.cursorY == E.numrows)
	{
		return;
	}

	erow *row=&E.row[E.cursorY];

	if(E.renderX > 0)
	{
		editorRowDelChars(row,E.renderX-1);
		E.renderX--;
		if(E.renderX == 0)  //comdition to prevent double deleteion as if E.renderX = 0, then next if case will be executed,					//which is a unwanted execution.Helps to tackle the issue of pressing Delete at the start of the 				    // line.
		{
			return;
		}
	}

	if(E.renderX == 0)    //Condition to check for backspacing at the start of the line.
	{
		if(E.cursorY !=0) //Condition to check if cursor is present at the start of the file.
		{
			E.renderX = E.row[E.cursorY-1].size; //move the cursor to the end of previous line on backspacing at the 
			//start of the current line.

			editorRowAppendString(&E.row[E.cursorY-1],E.row[E.cursorY].chars,E.row[E.cursorY].size); //Appending the
			//string of current line to string of previous line.

			//			editorFreeRow(&E.row[E.cursorY]);

			editorDelRow(E.cursorY); //Method to delete the current line and shift each line after it towards it by one.

			E.cursorY--;

		}
	}

}

void editorFreeRow(erow* row)  //Method to delete the strings of current line after it has been concatenated with string of previous                              //line after backspacing.
{
	free(row->chars);
	free(row->render);
}


void editorDelRow(int at)   //Method to free the string of current line and shift the strings after current line up by one.
{
	if(at < 0 || at >=E.numrows) //To check if cursor is at the end of the file or not, if it is present at the end of line
		//then do nothing.
	{
		return;
	}

	editorFreeRow(&E.row[at]);  //freeing the pointer of current line pointing to the string of current line.

	memmove(&E.row[at], &E.row[at+1], sizeof(erow) * (E.numrows - at -1));  //Since E.row is an array of erow type, we are using
	//memmove to shift the array towards the left by one										 //index. Here erow contais pointers to strings and int 									     // variable.

	E.numrows--; //as lines has moved up by one,so number of lines has reduced by one.
	E.dirty_flag++; //Indicating something is edited.


}


void editorRowAppendString(erow* row, char* s, size_t len)
{
	row->chars = realloc(row->chars,row->size+len+1);  //allocate more space to the string, to which another string is to be
	//concated.


	memcpy(&row->chars[row->size],s,len);      //Copying one string to the end of another string ( basically conctenating).

	row->size +=len;  //updating the size to new size.

	row->chars[row->size] = '\0'; //Forming a stirng by putting null character at the end of the array of characters.

	editorUpdateRow(row); //Method to update Render string as chars string is changed.

	E.dirty_flag++;  //Updating the dirty_flag to indicate change.
}


void editorEnterChars()
{
	if(E.renderX < E.row[E.cursorY].size && E.renderX > 0)
	{

	}
}


/********* Editor Operations ************/

void editorInsertChar(int c)    //Function to take character from keypress and call editorRowInsertChar() function to insert the
{				//character.
	if(E.cursorY == E.numrows)
	{
		editorInsertRow(E.numrows,"",0);
	}

	editorRowInsertChar(&E.row[E.cursorY],E.renderX,c);

	E.renderX++;
}


void editorInsertNewLines()    //Function to insert new lines when at the start of a line  or break current string and insert the 
{			       //end part of the string to next line.
	if(E.renderX==0)
	{
		editorInsertRow(E.cursorY,"",0);
	}
	else
	{
		erow* row = &E.row[E.cursorY];
		editorInsertRow(E.cursorY+1,&row->chars[E.renderX],row->size-E.renderX);

		row=&E.row[E.cursorY];

		row->size=E.renderX;

		row->chars[E.renderX]='\0';

		editorUpdateRow(row);
	}

	E.cursorY++;

	E.renderX=0;

}



/******** File I/O *********/

void editorOpen(char* filename)                //function to open the text file which is to be edited in the text editor.
{
	free(E.filename);

	E.filename = strdup(filename);

	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		die("fopen");
	}
	else
	{
		E.fileopen_flag = 0;
	}


	char line[100] = {0};
	size_t linecap=100;
	ssize_t linelen;

	while(1)
	{
		line[0] = 0;
		linecap = 100;

		if(NULL == fgets(line,linecap,fp))
		{
			linelen = -1;
		}
		else
		{
			linelen = strlen(line);
		}

		if(linelen != -1)
		{
			while(linelen >	0 && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
			{
				linelen--;
			}

			editorInsertRow(E.numrows,line,linelen);

		}
		else
		{

			fclose(fp);
			break;
		}
	}

	//	E.numrows+=2;

	E.dirty_flag=0;  //to set the dirty_flag to 0 after the file is opened for the first time as during opening the file, it
	//calls the fuction editorAppendRow(), which increments the dirty_flag but which is not correct to do, so
	//to compensate that, dirty_flag is set to 0 here.

}


/****** Append buffer ******/

//Append buffer is created so as to perform write() only one time everytime it refreshScreen is done as multiple write() create a
//flickering effect due to delay between multiple write().

struct abuf     //created a structure for building a dynamic string which is not available by default in C.
{
	char* b;
	int len;
};



void abAppend(struct abuf *ab, const char* s, int len)  //function to append to the dynamic allocated string and form a new string
{
	char* new = realloc(ab->b,ab->len + len);

	if(new == NULL)
	{
		return ;
	}

	memcpy(&new[ab->len],s,len);
	ab->b=new;
	ab->len +=len;
}

void abFree(struct abuf *ab)  //Function to deallocate memory to the dynamically allocated string at the end of refresh.
{
	free(ab->b);	
}


char* editorRowsToString(int* len)  
{
	int totlen = 0;

	char* buf;

	for(int i=0;i<E.numrows;i++)
	{
		totlen += E.row[i].size+1;
	}

	*len = totlen;

	buf = (char*) malloc(totlen);

	char* p = buf;

	for(int i=0;i<E.numrows;i++)
	{
		memcpy(p , E.row[i].chars , E.row[i].size);
		p +=E.row[i].size;

		*p = '\n';

		p++;
	}

	return buf;
}


void editorSave()       //function to write the contents of erow type array to file and thus save the changes.
{
	if(E.filename == NULL)
	{
		return;                        //not saving the file if filename is not provided as argument in command line.
	}

	int len;

	char* buf = editorRowsToString(&len);  //converting the whole content of file to a single string as it is the way it is
	// stored in a text file. All the lines compressed into a single string with each new
	//line marked by a \n


	int fd= open(E.filename,O_RDWR | O_CREAT | O_TRUNC, 0644); // returns a "file descriptor" which is a unique id for a open file ( file
	//stream). Open() function opens a file and thus establish a input/ouput
	//stream. O_RDWR is a flag which assigns permission to the stream for
	//reading and writing.O_CREAT is a flag which creates a file if any file
	//of the given filename doesn't exist. 0644 sets permissio for the file.
	//0644 allows owner of the file to read or write the file and read
	//permission for other users.
	//open(),O_RDWR,O_CREAT are from <fcntl.h>,(file control).


	if(fd != -1)
	{

		//if(ftruncate(fd,len) != -1)
		//{
		//Set the file size to the specified length of the string  which is to be writtten. ftruncate() is from
		//<unistd.h> (<unistd.h> provides access to the POSIX API.)

		if(write(fd,buf,len)==len)
		{
			close(fd);
			free(buf);
			E.dirty_flag=0; //Dirty_flag is set to 0 so as the file is saved and now all the changes have been saved.
			editorSetStatusMsg("%d bytes written to the disk",len);
			return;
		}

		close(fd);
		//}

	}
	editorSetStatusMsg("Can't Save! I/O error: %s", strerror(errno)); //strerror() has same job as perror() as both print error
	//message using the value stored in errno. Only difference
	//is that strerror() prints the error string through stdout
	//stream and perror() prints the error msg through stderr
	//stream.
	free(buf);


}


/******* Output ******/

void editorScroll()
{

	//E.renderX = 0;

	/*if(E.cursorY < E.numrows)
	{
		E.renderX = editorRowCxtoRx(&E.row[E.cursorY],E.cursorX);
	}
	 */

	if(E.cursorY < E.rowoff)     //For scrolling during Arrow_up keypress.(E.cursorY represents the position of the cursor in
	{			     // file not in the terminal).
		E.rowoff = E.cursorY;
	}

	if(E.cursorY >= (E.rowoff + E.screenrows))  //For scrolling the file beyond the visible region during Arrow_down keypress.
	{
		E.rowoff = E.cursorY -E.screenrows + 1;
	}

	if(E.renderX >= (E.coloff + E.screencols))
	{
		E.coloff = E.renderX - E.screencols + 1;
	}

	if(E.renderX < E.coloff)
	{
		E.coloff = E.renderX;
	}
}




void editorDrawRows(struct abuf *ab)
{       
	int y;
	int filerow;
	for(y=0;y<E.screenrows-2;y++)
	{
		filerow = y+E.rowoff;
		if(filerow>=E.numrows)
		{
			if(E.fileopen_flag && E.numrows==0 && y==E.screenrows/3)
			{
				abAppend(ab,"~",1);

				char welcome[80];
				int welcomelen = snprintf(welcome, sizeof(welcome),"Editor --version %s\r\n", EDITOR_VERSION);
				//snprintf() used to interpolate EDITOR_VERSION string to form the welcome string and hence form the welcome			   // message.

				if(welcomelen > E.screencols)
				{
					welcomelen = E.screencols;
				}

				int padding = (E.screencols - welcomelen)/2;

				while(padding > 0)
				{
					abAppend(ab," ",1);
					padding--;
				}

				abAppend(ab,welcome,welcomelen);

			}  else {

				abAppend(ab,"~\r\n",3);

			}
		}
		else
		{

			int len = E.row[filerow].rsize - E.coloff;

			if(len < 0)
			{
				len = 0;
			}

			if(E.screencols<len)
			{
				len = E.screencols;
			}

			abAppend(ab,&E.row[filerow].render[E.coloff],len);
			abAppend(ab,"\r\n",2);
		}

	}

	filerow=y+E.rowoff;

}


void editorDrawStatusBar(struct abuf* ab)
{
	//abAppend(ab, "\x1b[7m",4);

	char status[80];
	char rstatus[80];

	int len = snprintf(status, sizeof(status), "%.20s      --     %d lines %s", (E.filename ? E.filename : "[No filename]"), E.numrows, E.dirty_flag ? "(Modified)" : "");

	int rlen=snprintf(rstatus,sizeof(rstatus),"%d - %d   ", E.cursorY+1,E.numrows);

	abAppend(ab,status, len);

	if(len > E.screencols)
	{
		len = E.screencols;
	}

	while(len < E.screencols)
	{
		if(E.screencols - len == rlen)
		{
			abAppend(ab,rstatus,rlen);
			break;
		}
		else
		{
			abAppend(ab," ",1);
			len++;
		}
	}

	//abAppend(ab, "\x1b[m",3);
	abAppend(ab, "\r\n",2);
}


void editorDrawMessageBar(struct abuf* ab)  //It draws the message bar and displays the message for five seconds. It is called 
{                                           //inside the refreshscreen() function.
	//abAppend(ab, "\x1b[K",3);
	//abAppend(ab, VT100_ERASE_LINE_FROM_CUR, sizeof(VT100_ERASE_LINE_FROM_CUR));
	int msglen = strlen(E.statusmsg);
	if(msglen > E.screencols)
	{
		msglen = E.screencols;
	}


	abAppend(ab,E.statusmsg,msglen);
	abAppend(ab, "\r\n",2);

}



void editorRefreshScreen()   // To render the editor screen in the terminal.
{
	editorScroll();

	struct abuf ab=ABUF_INIT;

	printf(VT100_CURSOR_OFF);
	printf(VT100_CLEAR_SCREEN);
	printf(VT100_CURSOR_HOME);

	editorDrawRows(&ab);    //Function to draw the contents of the file on the screen.


	editorDrawStatusBar(&ab);  //Function to draw the status bar at the bottom of the page.
	editorDrawMessageBar(&ab);



	char buf[32];
	/*
	if(E.cursorX > (E.row[E.cursorY].size-1))
	{
		E.cursorX = E.row[E.cursorY].size-1;
	}
	 */

	int length=snprintf(buf,sizeof(buf),"\033[%i;%iH" ,(E.cursorY-E.rowoff + 1),(E.renderX-E.coloff + 1));

	//E.cursorY-E.rowoff+1 actually set the position of the cursor before displaying the cursor in the visible region of the 
	//terminal eventhough E.cursorY actually represents the position of the cursor in the file, not int the terminal.

	abAppend(&ab,buf,length);

	write(fileno(stdout),ab.b,ab.len);
	abFree(&ab);
	printf(VT100_CURSOR_ON);
	fflush((stdout));
}


void editorSetStatusMsg(const char *fmt, ...)    //Variadic function : used to point E.statusmsg to the string we want to print as
//msg. Also set E.statusmsg_time. (Store the message we want to print).
{
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg),fmt,ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}



/***** Input *******/

void editorMoveCursor(int key)
{
	switch(key)
	{
	case Arrow_up:
	{
		if(E.cursorY!=0)
		{
			E.cursorY--;
		}

		if(E.renderX > (E.row[E.cursorY].rsize-1))
		{
			E.coloff = E.row[E.cursorY].rsize - E.screencols + 1;
			if(E.coloff < 0)
			{
				E.coloff = 0;
			}

			E.renderX = E.row[E.cursorY].rsize;
		}
	}

	break;

	case Arrow_down:
	{

		if(E.cursorY <E.numrows)
		{
			E.cursorY++;

			if(E.cursorY == E.numrows && E.row[E.cursorY-1].rsize>=1)
			{
				for(int i=0;i<4;i++)
				{
					editorInsertRow(E.numrows,"",0);
				}
			}

			if(E.cursorY ==E.numrows)
			{
				if(E.row[E.cursorY-1].rsize<1)
				{
					E.cursorY--;
				}
			}


		}

		if(E.cursorX > (E.row[E.cursorY].rsize-1))
		{
			E.coloff = E.row[E.cursorY].rsize - E.screencols + 1;
			if(E.coloff < 0)
			{
				E.coloff = 0;
			}

			E.renderX = E.row[E.cursorY].rsize;

		}
	}
	break;

	case Arrow_left:
		if(E.renderX!=0)
		{
			E.renderX--;
		}
		else
		{
			if(E.cursorY > 0)
			{
				E.cursorY--;
				E.renderX = E.row[E.cursorY].rsize;
			}
		}

		break;

	case Arrow_right:
	{
		if(E.renderX < E.row[E.cursorY].rsize)
		{
			E.renderX++;
		}
		else
		{
			if(E.cursorY < E.numrows-1)
			{
				E.cursorY++;

				E.renderX = 0;
			}
		}

	}
	break;

	}

	/*	erow *row = (E.cursorY >= E.numrows) ? NULL : &E.row[E.cursorY];

	int rowlen = row ? row ->size : 0;

	if(E.renderX > rowlen)
	{
		E.renderX = rowlen;
	}
	 */

}



void editorProcessKeypress() {
	int c= editorReadKey();

	switch(c)
	{
	case CTRL_KEY('q'):
				{
		if(E.dirty_flag && global->quit_times  > 0)
		{
			editorSetStatusMsg("WARNING!! File has unsaved changes. Press Ctrl-Q %d more times to quit", global->quit_times );
			global->quit_times --;
			return ;
		}

		write(fileno(stdout), "\x1b[2J", 4);
		write(fileno(stdout), "\x1b[H", 3);
		disableRawMode();
		exit(0);

				}
	break;

	case Arrow_up:
	case Arrow_down:
	case Arrow_left:
	case Arrow_right:
		editorMoveCursor(c);
		break;	

	case Page_up:
	case Page_down:
	{
		if(c == Page_up)
		{
			E.cursorY = E.rowoff;

			if(E.renderX > (E.row[E.cursorY].rsize - 1))
			{
				E.coloff = E.row[E.cursorY].rsize - E.screencols + 1;

				if(E.coloff < 0)
				{
					E.coloff = 0;

				}

				E.renderX = E.row[E.cursorY].rsize;
			}



		}
		else
		{
			if(c == Page_down)
			{
				E.cursorY = E.rowoff + (E.screenrows - 1);

				if(E.cursorY > (E.numrows-1))
				{
					E.cursorY = E.numrows-1;
				}

				if(E.renderX > (E.row[E.cursorY].rsize -1))
				{
					E.coloff = E.row[E.cursorY].rsize - E.screencols + 1;

					if(E.coloff < 0)
					{
						E.coloff =0 ;
					}

					E.renderX = E.row[E.cursorY].rsize;
				}

			}

		}
	}

	break;

	case End:     //Keypress for moving the cursor to end of line.
	case Home:    //Keypress for moving the cursor to start of line.
	{
		if(c == End)
		{
			E.coloff = E.row[E.cursorY].rsize - E.screencols + 1;

			if(E.coloff < 0)
			{
				E.coloff = 0;
			}

			E.renderX= E.row[E.cursorY].rsize;
		}
		else
		{
			if(c == Home)
			{
				E.coloff = 0;

				E.renderX = 0;
			}
		}
	}

	break;

	case Del: {   //Delete Keypress deletes the character at the cursor position.


		editorMoveCursor(Arrow_right);// Since editorDelChar() method deletes the chacracter left of the cursor
		//position, so it is required to move the cursor one position right to delete
		//the current character at the cursor position.
		editorDelChars();


	}

	break;

	case CTRL_KEY('s'):       //mapping ctrl+s keypress to saving the changes in the file functionality.
							editorSave();

	break;

	case BackSpace:
	case CTRL_KEY('h'):       //ctrl + h has a value of 8 which is same as value of ASCII value of backspace in old
	// version.
	editorDelChars();

	break;

	case Enter: editorInsertNewLines();

	break;

	case CTRL_KEY('l'):

						break;

	case '\x1b':

		break;

	default: editorInsertChar(c);

	break;
	}

	global->quit_times  = EDITOR_QUIT_TIMES; //reset global->quit_times  if before completing pressing of Ctrl-Q three times consecutively, any
	//other key is pressed.

}




/******* Init *******/

void initEditor()
{

	if( getWindowSize(&E.screenrows, &E.screencols)==-1)
	{
		die("getWindowSize");
	}
}






int main(int argc, char *argv[])
{
	enableRawMode();
	E.cursorX=0;
	E.cursorY=0;
	E.renderX=0;	
	E.numrows=0;
	E.fileopen_flag= 1;
	E.rowoff=0;
	E.coloff=0;
	E.row = NULL;
	E.filename = NULL;
	E.statusmsg[0]='\0';
	E.statusmsg_time=0;
	E.dirty_flag=0;
	global->quit_times = EDITOR_QUIT_TIMES;
	if(argc >=2)
	{
		editorOpen(argv[1]);
	}

	editorSetStatusMsg("HELP: Ctrl-Q = quit | Ctrl-S = Save");

	while(1)
	{
		initEditor();	
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}



/*==============================================================================
  End of file
==============================================================================*/


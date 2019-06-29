/**********************************************************************
Filename:				scanner.c
Version: 				1.0                                         
Author:					Eric Dodds                                              
Student No:  			040-701-142                                              
Course Name/Number:		Compilers	CST8152	
Lab Sect:				402		
Assignment #:			2
Assignment name:		Buffer	
Due Date:				October 25th 2013        
Submission Date:		October 25th 2013
Professor:				Svillen Ranev                                           
Purpose:				
Files:					buffer.c buffer.h scanner.c token.h table.h
***************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>   /* standard input / output */
#include <ctype.h>   /* conversion functions */
#include <stdlib.h>  /* standard library functions and constants */
#include <string.h>  /* string functions */
#include <limits.h>  /* integer types constants */
#include <float.h>   /* floating-point types constants */

/*#define NDEBUG        to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

#define False -1
#define shortmax 32767
#define shortmin -32768

/* Global objects - variables */
/* This buffer is used as a repository for string literals.
   It is defined in platy_st.c */
extern Buffer * str_LTBL; /*String literal table */
int line; /* current line number of the source code */
extern int scerrnum;     /* defined in platy_st.c - run-time error number */

/* Local(file) global objects - variables */
static Buffer *lex_buf;/*pointer to temporary lexeme buffer*/

/* scanner.c static(local) function  prototypes */ 
static int char_class(char c); /* character class function */
static int get_next_state(int, char, int *); /* state machine function */
static int iskeyword(char * kw_lexeme); /*keywords lookup functuion */
static long atool(char * lexeme); /* converts octal string to decimal value */

/***************************************************************************
Name:						scanner_init()
Purpose:					Checks if the buffer is empty, and resets the getc_offset
Function In Parameters:		Buffer * sc_buf
Function Out Parameters:	Returns 1 on failure, returns 0 on success
Version:					1.0
Author:						Svillen Ranev
***************************************************************************/
int scanner_init(Buffer * sc_buf) {
  	if(b_isempty(sc_buf)) return EXIT_FAILURE;/*1*/
	b_set_getc_offset(sc_buf,0);/* in case the buffer has been read previously  */
	b_reset(str_LTBL);
	line = 1;
	return EXIT_SUCCESS;/*0*/
}

/***************************************************************************
Name:						mlwpar_next_token()
Purpose:					Gets a character at a time and processes and results the appopriate token
							and also contains the FSM used to set the appropriate state from the transition table
Function In Parameters:		Buffer * sc_buf
Function Out Parameters:	Returns Token t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token mlwpar_next_token(Buffer * sc_buf)
{
   Token t; /* token to return after recognition */
   unsigned char c; /* input symbol */
   int state = 0; /* initial state of the FSM */
   short lexstart;  /*start offset of a lexeme in the input buffer */
   short lexend;    /*end   offset of a lexeme in the input buffer */
   int accept = NOAS; /* type of state - initially not accepting */  
   char cnext; /*Used to load the next character after c*/
   int x; /*Used for for loops*/
   short size; /*Handle*/
   int count; /*Used to keep track of error string length*/
   int charCount = 0; /*Used to count number of chars loaded*/
/* 
lexstart is the offset from the beginning of the char buffer of the
input buffer (sc_buf) to the first character of the current lexeme,
which is being processed by the scanner.
lexend is the offset from the beginning of the char buffer of the
input buffer (sc_buf) to the last character of the current lexeme,
which is being processed by the scanner.
*/ 
    while (1) /*Endless loop*/
	{ 
		/*printf("Line %d\n", line); Used to test the line count of the program*/
		c = b_getc(sc_buf); /*Gets the next character*/

		if(isspace(c) && c != '\n' && c != '\r') /*Check for whitespace*/
			continue;
		switch(c){
		case('\r'): /*Next two cases account for new lines*/
			line++; /*Increments the line count*/
			continue;
		case('\n'):
			line++; /*Increments the line count*/
			continue;
		case('('): /*Processes left parenthesis in buffer*/
			t.code = LPR_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case(')'): /*Processes right parenthesis in buffer*/
			t.code = RPR_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case('{'): /*Processes left brace in buffer*/
			t.code = LBR_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case('}'): /*Processes right brace in buffer*/
			t.code = RBR_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case(','): /*Processes commas in buffer*/
			t.code = COM_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case(';'): /*Processes semi colons in buffer*/
			t.code = EOS_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		case('+'): /*Processes + symbols in buffer*/
			t.code = ART_OP_T; /*Sets the appropiate token code*/
			t.attribute.arr_op = PLUS;
			return t; /*Returns the token*/
		case('-'): /*Processes - symbols in buffer*/
			t.code = ART_OP_T; /*Sets the appropiate token code*/
			t.attribute.arr_op = MINUS;
			return t; /*Returns the token*/
		case('*'): /*Processes asterisks in buffer*/
			t.code = ART_OP_T; /*Sets the appropiate token code*/
			t.attribute.arr_op = MULT;
			return t; /*Returns the token*/
		case('/'): /*Processes forward slashes in buffer*/
			t.code = ART_OP_T; /*Sets the appropiate token code*/
			t.attribute.arr_op = DIV;
			return t; /*Returns the token*/
		case('>'): /*Processes greater than sign in buffer*/
			t.code = REL_OP_T; /*Sets the appropiate token code*/
			t.attribute.rel_op = GT;
			return t; /*Returns the token*/
		case('"'): /*Processes quotations in buffer*/
			size = b_getsize(str_LTBL); /*Gets the current size of the String table*/
			b_setmark(sc_buf, b_get_getc_offset(sc_buf)); /*Sets the mark at the start of the string*/
			cnext = b_getc(sc_buf); /*Check the next character*/
			while(cnext != '"') /*Loop until it finds next quotation mark*/
			{
				if(c == '\n' || c == '\r')
					line++; /*Increments the line count*/
				if(cnext == '\0' || cnext == '255')
				{
					t.code = ERR_T; /*Sets the error token code*/
					if(charCount > ERR_LEN)
					{
						b_set_getc_offset(sc_buf, b_getmark(sc_buf));
						b_retract(sc_buf);
						for(x = 0; x < 20; x++) /*Loops and adds the string to the error lexeme*/
						{
							t.attribute.err_lex[x] = b_getc(sc_buf);
							if(x > 16)
								t.attribute.err_lex[x] = '.'; /*Adds ... to signify string longer than allowed error length*/
							
						}
						t.attribute.err_lex[x] = '\0'; /*Make C type string*/
					}
					else
					{
						b_set_getc_offset(sc_buf, b_getmark(sc_buf));
						b_retract(sc_buf);
						for(x = 0; x <= charCount; x++) /*Loops and adds the string to the error lexeme*/
							t.attribute.err_lex[x] = b_getc(sc_buf);
						t.attribute.err_lex[charCount+1] = '\0'; /*Make C type string*/
						t.code = ERR_T;
					}
					while(c != '\0' && c!= '255') /*If no extra quotation found, move forward until EOF*/
						c = b_getc(sc_buf);
					b_retract(sc_buf); /*Retracts so that EOF can be processed seperately*/
					return t; /*Returns the token*/
				}
				else
				{
					cnext = b_getc(sc_buf); /*Gets the next character*/
					charCount++; /*Adds 1 to the character count of string length*/
				}
			}
			if(cnext == '"')
			{
				b_retract(sc_buf);
				count = b_get_getc_offset(sc_buf); /*Keeps track of the string size*/
				b_set_getc_offset(sc_buf, b_getmark(sc_buf)); /*Set the getc_offset to the start of the string*/

				while(b_get_getc_offset(sc_buf) < count)
					b_addc(str_LTBL, b_getc(sc_buf));
				
				b_addc(str_LTBL, '\0'); /*Make C type string*/
				b_getc(sc_buf); /*Move forward one so the " isnt picked up on next char*/
				t.code = STR_T; /*Sets the appropiate token code*/
				t.attribute.str_offset = size; /*Set the appropriate attribute*/
				return t; /*Returns the token*/
			}
		case('!'): /*Processes exclamation points in buffer*/
			cnext = b_getc(sc_buf); /*Get the next character*/
			if(cnext == '<')/*Check if valid quote*/
			{
				while(cnext != '\n')
					cnext = b_getc(sc_buf);
				line++; /*Increments the line counter*/
				if(cnext == '\0') /*Check for end of file*/
					b_retract(sc_buf);
				continue;
			}
			else if(cnext == '=')
			{
				t.code = REL_OP_T; /*Sets the appropiate token code*/
				t.attribute.rel_op = NE; /*Sets the appropriate attribute*/
				return t; /*Returns the token*/
			}
			else
			{
				t.code = ERR_T; /*Sets the appropiate token code*/
				t.attribute.err_lex[0] = '!'; /*Load the error lexeme with the necessary chars*/
				t.attribute.err_lex[1] = cnext;
				t.attribute.err_lex[2] = '\0'; /*Make C type string*/
				while(cnext != '\n') /*Move forward until the end of the line*/
					cnext = b_getc(sc_buf); /*Get the next character*/
				line++; /*Increments the line count*/
				return t; /*Returns the token*/
			}
		case('='):
			cnext = b_getc(sc_buf); /*Gets the next character*/
			if(cnext == '=')
			{
				b_set_getc_offset(sc_buf, b_get_getc_offset(sc_buf) + 1);
				t.code = REL_OP_T; /*Sets the appropriate token code*/
				t.attribute.rel_op = EQ; /*Sets the appropriate attribute*/
				return t; /*Returns the token*/
			}
			else
			{
				t.code = ASS_OP_T; /*Sets the appropiate token code*/
				b_retract(sc_buf);
				return t; /*Returns the token*/
			}
		case('<'): /*Processes less than in buffer*/
			cnext = b_getc(sc_buf);
			if(cnext == '>')
			{
				t.code = SCC_OP_T; /*Sets the appropiate token code*/
				return t; /*Returns the token*/
			}
			else
			{
				b_retract(sc_buf);
				t.code = REL_OP_T; /*Sets the appropiate token code*/
				t.attribute.rel_op = LT;
				return t; /*Returns the token*/
			}
		case('.'): /*Processes periods in buffer. Checks the following characters to see if either AND. or OR. follows the first .*/
			if(*b_get_chmemloc(sc_buf,sc_buf->getc_offset) == 'A' && *b_get_chmemloc(sc_buf,sc_buf->getc_offset+1) == 'N' &&  
			   *b_get_chmemloc(sc_buf,sc_buf->getc_offset+2) == 'D' && *b_get_chmemloc(sc_buf,sc_buf->getc_offset+3) == '.' )
			{
				sc_buf->getc_offset += 4;/*Increments the offset to account for finding the word AND.*/
				t.code = LOG_OP_T; /*Sets the appropiate token code*/
				t.attribute.log_op = AND;
				return t; /*Returns the token*/
			}
			else if(*b_get_chmemloc(sc_buf,sc_buf->getc_offset) == 'O' && *b_get_chmemloc(sc_buf,sc_buf->getc_offset+1) == 'R' &&  
					*b_get_chmemloc(sc_buf,sc_buf->getc_offset+2) == '.')
			{
				sc_buf->getc_offset += 3;/*Increments the offset to account for finding the word OR.*/
				t.code = LOG_OP_T; /*Sets the appropiate token code*/
				t.attribute.log_op = OR;
				return t; /*Returns the token*/
			}
			else
			{
				t.code = ERR_T; /*Sets the appropiate token code*/
				t.attribute.err_lex[0] = c;
				t.attribute.err_lex[1] = '\0';
				return t; /*Returns the token*/
			}
		case(SEOF): /*Processes Sentinal end of file symbol in buffer*/
			t.code = SEOF_T; /*Sets the appropiate token code*/
			return t; /*Returns the token*/
		 /* Process state transition table */  
		case('\0'):
			t.code = SEOF_T;
			return t; /*Returns the token*/
		}
		if(isalnum(c))
		{
			state = 0;
			b_setmark(sc_buf, b_get_getc_offset(sc_buf));
			while(accept == NOAS)
			{
				state = get_next_state(state, c, &accept);
				c = b_getc(sc_buf);
			}

			if(accept == ASWR)
				b_retract(sc_buf);

			if(c != SEOF)
				b_retract(sc_buf);

			lexstart = b_getmark(sc_buf);
			lexend = b_get_getc_offset(sc_buf);
			lex_buf = b_create(200, 0,'f'); /*Create buffer*/ 

			if(lex_buf == NULL) /*Check for runtime error*/
			{
				scerrnum = 0;
				t.code = ERR_T;
				strcpy(t.attribute.err_lex, "Run Time Error: ");
				return t; /*Returns the token*/
			}

			b_set_getc_offset(sc_buf, lexstart);
			b_retract(sc_buf);
			
			while(b_get_getc_offset(sc_buf) < lexend) /*Load lex_buf with lexeme*/
				b_addc(lex_buf, b_getc(sc_buf));

			b_addc(lex_buf,'\0'); /*Make string c-type and set token*/
			t = aa_table[state](b_get_chmemloc(lex_buf, 0));
			b_destroy(lex_buf); /*Free the lex buffer*/
			return t; /*Returns the token*/
		}
		else
		{
			t.code = ERR_T; /*Sets the error code*/
			t.attribute.err_lex[0] = c; /*Copies the char into error lexeme*/
			t.attribute.err_lex[1] = '\0'; /*Makes C type string*/
			return t; /*Returns the token*/
		}
	}
}

/***************************************************************************
Name:						get_next_state()
Purpose:					Sets a value representing the column in the transition table
Function In Parameters:		int state, char c, int *accept
Function Out Parameters:	Returns val
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int get_next_state(int state, char c, int *accept)
{
	int col;
	int next;
	col = char_class(c);
	next = st_table[state][col];
#ifdef DEBUG
printf("Input symbol: %c Row: %d Column: %d Next: %d \n",c,state,col,next);
#endif

       assert(next != IS);
 
#ifdef DEBUG
	if(next == IS){
	  printf("Scanner Error: Illegal state:\n");
	  printf("Input symbol: %c Row: %d Column: %d\n",c,state,col);
	  exit(1);
	}
#endif
	*accept = as_table[next];
	return next;
}

int char_class (char c)
{
        int val = 0;

		if(isalpha(c)) /*If c is an alpha character set row value*/
			val = 0;

		else if(isdigit(c)) /*If c is an digit set row value*/
		{
			if(c == '0')
				val = 1;

			else if(c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7')
				val = 2;

			else if(c == '8' || c =='9')
				val = 3;
		}
		else if(c == '.')
			val = 4;

		else if(c == '#')
			val = 5;

		else /*If c is any other character set row value*/
			val = 6;
        
        return val;
}

/***************************************************************************
Name:						aa_func02()
Purpose:					Sets a code and attribute depending on whether lexeme is a keyword or not
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func02(char lexeme[]){

	Token t; /*Creates token*/
	int length; /*Checks the length of the lexeme string*/
	int x; /*Used for looping*/
	int keyindex = iskeyword(lexeme); /*Calls the iskeyword function and returns an index position if a match is found*/

	if(keyindex >= 0)
	{
		t.code = KW_T; /*Sets the appropriate code*/
		t.attribute.kwt_idx = keyindex; /*Sets the appropriate attribute*/
		return t; /*Returns the token*/
	}

	length = strlen(lexeme); /*Store the length of the lexeme*/
	if(length > VID_LEN) /*If longer than VID_LEN we only want the first VID_LEN chars so length is adjusted to that*/
		length = VID_LEN;

	for(x = 0; x < length; x++) /*Copies in the error lexeme*/
		t.attribute.vid_lex[x] = lexeme[x];

	t.attribute.vid_lex[length] = '\0'; /*Makes C type string*/
	t.code = AVID_T; /*Sets the appropriate code*/
	return t; /*Returns the token*/
}

/***************************************************************************
Name:						aa_func03()
Purpose:					Acceptign function for the string variable identifier
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func03(char lexeme[]){

	Token t; /*Creates token*/
	int length; /*Checks the length of the lexeme string*/
	int x; /*Used for looping*/

	length = strlen(lexeme); /*Store the length of the lexeme*/
	if(length > VID_LEN)
		length = VID_LEN-1;

	for(x = 0; x < length; x++) /*Loops through and sets the attribute to the lexeme*/
		t.attribute.vid_lex[x] = lexeme[x];

	if(strlen(lexeme) > VID_LEN - 1)
	{
		t.attribute.vid_lex[length] = '#'; /*Add the # to make valid SVID*/
		length++;
	}

	t.attribute.vid_lex[length] = '\0'; /*Makes C type string*/
	t.code = SVID_T; /*Sets the appropriate code*/
	return t; /*Returns the token*/
}

/***************************************************************************
Name:						aa_func08()
Purpose:					Acceptign function for the floating-point literal
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func08(char lexeme[]){

	Token t; /*Creates token*/
	double temp; /*Used to store the lexeme value*/
	int x; /*Used for looping*/

	temp = atof(lexeme); /*Converts the lexeme to a double*/
	if(temp == 0 || temp > 0 && temp <= FLT_MAX && temp >= FLT_MIN) /*Checks if the lexeme value is greater than the capacity of float*/
	{
		t.code = FPL_T;
		t.attribute.flt_value = (float)temp;
		return t; /*Returns the token*/
	}
	else
	{		
		t.code = ERR_T; /*Sets the appropiate token code*/
		for(x = 0; x < ERR_LEN; x++)
			t.attribute.err_lex[x] = lexeme[x];
		t.attribute.err_lex[ERR_LEN] = '\0';
		return t; /*Returns the token*/
	}
}

/***************************************************************************
Name:						aa_func05()
Purpose:					Acceptign function for the integer literal - decimal constant
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func05(char lexeme[]){

	Token t; /*Creates token*/
	double temp; /*Used to store the lexeme value*/
	int x; /*Used for looping*/

	temp = atof(lexeme); /*Converts the lexeme to a double*/
	if (temp == 0 || temp > 0 && temp <= shortmax && temp >= shortmin)  /*Checks if the lexeme value is greater than the capacity of float*/
	{
		t.code = INL_T; /*Sets the appropiate token code*/
		t.attribute.int_value = (int)temp; /*Set the appropriate attribute*/
		return t; /*Returns the token*/
	}
	else
	{
		t.code = ERR_T; /*Sets the appropiate token code*/
		for(x = 0; x < ERR_LEN; x++) /*Loop and copy in the error lexeme*/
			t.attribute.err_lex[x] = lexeme[x];
		t.attribute.err_lex[ERR_LEN] = '\0'; /*Make C type string*/
		return t; /*Returns the token*/
	}
}

/***************************************************************************
Name:						aa_func11()
Purpose:					Acceptign function for the integer literal - octal constant
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func11(char lexeme[]){

	Token t;
	long temp;
	int x;

	temp = atool(lexeme); /*Converts the lexeme to a double*/
	if (temp >= 0 && temp <= SHRT_MAX) /*Checks if the lexeme value is greater than the capacity of float*/
	{
		t.code = INL_T; /*Sets the appropiate token code*/
		t.attribute.int_value = (int)temp;
		return t; /*Returns the token*/
	}
	else /*If less than the capacity of long then set the value to the token*/
	{
		t.code = ERR_T; /*Sets the appropiate token code*/
		for(x = 0; x < ERR_LEN; x++)
			t.attribute.err_lex[x] = lexeme[x];
		t.attribute.err_lex[x] = '\0';
		return t; /*Returns the token*/
	}
}

/***************************************************************************
Name:						aa_func12()
Purpose:					Acceptign function for the error token
Function In Parameters:		char lexeme[]
Function Out Parameters:	Returns t
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Token aa_func12(char lexeme[]){

	Token t; /*Creates token*/
	int x; /*Used for looping*/

	t.code = ERR_T; /*Sets the appropiate token code*/
	if(strlen(lexeme) > ERR_LEN) /*If the error token is longer than ERR_LEN only take the first ERR_LEN character*/
	{
		for(x = 0; x < ERR_LEN; x++)
			t.attribute.err_lex[x] = lexeme[x];
		t.attribute.err_lex[ERR_LEN] = '\0'; /*Make C type string*/
		return t; /*Returns the token*/
	}
	else /*Otherwise set err_lex to the full lexeme*/
	{
		t.code = ERR_T; /*Sets the appropiate token code*/
		strcpy(t.attribute.err_lex, lexeme);
		return t; /*Returns the token*/
	}
}

/***************************************************************************
Name:						atool()
Purpose:					Converts an ASCII string representing an octal constant
							to integer value
Function In Parameters:		char * lexeme
Function Out Parameters:	Returns rem
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
long atool(char * lexeme){

	long octal = 0;
	long decimal = 0;
	int power = 1;
	
	octal = atol(lexeme); /*Converts the lexeme into an integer value*/
	while(octal != 0)
	{
		decimal += (octal % 10) * power; /*Takes the mod 10 of the octal value multiplied by x*/
		octal /= 10; /*Divide octal value by 10*/
		power *= 8; /*Mutliply the number recieved from the mod line by 8*/
	}
	return decimal; /*Return the value converted to base 10*/
}

/***************************************************************************
Name:						iskeyword()
Purpose:					Checks if the lexeme matches a keyword in the table
Function In Parameters:		char * lexeme
Function Out Parameters:	Returns the index in the table if keyword and -1 if not found
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int iskeyword(char * lexeme){
	
	int x; /*Used for looping*/
	
	for(x = 0; x < KWT_SIZE; x++) /*Look through the keyword table*/
	{
		if(strcmp(kw_table[x], lexeme) == 0) /*Check if the lexeme matches a keyword in the table*/
			return x; /*Returns the index in the keyword table*/
	}
	return False; /*Returns -1 on no match*/
}
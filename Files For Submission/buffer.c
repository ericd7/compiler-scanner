/**********************************************************************
Filename:				buffer.c
Version: 				1.0                                         
Author:					Eric Dodds                                              
Student No:  			040-701-142                                              
Course Name/Number:		Compilers	CST8152	
Lab Sect:				402		
Assignment #:			1
Assignment name:		Buffer	
Due Date:				September 25th 2013        
Submission Date:		September 25th 2013
Professor:				Svillen Ranev                                           
Purpose:				To implement a buffer which will read in characters from a file and expand the buffer based
						on command line parameters given at run time.
Files:					buffer.c buffer.h platy_bt.c
***************************************************************************/
#include "buffer.h"

/***************************************************************************
Name:						b_create
Purpose:					Creates a new buffer in memory, checks for abnormal
							function parameters the function
Function In Parameters:		short init_capacity, char inc_factor, char o_mode
Function Out Parameters:	Returns NULL on failure, returns pointer to buffer on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Buffer * b_create (short init_capacity,char inc_factor,char o_mode)
{

	Buffer* temp; /*Creates a buffer structure*/
	temp = (Buffer*)calloc(1,sizeof(Buffer)); /*allocates memory for the buffer*/
	if(temp == NULL)
		return NULL;
	temp->ca_head = (char*)malloc(init_capacity); /*allocates memory for the characters to be input into the buffer*/
	/*Handles all of the different command line input parameters and increment factors*/
	if(o_mode == 'f' || (unsigned char)inc_factor == 0)
	{
		temp->mode = 0;
		temp->inc_factor = 0;
	}
	else if(o_mode == 'f' && (unsigned char)inc_factor != 0)
	{
		temp->mode = 0;
		temp->inc_factor = 0;
	}
	else if(o_mode == 'a' && (unsigned char)inc_factor >= 1 && (unsigned char)inc_factor <= 255)
	{
		temp->mode = 1;
		temp->inc_factor = (unsigned char)inc_factor;
	}
	else if(o_mode == 'm' && (unsigned char)inc_factor >= 1 && (unsigned char)inc_factor <= 100)
	{
		temp->mode = -1;
		temp->inc_factor = (unsigned char)inc_factor;
	}
	else
	{
		return NULL;
	}
	temp->capacity = init_capacity;
	return temp;
}

/***************************************************************************
Name:						b_addc
Purpose:					Resets r_flag and tries to add a character to the array.
Function In Parameters:		Buffer * const pBD, char symbol
Function Out Parameters:	Returns NULL on failure, returns pointer to buffer on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Buffer *b_addc (Buffer * const pBD, char symbol)
{
	if(pBD == NULL)
		return NULL;
	else
	{
		short available; /*Variables to be used in the calculation of the multiplicative buffer size increase*/
		short capacity;
		short newIncrement;
		pBD->r_flag = 0;
		if (pBD->addc_offset < pBD->capacity) /*If we are withing the bounds of the buffer add the char*/
		{
			pBD->ca_head[pBD->addc_offset] = symbol;
			if(pBD->addc_offset != SHRT_MAX)
			{
				pBD->addc_offset++;
			}
			return pBD;
		}
		else
		{
			/*Decide how to increment based on the mode*/
			if(pBD->mode == 0)
				return NULL;

			else if(pBD->mode == 1) /*Handles the expansion for additive mode*/
			{
				if((short)(pBD->inc_factor + pBD->capacity) < 0)
				{
					pBD->ca_head = (char*)realloc(pBD->ca_head, SHRT_MAX);
					if(pBD->ca_head == NULL) /*Makes sure the realloc was successful*/
					{
                        return NULL;
					}
					pBD->capacity = SHRT_MAX;
				}
				else
				{
					pBD->ca_head = (char*)realloc(pBD->ca_head, (pBD->inc_factor + pBD->capacity));
					if(pBD->ca_head == NULL)/*Makes sure the realloc was successful*/
					{
                        return NULL;
					}
					pBD->capacity += pBD->inc_factor;
				}
			}
			else if(pBD->mode == -1) /*Handles the expansion for multiplicative mode*/
			{
				/*Goes through the equasion to expand*/
				available = SHRT_MAX - pBD->capacity;
				newIncrement = available * pBD->inc_factor / 100;
				capacity = (short)(pBD->capacity + newIncrement);

				if((short)(newIncrement + pBD->capacity) < pBD->capacity)
				{
					pBD->ca_head = (char*)realloc(pBD->ca_head, SHRT_MAX);
					if(pBD->ca_head == NULL) /*Makes sure the realloc was successful*/
					{
                        return NULL;
					}
					pBD->capacity = SHRT_MAX;
				}
				else
				{
					pBD->ca_head = (char*)realloc(pBD->ca_head, (pBD->capacity + newIncrement));
					if(pBD->ca_head == NULL) /*Makes sure the realloc was successful*/
					{
                        return NULL;
					}
					pBD->capacity = (short)(pBD->capacity + newIncrement);
				}
			}
			/*After expanding the buffer tries to add the character again*/
			pBD->r_flag = SET_R_FLAG;
			if (pBD->addc_offset < pBD->capacity)
			{
				pBD->ca_head[pBD->addc_offset] = symbol;
				if(pBD->addc_offset != SHRT_MAX)
				{
					pBD->addc_offset++;
				}
				return pBD;
			}
		}
	}
	return pBD;	
}



/***************************************************************************
Name:						b_reset
Purpose:					Keeps the currently assigned memory but resets the offset back
							to the start of the array
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_reset(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	/*Resets all of the offset markers to 0 ready to be reused but keeps the memory intact*/
	pBD->addc_offset = 0;
	pBD->mark_offset = 0;
	pBD->inc_factor = 0;
	pBD->r_flag = 0;
	pBD->eob = 0;
	return 0;
}

/***************************************************************************
Name:						b_destroy
Purpose:					Frees the allocated memory for the buffer
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
void b_destroy(Buffer * const pBD)
{
	if(pBD == NULL)
		return;
	free(pBD->ca_head); /*Frees the head first to avoid dangling pointer*/
	free(pBD);
	return;
}

/***************************************************************************
Name:						b_isfull
Purpose:					Checks if the character buffer is full
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns 1 on space in buffer and 0 otherwise
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_isfull(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	
	if(b_getcapacity(pBD) == pBD->addc_offset)
		return 1;
	else
		return 0;
}

/***************************************************************************
Name:						b_getsize
Purpose:					Returns the current size of the buffer
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns the current size of the buffer on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
short b_getsize(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->addc_offset;
}

/***************************************************************************
Name:						b_getcapacity
Purpose:					Returns the current capacity of the buffer
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns capacity of buffer on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
short b_getcapacity(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->capacity;
}

/***************************************************************************
Name:						b_setmark
Purpose:					Sets mark_offset to mark, mark must be within capacity of buffer
Function In Parameters:	    Buffer * const pBD, short mark
Function Out Parameters:	Returns -1 on failure, returns mark on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_setmark(Buffer * const pBD, short mark)
{
	if(pBD == NULL)
		return R_FAIL_1;

	if(pBD->mark_offset <= (short)pBD->capacity)
	{
		pBD->mark_offset = mark; /*Sets the mark offset equal to mark*/
		return (int)mark;
	}
	else
		return R_FAIL_1;
}

/***************************************************************************
Name:						b_getmark
Purpose:					Returns the mark_offset of the buffer
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns mark_offset on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_getmark(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->mark_offset;
}

/***************************************************************************
Name:						b_getmode
Purpose:					returns the value of mode
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns mode on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_getmode(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->mode;
}

/***************************************************************************
Name:						b_load
Purpose:					Loads an open input file into a buffer. 
Function In Parameters:		FILE * const fi, Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns -2 if unable to fit into buffer 
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_load (FILE * const fi, Buffer * const pBD)
{
	char temp = 0; /*Used to store the character to input into the buffer*/
	if(pBD == NULL || fi == NULL)
		return R_FAIL_1;

	while(!feof(fi))
	{
		temp = (char)fgetc(fi); /*Stores the read character to be checked if EOF or valid to add to buffer*/ 
		if(temp == EOF)
			break;
		else if(temp!=EOF)
		{
			if(pBD->addc_offset < SHRT_MAX)
				b_addc(pBD,temp); /*Sends the buffer pointer and symbol to the addc function to be added to the buffer*/
		}
		else
			return (pBD->addc_offset-1);
	}
	return (pBD->addc_offset);
}

/***************************************************************************
Name:						b_isempty
Purpose:					Checks the addc_offset of the current buffer
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns 1 if offset is 0
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_isempty(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;

	if(pBD->addc_offset == 0)
		return 1;
	else
		return 0;
}

/***************************************************************************
Name:						b_eob
Purpose:					Function returns eob to the calling function
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns eob on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_eob(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->eob;
}

/***************************************************************************
Name:						b_getc
Purpose:					Function returns
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns character located at getc_offset
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
char b_getc(Buffer * const pBD)
{
	int x;
	if(pBD == NULL)
	{
		return R_FAIL_2;
	}
	else if(pBD->getc_offset == pBD->addc_offset)
	{
		pBD->eob = 1;
		return R_FAIL_1;
	}
	pBD->eob = 0;
	x = pBD->getc_offset;
	pBD->getc_offset++;
	return pBD->ca_head[x];
}
/***************************************************************************
Name:						b_print
Purpose:					Prints the current contents of the character buffer. Checks if buffer is empty
							before printing, and then sets getc_offset if it is. Otherwise it prints the contents
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_print (Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	b_set_getc_offset(pBD, 0);
	if(b_isempty(pBD) == 1) /*If the buffer is empty state so*/
		printf("The buffer is empty.\n");
	else /*Cycles through the buffer and prints each character*/
	{
		if(pBD->eob == 1)
			printf("%c", b_getc(pBD));
	
			while(pBD->getc_offset < pBD->addc_offset)
			{
				if(b_eob(pBD) == 1)
				{
					return pBD->getc_offset;
				}
				else
					printf("%c", b_getc(pBD));
			}
		printf("\n");
	}
	return pBD->getc_offset;
}

/***************************************************************************
Name:						b_pack
Purpose:					Adjusts the capacity and updates appropriate members of the buffer structure
							and shrinks the buffer.
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns NULL on failure
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
Buffer *b_pack(Buffer * const pBD)
{
	if(pBD == NULL)	
		return NULL;
	if(pBD->addc_offset < 0 || pBD->addc_offset == SHRT_MAX) /*Makes sure we are not going to overflow*/
		return pBD;

	pBD->capacity = ((pBD->addc_offset+1)*sizeof(char));
	realloc(pBD->ca_head, pBD->capacity);

	if(pBD->ca_head == NULL)
		return NULL;
	else
	{	
		pBD->r_flag = SET_R_FLAG;
		return pBD;
	}
}


/***************************************************************************
Name:						b_get_r_flag
Purpose:					Returns the buffers r_flag to the calling function
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns r_flag on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
char b_get_r_flag(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->r_flag;
}

/***************************************************************************
Name:						b_retract
Purpose:					Returns getc_offset to the calling function
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns ????? on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_retract(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else if(pBD->getc_offset > 0)
	{
		pBD->getc_offset = pBD->getc_offset - 1;
		return 0;
	}
	else
		return R_FAIL_1;
}

/***************************************************************************
Name:						b_get_getc_offset
Purpose:					Returns the getc_offset to the calling function
Function In Parameters:		Buffer * const pBD
Function Out Parameters:	Returns -1 on failure, returns getc_offset on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
short b_get_getc_offset(Buffer * const pBD)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
		return pBD->getc_offset;
}

/***************************************************************************
Name:						b_set_getc_offset
Purpose:					Sets the buffer getc_offset to offset
Function In Parameters:		Buffer * const pBD, short offset
Function Out Parameters:	Returns -1 on failure
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
int b_set_getc_offset(Buffer * const pBD, short offset)
{
	if(pBD == NULL)
		return R_FAIL_1;
	else
	{
		pBD->getc_offset = offset;
		return 0;
	}
}

/***************************************************************************
Name:						b_get_chemloc
Purpose:					Returns a pointer to the location of a character in the buffer, specified by offset
Function In Parameters:		Buffer * const pBD, short offset
Function Out Parameters:	Returns NULL on failure, returns pointer to character in buffer on success
Version:					1.0
Author:						Eric Dodds
***************************************************************************/
/*char * b_get_chmemloc(Buffer * const pBD, short offset)
{
		if(pBD == NULL || offset == 0)
			return NULL;

		if(pBD->ca_head[offset] != '\0') /*Makes sure there is a character at the location
			return(pBD->ca_head + offset);
		else
			return NULL;
}*/

char * b_get_chmemloc(Buffer * const pBD, short offset)
{
	if(pBD == NULL || offset >= pBD->addc_offset)
		return NULL;

	else
	{
		char * char_loc;
		char_loc = pBD->ca_head + offset;
		return char_loc;
	}
}

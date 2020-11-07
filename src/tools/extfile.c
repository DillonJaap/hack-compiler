#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "extfile.h"

static int get_num_lines(FILE* fp)
{
	int num_lines = 1;

	long pos = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	for (char c = fgetc(fp); c != EOF; c = fgetc(fp))
	{
		if (c == '\n')
			num_lines++;
	}
	fseek(fp, pos, SEEK_SET);
	return num_lines;
}
 
static void get_col_lengths(FILE* fp, int col_lens[])
{
	long pos = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	col_lens[0] = 0;
	int i = 0;
	for (char c = fgetc(fp); c != EOF; c = fgetc(fp))
	{
		col_lens[i]++;
		if (c == '\n')
			col_lens[++i] = 0;
	}
	fseek(fp, pos, SEEK_SET);
}

XFILE init_xfile(FILE* fp, char* name)
{
	// copy filename
	char* xname = malloc(strlen(name) + 1);
	strcpy(xname, name);

	// find num lengths
	int* col_lens = malloc(sizeof(int) * get_num_lines(fp));
	get_col_lengths(fp, col_lens);
	return (XFILE){fp, xname, 1, 1, col_lens};
}

// keep track of line number and col number
char xfgetc(XFILE* xf)
{
	char c = fgetc(xf->fp);
	xf->col++;

	if (xf->col > xf->col_lens[xf->line - 1])
	{
		xf->line++;
		xf->col = 1;
	}
	return c;
}

// like fgetc but exits if it encounters EOF
char xfgetc_EOF(XFILE* xf)
{
	char c = xfgetc(xf);
	if (c == EOF)
	{
		fprintf(stderr, "Error, unexpected EOF\n");
		exit(-1);
	}
	return c;
}

// go back one char
void back_char(XFILE* xf)
{
	fseek(xf->fp, -1, SEEK_CUR);

	xf->col--;
	if (xf->col < 1)
	{
		xf->line--;
		xf->col = xf->col_lens[xf->line-1];
	}
}

// get the nth character ahead of this one
char next_char(XFILE* xf)
{
	char c = fgetc(xf->fp);
	fseek(xf->fp, -1, SEEK_CUR);
	return c;
}

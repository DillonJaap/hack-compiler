#ifndef EXTFILE_H
#define EXTFILE_H

#include <stdio.h>
#include <stdbool.h>
// struct that stores metadata for a file
typedef struct XFILE
{
	FILE* fp;
	char* name;
	int line;
	int col;
	int* col_lens;
} XFILE;

static int get_num_lines(FILE* fp);
static void get_col_lengths(FILE* fp, int col_lens[]);
XFILE init_xfile(FILE* fp, char* name);

char xfgetc(XFILE* xf);
char xfgetc_EOF(XFILE* xf);
void back_char(XFILE* xf);
char next_char(XFILE* xf);
#endif

#include <stdio.h>
#include <string.h>
#include "tokenizer.h"
#include "parser.h"
#include "symbol-table.h"

#ifdef _WIN32
#define SEP_CHAR '\\'
#else
#define SEP_CHAR '/'
#endif

#define XML_DIR "./xml/"

void get_name(char* dest, char* src)
{
	char* mark = src;
	char* prev = src;

	for(; *src != '\0'; src++)              // get to end of string
	{
		if (*prev == SEP_CHAR)
			mark = src;
		prev = src;
	}
	while(*mark != '\0' && *mark != '.')              // get to end of string
		*dest++ = *mark++;
	*dest = '\0';
}

int main(int argc, char* argv[])
{
	char fname[128];
	char token_name[256];
	char tree_name[256];

	get_name(fname, argv[1]);

	sprintf(token_name, "%s%sT.xml", XML_DIR, fname);
	sprintf(tree_name,  "%s%s.xml",  XML_DIR, fname);

	FILE* in = fopen(argv[1], "r");
	FILE* token_file = fopen(token_name, "w");	
	FILE* tree_file  = fopen(tree_name,  "w");	

	TokenQueue* queue = tokenize(in, token_file);
//	print_token_xml(token_file, queue);

	ParseNode* root = compile(queue);
	print_tree(tree_file, root);

	fclose(in);
	fclose(token_file);
	fclose(tree_file);
	/*
	SymbolTable* st = init_st();
	st_insert(st, "AX", ST_INT, SK_LOCAL);
	st_insert(st, "AY", ST_INT, SK_LOCAL);
	st_insert(st, "AZ", ST_INT, SK_LOCAL);
	st_insert(st, "B1", ST_CHAR, SK_ARG);
	st_insert(st, "C1", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "C2", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "C3", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "D", ST_CLASS_NAME, SK_FIELD);

	st_print(st);
	st_clear(st);
	//st_print(st);

	st_insert(st, "AX", ST_INT, SK_LOCAL);
	st_insert(st, "AY", ST_INT, SK_LOCAL);
	st_insert(st, "AZ", ST_INT, SK_LOCAL);
	st_insert(st, "B1", ST_CHAR, SK_ARG);
	st_insert(st, "C1", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "C2", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "C3", ST_BOOLEAN, SK_STATIC);
	st_insert(st, "D", ST_CLASS_NAME, SK_FIELD);
	st_print(st);
	*/
	return 0;
}

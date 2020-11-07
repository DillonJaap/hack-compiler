#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "tokenizer.h"
#include "hashmap.h"
#include "extfile.h"
#include "parser.h"
#include "comp-types.h"


// macro to simplify inserting items into hashtable, make sure to define s
#define symbol_insert(key, value)\
	strcpy(s, value);\
	ht_insert(symbols, key, (void*)s)

HashTable* get_symbols()
{
	HashTable* symbols = init_ht(STRING);
	char s[8];
	symbol_insert("(", "(");
	symbol_insert(")", ")");
	symbol_insert("[", "[");
	symbol_insert("]", "]");
	symbol_insert("{", "{");
	symbol_insert("}", "}");
	symbol_insert(".", ".");
	symbol_insert(",", ",");
	symbol_insert(";", ";");
	symbol_insert("+", "+");
	symbol_insert("-", "-");
	symbol_insert("*", "*");
	symbol_insert("/", "/");
	symbol_insert("|", "|");
	symbol_insert("&", "&amp;");
	symbol_insert("<", "&lt;");
	symbol_insert(">", "&gt;");
	symbol_insert("\"", "&quot;");
	symbol_insert("=", "=");
	symbol_insert("~", "~");
	return symbols;
}

#define keyword_insert(value)\
	strcpy(s, value);\
	ht_insert(keywords, value, (void*)s)

HashTable* get_keywords()
{
	char s[24];
	HashTable* keywords = init_ht(STRING);
	keyword_insert("class");
	keyword_insert("constructor");
	keyword_insert("function");
	keyword_insert("method");
	keyword_insert("static");
	keyword_insert("field");
	keyword_insert("var");
	keyword_insert("int");
	keyword_insert("char");
	keyword_insert("boolean");
	keyword_insert("void");
	keyword_insert("null");
	keyword_insert("true");
	keyword_insert("false");
	keyword_insert("let");
	keyword_insert("do");
	keyword_insert("if");
	keyword_insert("else");
	keyword_insert("while");
	keyword_insert("return");
	keyword_insert("this");
	return keywords;
}


// puts a copy of the given token into a list
bool token_enqueue(TokenQueue* queue, Token token)
{
	if (queue->tail != NULL)
	{
		queue->tail->next = malloc(sizeof(TokenNode));
		queue->tail = queue->tail->next; 
	}
	else
	{
		queue->tail = malloc(sizeof(TokenNode));
		queue->head = queue->current = queue->tail;
	}

	queue->tail->next = NULL;

	// memcopy
	queue->tail->token = token;

	return true;
}

// removes a copy of the given token from the list, and puts the copy into a
// token
bool token_dequeue(TokenQueue* queue, Token* token)
{
	if (queue->head == NULL)
		return false;

	*(token) = queue->current->token;
	queue->current = queue->current->next; 
	return true;
}

void token_copy(Token** dest, Token* src)
{
	if (*dest == NULL)
		*dest = malloc(sizeof(TokenNode));
	**dest = *src;
}

// skip a comment, if there is no comment to skip, return false, if skipped
// sucessfully, return true
bool skip_comment(XFILE* xf)
{
	char c = xfgetc(xf);
	if (c == '/')
	{
		if (next_char(xf) == '*')
		{
			while(xfgetc_EOF(xf) != '*' || next_char(xf) != '/');
			xfgetc_EOF(xf);
			return true;
		}
		else if (next_char(xf) == '/')
		{
			while(xfgetc_EOF(xf) != '\n');
			return true;
		}
	}
	back_char(xf);
	return false;
}

// set the file pos to the next char that is part of a token 
// (not a whitespace or comment), return false if no such char exists
bool seek_next_token(XFILE* xf)
{
	char c;
	for (;;)
	{
		if (skip_comment(xf))
			continue;
		else if (c == EOF)
			return false;
		else if (isspace(c = xfgetc(xf)))
		{
			while (isspace(c = xfgetc(xf)));
			back_char(xf);
			if (c == EOF)
				return false;
		}
		else
		{
			back_char(xf);
			return true;
		}
	}
}

bool correct_terminator(char c)
{
	if (isspace(c)
		       	|| c == '}' || c == ')' || c == ']' || c == ',' 
			|| c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '&' || c == '|' || c == '<' || c == '>'
			|| c == ';')
	{
		return true;
	}
	fprintf(stderr, "Error, Improper identifier\n");
	return false;
}


bool is_symbol(char c)
{
	static HashTable* symbols = NULL;
	if (symbols == NULL)
		symbols = get_symbols();

	char s[2];
	s[0] = c;
	s[1] = '\0';
	return ht_exists(symbols, s);
}

bool is_keyword(char* s)
{
	static HashTable* keywords = NULL;
	if (keywords == NULL)
		keywords = get_keywords();
	return ht_exists(keywords, s);
}

char* grab_symbol(XFILE* xf)
{
	static HashTable* symbols = NULL;
	if (symbols == NULL)
		symbols = get_symbols();

	char* value = malloc(sizeof(char) * 256);
	char sym[2];
	sym[0] = xfgetc(xf);
	sym[1] = '\0';
	ht_find(symbols, sym, (void*)value);
	return value;
}

char* grab_int_const(XFILE* xf)
{
	char* value = malloc(sizeof(char) * 256);

	int i = 0;
	for (; isdigit(value[i] = xfgetc(xf)); i++);
	back_char(xf); // make sure the next token starts on the correct character
	correct_terminator(value[i]);
	value[i] = '\0';

	return value;
}

char* grab_string_const(XFILE* xf)
{
	char* value = malloc(sizeof(char) * 1028);

	int i = 0;
	xfgetc(xf);
	for (; (value[i] = xfgetc(xf)) != '"'; i++);
	value[i] = '\0';
	return value;
}

char* grab_id_and_kw(XFILE* xf)
{
	char* value = malloc(sizeof(char) * 1028);

	int i = 0;
	for (; isalnum(value[i] = xfgetc(xf)) || value[i] == '_'; i++);
	value[i] = '\0';
	back_char(xf);

	return value;
}

void get_token_string(char* dest, LexType type)
{
	switch (type)
	{
		case KEYWORD:          strcpy(dest, "keyword");         break;
		case SYMBOL:           strcpy(dest, "symbol");          break;
		case INTEGER_CONSTANT: strcpy(dest, "integerConstant"); break;
		case STRING_CONSTANT:  strcpy(dest, "stringConstant");  break;
		case IDENTIFIER:       strcpy(dest, "identifier");      break;
	}
}

// returns false if there is an error
Token get_token(XFILE* xf)
{
	if (!seek_next_token(xf))
		return (Token){SYMBOL, NULL, -1};

	LexType type;
	char* value;
	int col_beg = 0;
	int col_end = 0;

	char nextc = next_char(xf);
	col_beg = xf->col;

	if (nextc == '"')   // check if string constant
	{
		value = grab_string_const(xf);
		type  = STRING_CONSTANT;
	}
	else if (isdigit(nextc)) // check if integer constant
	{
		value = grab_int_const(xf);
		type  = INTEGER_CONSTANT;
	}
	else if (is_symbol(nextc))
	{
		value = grab_symbol(xf);
		type  = SYMBOL;
	}
	else                     // either keyword or identifier
	{
		value = grab_id_and_kw(xf);
		if (is_keyword(value))
			type = KEYWORD;
		else
			type = IDENTIFIER;
	}

	col_end = xf->col;
	return (Token){type, value, xf->line, col_beg, col_end};
}

void print_token(FILE* f, Token token)
{
		char ts[32];
		get_token_string(ts, token.type);
		fprintf(f, "<%s> %s </%s>\n", ts, token.value, ts);
		printf("<%s> %s </%s>\n", ts, token.value, ts);
}

void print_token_xml(FILE* f, TokenQueue* queue)
{
	TokenNode* current = queue->current; // save position
	Token token;

	fprintf(f, "<tokens>\n");
	printf("<tokens>\n");
	while (queue->current->next != NULL)
	{
		token_dequeue(queue, &token);
		print_token(f, token);
	}
	fprintf(f, "</tokens>\n");
	printf("</tokens>\n");

	queue->current = current; // restore queue positiion
}

TokenQueue* tokenize(FILE* in, FILE* out)
{
	Token token;
	TokenQueue* queue = malloc(sizeof(TokenQueue));
	XFILE xf = init_xfile(in, "test");

	// build token queue
	do {
		token = get_token(&xf);
		token_enqueue(queue, token);
	} while(token.value != NULL);

	return queue;
}

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hashmap.h"
#include "extfile.h"
#include "comp-types.h"

typedef struct Token
{
	LexType type;
	char* value;	
	int line_num;
	int col_beg;
	int col_end;
} Token;

// queue for tokens
typedef struct TokenNode
{
	Token token;
	struct TokenNode* next;
} TokenNode;

typedef struct
{
	TokenNode* head;
	TokenNode* current;
	TokenNode* tail;
} TokenQueue;

HashTable* get_symbols();
HashTable* get_keywords();

bool token_enqueue(TokenQueue* queue, Token token);
bool token_dequeue(TokenQueue* queue, Token* token);
void token_copy(Token** dest, Token* src);

bool skip_comment(XFILE* xf);
bool seek_next_token(XFILE* xf);
bool correct_terminator(char c);

bool is_symbol(char c);
bool is_keyword(char* s);

char* grab_symbol(XFILE* xf);
char* grab_int_const(XFILE* xf);
char* grab_string_const(XFILE* xf);
char* grab_id_and_kw(XFILE* xf);

void get_token_string(char* dest, LexType type);
Token get_token(XFILE* xf);
void print_token(FILE* f, Token token);
void print_token_xml(FILE* f, TokenQueue* queue);
TokenQueue* tokenize(FILE* in, FILE* out);

#endif

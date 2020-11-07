#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define ST_LENGTH 64 
#define ST_KEY_LEN 64 

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Symbol 
{
	char* type;
	char* kind;
	char* name;
	int num;
	struct Symbol* next;
} Symbol;

typedef struct SymbolTable
{
	int arg_num;
	int local_num;
	int static_num;
	int field_num;
	Symbol** table;
} SymbolTable;

uint32_t st_hash(char* s);
SymbolTable* init_st();
Symbol* init_symbol(SymbolTable* st, char* name, char* type, char* kind);
bool st_insert(SymbolTable* st, char* name, char* type, char* kind);
bool st_exists(SymbolTable* st, char* name);
bool st_find(SymbolTable* st, Symbol** symbol, char* name);
bool symbol_free(Symbol* symbol);
bool st_free_row(Symbol* symbol);
bool st_clear(SymbolTable* st);
bool st_print(SymbolTable* st);

#endif

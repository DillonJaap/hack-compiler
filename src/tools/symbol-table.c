#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "symbol-table.h"

uint32_t st_hash(char* s)
{
	int hash = 7;
	for (; *s != '\0'; s++)
		hash = hash * 31 + *s;
	return hash % ST_LENGTH;
}

SymbolTable* init_st()
{
	SymbolTable* st = malloc(sizeof(SymbolTable));
	st->table       = calloc(ST_LENGTH, sizeof(Symbol*));

	st->arg_num    = 0;
	st->local_num  = 0;
	st->static_num = 0;
	st->field_num  = 0;

	for (int i = 0; i < ST_LENGTH; i++)
		st->table[i] = NULL;

	return st;
}

Symbol* init_symbol(SymbolTable* st, char* name, char* type, char* kind)
{
	// init symbol
	Symbol* symbol;
	symbol = malloc(sizeof(Symbol));

	symbol->name = malloc(strlen(name) + 1); 
	symbol->type = malloc(strlen(type) + 1); 
	symbol->kind = malloc(strlen(kind) + 1); 
	strcpy(symbol->name, name);
	strcpy(symbol->type, type);
	strcpy(symbol->kind, kind);
	symbol->next = NULL;

	return symbol;
}

bool st_insert(SymbolTable* st, char* name, char* type, char* kind)
{
	uint32_t index = st_hash(name);

	// symbol is first in current row
	if (st->table[index] == NULL)
	{
		st->table[index] = init_symbol(st, name, type, kind);
		return true;
	}
		
	// find where to insert symbol
	Symbol* symbol = st->table[index]; 
	for (;;)
	{
		// symbol already exists return false
		if (strcmp(symbol->name, name) == 0)
			return false;
		// end of list, create new symbol
		else if (symbol->next == NULL)
		{
			symbol->next = init_symbol(st, name, type, kind);
			return true;
		}
		symbol = symbol->next;
	}
}


bool st_exists(SymbolTable* st, char* name)
{
	uint32_t index = st_hash(name);
	Symbol* symbol = st->table[index];

	// first symbol in current row
	if (symbol == NULL)
		return false;

	while (strcmp(symbol->name, name) != 0)
	{
		if (symbol->next == NULL)
			return false;
		symbol = symbol->next;
	}
	return true;
}

bool st_find(SymbolTable* st, Symbol** symbol, char* name)
{
	uint32_t index = st_hash(name);
	*symbol = st->table[index];

	// first symbol in current row
	if (*symbol == NULL)
		return false;

	while (strcmp((*symbol)->name, name) != 0)
	{
		if ((*symbol)->next == NULL)
			return false;
		 *symbol = (*symbol)->next;
	}
	return true;
}

bool symbol_free(Symbol* symbol)
{
	free(symbol->name);
	free(symbol->type);
	free(symbol->kind);
	free(symbol);
	return true;
}

bool st_free_row(Symbol* symbol)
{
	if (symbol == NULL)
		return false;
	else if (symbol->next != NULL)
		st_free_row(symbol->next);
	symbol_free(symbol);
	return true;
}

// make the table empty
bool st_clear(SymbolTable* st)
{
	st->local_num = 0;
	st->arg_num   = 0;
	for (int i = 0; i < ST_LENGTH; i++)
	{
		st_free_row(st->table[i]);
		st->table[i] = NULL;
	}
	return true;
}

bool st_print(SymbolTable* st)
{
	printf("+-------------------------------------------+\n");
	printf("| %8s | %8s | %8s | %8s |\n", "name", "type", "kind", "#");
	printf("+----------+----------+----------+----------+\n");
	Symbol* symbol = NULL;
	for (int i = 0; i < ST_LENGTH; i++)
	{
		symbol = st->table[i];
		for (; symbol != NULL; symbol = symbol->next)
		{
			printf("| %8s | %8s | %8s | %8d |\n", symbol->name,
				   	symbol->type, symbol->kind, symbol->num);
		}
	}
	printf("+-------------------------------------------+\n");
	return true;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hashmap.h"

uint32_t hash_string(char* s)
{
	int hash = 7;
	for (; *s != '\0'; s++)
		hash = hash * 31 + *s;
	return hash;
}

void ht_copy_data(Data* dest, void* src, DataType type)
{
	// copy data
	switch (type)
	{
		case INT: 
			dest->i = *(int*)src;
			break;
		case FLOAT:
			dest->f = *(double*)src;
			break;
		case STRING:
			strcpy(dest->str, (char*)src);
		       	break;
		default:
			printf("error: specifiy data type to be copied!\n");
			break;
	}
}

struct Cell* init_cell(char key[], void* data, DataType type)
{
	// init cell
	struct Cell* cell;
	cell = malloc(sizeof(struct Cell));
	cell->next = NULL;

	//copy key
	strcpy(cell->key, key);

	// copy data
	ht_copy_data(&cell->data, data, type);
	
	return cell;
}

HashTable* init_ht(DataType type)
{
	HashTable* ht = malloc(sizeof(ht) * sizeof(struct Cell*) * LENGTH);

	ht->type = type;
	for (int i = 0; i < LENGTH; i++)
		ht->table[i] = NULL;

	return ht;
}

void ht_insert(HashTable* ht, char* key, void* data)
{
	uint32_t index = hash_string(key) % LENGTH;
	struct Cell* cell;

	// cell is first in current row
	if (ht->table[index] == NULL)
	{
		ht->table[index] = init_cell(key, data, ht->type);
		return;
	}
		
	// find where to insert cell
	cell = ht->table[index]; 
	for (;;)
	{
		// cell already exists update data
		if (strcmp(cell->key, key) == 0)
		{
			ht_copy_data(&cell->data, data, ht->type);
			return;
		}
		// end of list, create new cell
		else if (cell->next == NULL)
		{
			cell->next = init_cell(key, data, ht->type);
			return;
		}

		cell = cell->next;
	}
}

int ht_delete(HashTable* ht, char* key)
{
	uint32_t index = hash_string(key) % LENGTH;
	struct Cell temp_cell; 
	struct Cell* cell = &temp_cell; 

	cell->next = ht->table[index];

	for (;;)
	{
		if (cell->next == NULL)
			return 0;
		else if (strcmp(cell->next->key, key) == 0)
		{
			struct Cell* tmp = cell->next;
			cell->next = cell->next->next;
			free(tmp);
			return 1;
		}
		cell = cell->next;
	}
}

bool ht_exists(HashTable* ht, char* key)
{
	uint32_t index = hash_string(key) % LENGTH;
	struct Cell* cell = ht->table[index];

	// first cell in current row
	if (cell == NULL)
		return false;

	while (strcmp(cell->key, key) != 0)
	{
		if (cell->next == NULL)
			return false;
		cell = cell->next;
	}
	return true;
}

bool ht_find(HashTable* ht, char* key, void* data)
{
	uint32_t index = hash_string(key) % LENGTH;
	struct Cell* cell = ht->table[index];

	// first cell in current row
	if (cell == NULL)
		return false;

	while (strcmp(cell->key, key) != 0)
	{
		if (cell->next == NULL)
			return false;
		 cell = cell->next;
	}

	switch (ht->type)
	{
		case INT: 
			*(int*)data = cell->data.i;
			break;
		case FLOAT:
			*(double*)data = cell->data.f;
			break;
		case STRING:
			strcpy((char*)data, cell->data.str);
		       	break;
		default:
			printf("error: specifiy data type to be copied!\n");
			break;
	}
	return true;
}

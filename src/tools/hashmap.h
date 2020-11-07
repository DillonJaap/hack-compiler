#ifndef HASHMAP_H
#define HASHMAP_H
#define LENGTH 528
#define DATA_LEN 128

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// data types
typedef enum DataType
{
	NONE   = 0x0,
	INT    = 0x1,
	FLOAT  = 0x2,
	STRING = 0x4
} DataType;

typedef union Data
{
	int32_t i;
	double  f;
	char    str[DATA_LEN];
} Data;

struct Cell 
{
	struct Cell* next;
	union Data data;
	char key[DATA_LEN];
};

typedef struct HashTable
{
	DataType type;	
	struct Cell* table[LENGTH];
} HashTable;

uint32_t hash_string(char* s);
void ht_copy_data(Data* dest, void* src, DataType type);

struct Cell* init_cell(char key[], void* data, DataType type);
HashTable*   init_ht(DataType type);

void ht_insert(HashTable* ht, char* key, void* data);
int  ht_delete(HashTable* ht, char* key);
bool ht_exists(HashTable* ht, char* key);
bool ht_find(HashTable* ht, char* key, void* data);

#endif

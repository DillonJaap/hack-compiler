#include <stdlib.h>
#include <stdio.h>
#include "dynamicarray.h"
#include "parser.h"

darray* init_darray(size_t init_size)
{
	//darray* da = malloc(sizeof(darray));
	darray* da = malloc(sizeof(darray));
	ParseNode** array = calloc(init_size, sizeof(ParseNode*));
	*da = (darray){.array = array, .used = 0, .size = init_size};
	return da;
}

void darray_insert(darray* da, ParseNode* node)
{
	if (da->used >= da->size)
	{
		size_t size = (size_t)(da->size * 1.5 + 1);
		da = reallocarray((void*)da, size, sizeof(ParseNode*));
	}
	da->array[(da->used)] = node;
	printf("used: %ld, value: %x\n", da->used, node);
	(da->used)++;
}

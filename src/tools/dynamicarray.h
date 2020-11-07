#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H
#include <stdlib.h>
#include "parser.h"

struct ParseNode;

typedef struct darray
{
	struct ParseNode** array;
	size_t used;
	size_t size;
} darray;

darray* init_darray(size_t init_size);
void darray_insert(darray* da, struct ParseNode* node);

#endif

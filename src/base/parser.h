#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "tokenizer.h"
#include "comp-types.h"
#include "symbol-table.h"

typedef struct ParseNode
{
	LexType type;	
	char* value; // only leaf nodes have a value
	struct ParseNode* child;
	struct ParseNode* last_child;
	struct ParseNode* next_sibling;
} ParseNode;

ParseNode* init_ParseNode(LexType type);
static ParseNode* init_child_node(ParseNode* parent, LexType type, char* value);
static void delete_child_node(ParseNode* parent);

void get_parse_string(char* dest, LexType type);
void print_indent(FILE* f, int lvl);
static void print_tree_inner(FILE* f, ParseNode* node, int indent_lvl);
void print_tree(FILE* f, ParseNode* node);

static bool look_ahead(TokenQueue* queue, int amount, int arg_count, ...);

static bool compile_empty(ParseNode* parent);
static bool compile_element(TokenQueue* queue, ParseNode* parent, LexType type);
static bool compile_className(TokenQueue* queue, ParseNode* node);
static bool compile_subroutineName(TokenQueue* queue, ParseNode* node);
static bool compile_varName(TokenQueue* queue, ParseNode* node);

static bool compile_matching_element(TokenQueue* queue, ParseNode* parent, 
		 LexType type, int arg_count,  va_list args);
static bool compile_keyword(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...);
static bool compile_symbol(TokenQueue* queue, ParseNode* node, 
		int arg_count, ...);
static bool compile_identifier(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...);
static bool compile_int_const(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...);
static bool compile_str_const(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...);

ParseNode* compile(TokenQueue* queue);
static bool compile_class(TokenQueue* queue, ParseNode* root);
static bool compile_classVarDec(TokenQueue* queue, ParseNode* parent, 
		SymbolTable* class_table);
static bool compile_type(TokenQueue* queue, ParseNode* parent);
static bool compile_subroutineDec(TokenQueue* queue, ParseNode* parent,
		SymbolTable* class_table);
static bool compile_parameterList(TokenQueue* queue, ParseNode* parent);
static bool compile_subroutineBody(TokenQueue* queue, ParseNode* parent);
static bool compile_varDec(TokenQueue* queue, ParseNode* parent);
static bool compile_statements(TokenQueue* queue, ParseNode* parent);
static bool compile_statement(TokenQueue* queue, ParseNode* parent);
static bool compile_letStatement(TokenQueue* queue, ParseNode* parent);
static bool compile_ifStatement(TokenQueue* queue, ParseNode* parent);
static bool compile_whileStatement(TokenQueue* queue, ParseNode* parent);
static bool compile_doStatement(TokenQueue* queue, ParseNode* parent);
static bool compile_returnStatement(TokenQueue* queue, ParseNode* parent);
static bool compile_expression(TokenQueue* queue, ParseNode* parent);
static bool compile_term(TokenQueue* queue, ParseNode* parent);
static bool compile_subroutineCall(TokenQueue* queue, ParseNode* parent);
static bool compile_expressionList(TokenQueue* queue, ParseNode* parent);
static bool compile_op(TokenQueue* queue, ParseNode* parent);
static bool compile_unaryOp(TokenQueue* queue, ParseNode* parent);
static bool compile_keywordConstant(TokenQueue* queue, ParseNode* parent);

#endif

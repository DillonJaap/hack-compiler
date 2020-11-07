#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "parser.h"
#include "tokenizer.h"
#include "dynamicarray.h"
#include "comp-types.h"

// find the number of tokens between the current one and the one matching the
// string s

ParseNode* init_ParseNode(LexType type)
{
	ParseNode* node = malloc(sizeof(ParseNode));
	*node = (ParseNode){.type = type, .value = NULL, .child = NULL, 
		.next_sibling = NULL};
	return node;
}

static ParseNode* init_child_node(ParseNode* parent, LexType type)
{
	if (parent->child == NULL)
	{
		parent->child = init_ParseNode(type);
		return parent->child;
	}

	ParseNode* child = parent->child;
	for (; child->next_sibling != NULL; child = child->next_sibling);
	child->next_sibling = init_ParseNode(type);
	return child->next_sibling;
}

static void delete_child_node(ParseNode* parent)
{
	if (parent->child == NULL)
		return;

	if (parent->child->next_sibling == NULL)
	{
		free(parent->child);
		parent->child = NULL;
		return;
	}

	ParseNode* child =  parent->child;
	for (; child->next_sibling->next_sibling != NULL; 
			child = child->next_sibling);
	free(child->next_sibling);
	child->next_sibling = NULL;
}

void get_parse_string(char* dest, LexType type)
{
	switch (type)
	{
		case KEYWORD:           strcpy(dest, "keyword");         break;
		case SYMBOL:            strcpy(dest, "symbol");          break;
		case IDENTIFIER:        strcpy(dest, "identifier");      break;
		case INTEGER_CONSTANT:  strcpy(dest, "integerConstant"); break;
		case STRING_CONSTANT:   strcpy(dest, "stringConstant");  break;
		case KEYWORD_CONSTANT:  strcpy(dest, "keywordConstant"); break;
		case CLASS:             strcpy(dest, "class");           break;
		case CLASS_VAR_DEC:     strcpy(dest, "classVarDec");     break;
		case TYPE:              strcpy(dest, "type");            break;
		case SUBROUTINE_DEC:    strcpy(dest, "subroutineDec");   break;
		case PARAMETER_LIST:    strcpy(dest, "parameterList");   break;
		case SUBROUTINE_BODY:   strcpy(dest, "subroutineBody");  break;
		case VAR_DEC:           strcpy(dest, "varDec");          break;
		case STATEMENTS:        strcpy(dest, "statements");      break;
		case STATEMENT:         strcpy(dest, "statement");       break;
		case LET_STATEMENT:     strcpy(dest, "letStatement");    break;
		case IF_STATEMENT:      strcpy(dest, "ifStatement");     break;
		case WHILE_STATEMENT:   strcpy(dest, "whileStatement");  break;
		case DO_STATEMENT:      strcpy(dest, "doStatement");     break;
		case RETURN_STATEMENT:  strcpy(dest, "returnStatement"); break;
		case EXPRESSION:        strcpy(dest, "expression");      break;
		case TERM:              strcpy(dest, "term");            break;
		case SUBROUTINE_CALL:   strcpy(dest, "subroutineCall");  break;
		case EXPRESSION_LIST:   strcpy(dest, "expressionList");  break;
		case OP:                strcpy(dest, "op");              break;
		case UNARY_OP:          strcpy(dest, "unaryOp");         break;
		default: strcpy(dest, ""); break;
	}
}

void print_indent(FILE* f, int lvl)
{
	for (int i = 0; i < lvl; i++)
	{
		printf("  ");
		fprintf(f, "  ");
	}
}

static void print_tree_inner(FILE* f, ParseNode* node, int indent_lvl)
{
	char pstring[32];
	get_parse_string(pstring, node->type);

	if (node->type == EMPTY) // empty list/set
		return;
	if (node->child == NULL) // leaf node
	{
		print_indent(f, indent_lvl);
		printf("<%s> %s </%s>\n", pstring, node->value, pstring);
		fprintf(f, "<%s> %s </%s>\n", pstring, node->value, pstring);
	}
	else
	{
		print_indent(f, indent_lvl);
		printf("<%s>\n", pstring);
		fprintf(f, "<%s>\n", pstring);

		print_tree_inner(f, node->child, indent_lvl+1);

		print_indent(f, indent_lvl);
		printf("</%s>\n", pstring);
		fprintf(f, "</%s>\n", pstring);
	}

	if (node->next_sibling != NULL)
		print_tree_inner(f, node->next_sibling, indent_lvl);
}

void print_tree(FILE* f, ParseNode* node)
{
	print_tree_inner(f, node, 0);
}


// true if token matches s
static bool look_ahead(TokenQueue* queue, int amount, int arg_count, ...)
{
	TokenNode* current = queue->current; // save position

	Token token;
	for (int i = 0; i <= amount; i++)
		token_dequeue(queue, &token);
	bool ret = false;

	va_list valist;
	va_start(valist, arg_count);

	for (int i = 0; i < arg_count; i++)
	{
		if (strcmp(token.value, va_arg(valist, char*)) == 0) 
			ret = true;
	}

	va_end(valist);
	queue->current = current; // restore queue positiion
	return ret;
}

//******************************************************************************
// Compile Terminal elements
//******************************************************************************
static bool compile_empty(ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, EMPTY);
	return true;
}

static bool compile_element(TokenQueue* queue, ParseNode* parent, LexType type)
{

	TokenNode* current = queue->current;
	ParseNode* child = init_child_node(parent, type);

	Token token;
	token_dequeue(queue, &token);


	if (token.type == type)
	{
		child->value = malloc(strlen(token.value) + 1);
		strcpy(child->value, token.value);
		return true;
	}

	// restore queue, did not sucessfully grab lexical element. Also delete
	// created child node
	queue->current = current; 
	delete_child_node(parent);
	return false;
}

static bool compile_className(TokenQueue* queue, ParseNode* node)
{
	return compile_element(queue, node, IDENTIFIER);
}

static bool compile_subroutineName(TokenQueue* queue, ParseNode* node)
{
	return compile_element(queue, node, IDENTIFIER);
}

static bool compile_varName(TokenQueue* queue, ParseNode* node)
{
	return compile_element(queue, node, IDENTIFIER);
}

// Terminal element
// true if token matches any of the variable arguments
static bool compile_matching_element(TokenQueue* queue, ParseNode* parent,
	   	LexType type, int arg_count,  va_list args)
{
	TokenNode* current = queue->current;
	ParseNode* child = init_child_node(parent, type);

	Token token;
	token_dequeue(queue, &token);

	if (token.type != type) 
	{
		queue->current = current; 
		delete_child_node(parent);
		char pstring[32];
		char pstring2[32];
		get_parse_string(pstring, token.type);
		get_parse_string(pstring2, type);
		return false;
	}


	for (int i = 0; i < arg_count; i++)
	{
		if (strcmp(token.value, va_arg(args, char*)) == 0)                 
		{
			child->value = malloc(strlen(token.value) + 1);
			strcpy(child->value, token.value);
			return true;
		}
	}

	// restore queue, did not sucessfully grab lexical element. Also delete
	// created child node
	queue->current = current; 
	delete_child_node(parent);
	return false;
}

static bool compile_keyword(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...)
{
	if (arg_count == 0)
		return compile_element(queue, node, KEYWORD);

	va_list args;
	va_start(args, arg_count);
	bool ret = compile_matching_element(queue, node, KEYWORD, arg_count, args);
	va_end(args);
	return ret;
}

static bool compile_symbol(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...)
{
	if (arg_count == 0)
		return compile_element(queue, node, SYMBOL);

	va_list args;
	va_start(args, arg_count);
	bool ret = compile_matching_element(queue, node, SYMBOL, arg_count, args);
	va_end(args);
	return ret;
}

static bool compile_identifier(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...)
{
	if (arg_count == 0)
		return compile_element(queue, node, IDENTIFIER);

	va_list args;
	va_start(args, arg_count);
	bool ret = compile_matching_element(queue, node, IDENTIFIER, arg_count,
		   	args);
	va_end(args);
	return ret;
}

static bool compile_int_const(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...)
{
	if (arg_count == 0)
		return compile_element(queue, node, INTEGER_CONSTANT);

	va_list args;
	va_start(args, arg_count);
	bool ret = compile_matching_element(queue, node, INTEGER_CONSTANT, 
			arg_count, args);
	va_end(args);
	return ret;
}

static bool compile_str_const(TokenQueue* queue, ParseNode* node,
	   	int arg_count, ...)
{
	if (arg_count == 0)
		return compile_element(queue, node, STRING_CONSTANT);
	
	va_list args;
	va_start(args, arg_count);
	bool ret = compile_matching_element(queue, node, STRING_CONSTANT,
		   	arg_count, args);
	va_end(args);
	return ret;
}

//******************************************************************************
// Compile Program Structure
//******************************************************************************
ParseNode* compile(TokenQueue* queue)
{
	ParseNode* root = init_ParseNode(CLASS);
	compile_class(queue, root);
	return root;
}

// first called function will be the root node
static bool compile_class(TokenQueue* queue, ParseNode* root)
{
	root->type = CLASS;
	if (!compile_keyword(queue, root, 1, "class")) // not a class
		return false; 

	compile_className(queue, root);
	compile_symbol(queue, root, 1, "{");
	while(compile_classVarDec(queue, root));
	while(compile_subroutineDec(queue, root));
	compile_symbol(queue, root, 1, "}");

	return true;
}

static bool compile_classVarDec(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, CLASS_VAR_DEC);

	// not a classVarDec
	if (!compile_keyword(queue, child, 2, "static", "field")) 
	{
		delete_child_node(parent);
		return false; 
	}

	compile_type(queue, child);
	compile_varName(queue, child);

	while (compile_symbol(queue, child, 1, ","))
		compile_varName(queue, child);

	compile_symbol(queue, child, 1, ";");
	return true;
}

static bool compile_type(TokenQueue* queue, ParseNode* parent)
{

	return (compile_keyword(queue, parent, 3, "int", "char", "boolean") 
			|| compile_className(queue, parent));
}

static bool compile_subroutineDec(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, SUBROUTINE_DEC);

	if (!compile_keyword(queue, child, 3, "constructor", "function", "method"))
	{
		delete_child_node(parent);
		return false;
	}

	if (!compile_type(queue, child)) 
		compile_keyword(queue, child, 1, "void");

	compile_subroutineName(queue, child);
	compile_symbol(queue, child, 1, "(");
	compile_parameterList(queue, child);
	compile_symbol(queue, child, 1, ")");
	compile_subroutineBody(queue, child);

	return true;
}

static bool compile_parameterList(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, PARAMETER_LIST);

	if (!compile_type(queue, child)) 
	{
		compile_empty(child);
		return true; // parameter list can be empty
	}

	compile_varName(queue, child);

	while (compile_symbol(queue, child, 1, ","))
	{
		compile_type(queue, child);
		compile_varName(queue, child);
	}
	return true;
}

static bool compile_subroutineBody(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, SUBROUTINE_BODY);

	if (!compile_symbol(queue, child, 1, "{")) 
	{
		delete_child_node(parent);
		return false;
	}

	while(compile_varDec(queue, child));
	compile_statements(queue, child);
	compile_symbol(queue, child, 1, "}");
	return true;
}

static bool compile_varDec(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, VAR_DEC);

	if (!compile_keyword(queue, child, 1, "var")) 
	{
		delete_child_node(parent);
		return false;
	}

	compile_type(queue, child);
	compile_varName(queue, child);

	while (compile_symbol(queue, child, 1, ","))
		compile_varName(queue, child);
	compile_symbol(queue, child, 1, ";");

	return true;
}

//******************************************************************************
// Compile Statements
//******************************************************************************
static bool compile_statements(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, STATEMENTS);

	/*
	if (!compile_statement(queue, child)) 
	{
		delete_child_node(parent);
		return false;
	}
	*/
	// statements can be empty
	while(compile_statement(queue, child));
	return true;
}

static bool compile_statement(TokenQueue* queue, ParseNode* parent)
{
	return (compile_letStatement(queue, parent)
			|| compile_ifStatement(queue, parent)
			|| compile_whileStatement(queue, parent)
			|| compile_doStatement(queue, parent)
			|| compile_returnStatement(queue, parent));
}

static bool compile_letStatement(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, LET_STATEMENT);

	if (!compile_keyword(queue, child, 1, "let")) 
	{
		delete_child_node(parent);
		return false;
	}
	compile_varName(queue, child);

	if (compile_symbol(queue, child, 1, "["))
	{
		compile_expression(queue, child);
		compile_symbol(queue, child, 1, "]");
	}
	compile_symbol(queue, child, 1, "=");
	compile_expression(queue, child);
	compile_symbol(queue, child, 1, ";");
	return true;
}

static bool compile_ifStatement(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, IF_STATEMENT);

	if (!compile_keyword(queue, child, 1, "if")) 
	{
		delete_child_node(parent);
		return false;
	}

	compile_symbol(queue, child, 1, "(");
	compile_expression(queue, child);
	compile_symbol(queue, child, 1, ")");

	compile_symbol(queue, child, 1, "{");
	compile_statements(queue, child);
	compile_symbol(queue, child, 1, "}");

	if (compile_keyword(queue, child, 1, "else"))
	{
		compile_symbol(queue, child, 1, "{");
		compile_statements(queue, child);
		compile_symbol(queue, child, 1, "}");
	}
	return true;
}

static bool compile_whileStatement(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, WHILE_STATEMENT);

	if (!compile_keyword(queue, child, 1, "while")) 
	{
		delete_child_node(parent);
		return false;
	}

	compile_symbol(queue, child, 1, "(");
	compile_expression(queue, child);
	compile_symbol(queue, child, 1, ")");

	compile_symbol(queue, child, 1, "{");
	compile_statements(queue, child);
	compile_symbol(queue, child, 1, "}");
	return true;
}

static bool compile_doStatement(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, DO_STATEMENT);

	if (!compile_keyword(queue, child, 1, "do")) 
	{
		delete_child_node(parent);
		return false;
	}

	compile_subroutineCall(queue, child);
	compile_symbol(queue, child, 1, ";");
	return true;
}

static bool compile_returnStatement(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, RETURN_STATEMENT);

	if (!compile_keyword(queue, child, 1, "return")) 
	{
		delete_child_node(parent);
		return false;
	}

	compile_expression(queue, child);
	compile_symbol(queue, child, 1, ";");
	return true;
}

//******************************************************************************
// Compile Expressions
//******************************************************************************

static bool compile_expression(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, EXPRESSION);

	if (!compile_term(queue, child))
	{
		delete_child_node(parent);
		return false;
	}

	while (compile_op(queue, child)) 
		compile_term(queue, child);
	return true;
}

// compile_term is LL2
static bool compile_term(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, TERM);
	bool ret = true;

	if (compile_int_const(queue, child, 0)
			|| compile_str_const(queue, child, 0)
			|| compile_keywordConstant(queue, child))
	{
		return true;
	}
	else if (compile_unaryOp(queue, child))
	{
		compile_term(queue, child);
		return true;
	}
	else if (compile_subroutineCall(queue, child)) // LL2 in subroutine call
	{
		return true;
	}
	else if (compile_varName(queue, child))
	{
		if (compile_symbol(queue, child, 1, "["))
		{
			compile_expression(queue, child);
			compile_symbol(queue, child, 1, "]");
		}
		return true;
	}
	else if (compile_symbol(queue, child, 1, "("))
	{
		compile_expression(queue, child);
		compile_symbol(queue, child, 1, ")");
		return true;
	}

	delete_child_node(parent);
	return false;
}

// LL2
// Does not have its own tag for some reason... idk why, weird decision
static bool compile_subroutineCall(TokenQueue* queue, ParseNode* parent)
{
	if (look_ahead(queue, 1, 1, "."))
	{
		if (!compile_className(queue, parent) && !compile_varName(queue, parent))
			return false;

		compile_symbol(queue, parent, 1, ".");
		compile_subroutineName(queue, parent);
		compile_symbol(queue, parent, 1, "(");
		compile_expressionList(queue, parent);
		compile_symbol(queue, parent, 1, ")");
		return true;
	}
	if (look_ahead(queue, 1, 1, "("))
	{
		if (!compile_subroutineName(queue, parent))
		{
			return false;
		}
		compile_symbol(queue, parent, 1, "(");
		compile_expressionList(queue, parent);
		compile_symbol(queue, parent, 1, ")");
		return true;
	}

	return false;
}

static bool compile_expressionList(TokenQueue* queue, ParseNode* parent)
{
	ParseNode* child = init_child_node(parent, EXPRESSION_LIST);

	if (!compile_expression(queue, child))
	{
		compile_empty(child);
		return true;
	}

	while (compile_symbol(queue, child, 1, ","))
		compile_expression(queue, child);
	return true;
}

static bool compile_op(TokenQueue* queue, ParseNode* parent)
{
	return (compile_symbol(queue, parent, 9, "+", "-", "*", "/", "&amp;", "|", "&lt;", 
				"&gt;", "="));
}

static bool compile_unaryOp(TokenQueue* queue, ParseNode* parent)
{
	return (compile_symbol(queue, parent, 2, "-", "~"));
}

static bool compile_keywordConstant(TokenQueue* queue, ParseNode* parent)
{
	return (compile_keyword(queue, parent, 4, "true", "false", "null", "this"));
}

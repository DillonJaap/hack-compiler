#ifndef COMPTYPES_H
#define COMPTYPES_H

typedef enum LexType
{
	// Terminal lexical elements
	EMPTY = 0, // terminator for empty set/list
	KEYWORD = 1,
	SYMBOL = 2,
	IDENTIFIER = 3,
	INTEGER_CONSTANT = 4,
	STRING_CONSTANT = 5,
	// only contains a keyword a child
	KEYWORD_CONSTANT = 6,

	// Non-terminal
	// Program structure
	CLASS = 7,
	CLASS_VAR_DEC = 8,
	TYPE = 9,
	SUBROUTINE_DEC = 10,
	PARAMETER_LIST = 11,
	SUBROUTINE_BODY = 12,
	VAR_DEC = 13,

	// statements
	STATEMENTS = 14,
	STATEMENT = 15,
	LET_STATEMENT = 16,
	IF_STATEMENT = 17,
	WHILE_STATEMENT = 18,
	DO_STATEMENT = 19,
	RETURN_STATEMENT = 20,

	// expressions
	EXPRESSION = 21,
	TERM = 22,
	SUBROUTINE_CALL = 23,
	EXPRESSION_LIST = 24,
	OP = 25, // same as symbol
	UNARY_OP = 26,
} LexType;

#endif

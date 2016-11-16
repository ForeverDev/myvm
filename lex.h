#ifndef LEX_H
#define LEX_H

#include <stdio.h>
#include <stdint.h>

typedef struct Token Token;
typedef struct TokenList TokenList;
typedef enum TokenType TokenType;
typedef struct LexState LexState;

struct LexState {
	TokenList* tokens;
	FILE* handle;
	char* contents;
};

enum TokenType {
	TOK_NOTYPE = 0,
	TOK_NUMBER,
	TOK_IDENTIFIER,
	TOK_OPERATOR
};

struct Token {
	unsigned int line;
	TokenType type;
	union {
		char* sval;
		char oval;
		uint8_t nval;
	};
};

struct TokenList {
	Token* value;
	TokenList* head;
	TokenList* next;
};

TokenList* lex_file(const char*);

#endif

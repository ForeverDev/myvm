#ifndef LEX_H
#define LEX_H

#include <stdio.h>

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
	TOK_IDENTIFIER
};

struct Token {
	char* str;
	unsigned int line;
	TokenType type;
};

struct TokenList {
	Token* value;
	TokenList* head;
	TokenList* next;
};

TokenList* lex_file(const char*);

#endif

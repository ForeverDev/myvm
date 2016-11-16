#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenList* new_list();
static LexState* lex_init(const char*);
static void lex_generate_tokens();
static void append_token(LexState*, const char*, TokenType);

static TokenList*
new_list() {
	TokenList* list = malloc(sizeof(TokenList));
	list->value = NULL;
	list->head = list;
	list->next = NULL;
	return list;
}

static void
append_token(LexState* lexer, const char* str, TokenType type) {
	Token* new = malloc(sizeof(Token));
	new->str = malloc(strlen(str) + 1);
	strcpy(new->str, str);

	/* now append to the list */
	if (!lexer->tokens->head->value) {
		/* no need to make a new list if it is empty */
		lexer->tokens->value = new;
	} else {
		TokenList* newlist = malloc(sizeof(TokenList));
		newlist->value = new;
		newlist->head = lexer->tokens;
		newlist->next = NULL;
		TokenList* tail = lexer->tokens;
		while (tail->next) {
			tail = tail->next;
		}
		tail->next = newlist;
	}
}

static LexState*
lex_init(const char* filename) {
	LexState* lexer = malloc(sizeof(LexState));
	lexer->handle = fopen(filename, "rb");
	if (!lexer->handle) {
		free(lexer);
		return NULL;
	}
	unsigned long long flen;
	fseek(lexer->handle, 0, SEEK_END);
	flen = ftell(lexer->handle);
	fseek(lexer->handle, 0, SEEK_SET);
	lexer->contents = malloc(flen + 1);
	fread(lexer->contents, flen, 1, lexer->handle);
	lexer->contents[flen] = 0;
	lexer->tokens = new_list();
	return lexer;
}

static void
lex_generate_tokens(LexState* lexer) {
	/* this is where tokens are generated... */
	char* scan = lexer->contents;
	char* buf;
	char* at;
	size_t length;
	while (*scan) {
		at = scan;
		if (*at == ' ') {
			scan++;
			continue;
		} else if (isdigit(*at)) {
			length = 0;
			while (isdigit(*at)) {
				length++;
				at++;
			}
			buf = malloc(length + 1);
			strncpy(buf, scan, length);
			scan += length;
			append_token(lexer, buf, TOK_IDENTIFIER); 
			free(buf);
			buf = NULL;
		} else {
			scan++;
		}
	}

	/* print the tokens for debugging */
	for (TokenList* i = lexer->tokens; i; i = i->next) {
		printf("%s\n", i->value->str);
	}	
}

TokenList*
lex_file(const char* filename) {
	
	LexState* lexer = lex_init(filename);

	lex_generate_tokens(lexer);

	TokenList* save = lexer->tokens;
	free(lexer);
	return save;	
}

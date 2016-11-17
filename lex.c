#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenList* new_list();
static LexState* lex_init(const char*);
static void lex_generate_tokens();
static void append_token(LexState*, Token*);
static void print_token_list(TokenList*);

static void
print_token_list(TokenList* list) {
	if (!list->value) {
		return;
	}
	for (TokenList* i = list; i; i = i->next) {
		switch (i->value->type) {
			case TOK_NUMBER:
				printf("(TYPE: NUMBER,     VALUE: %d)\n", i->value->nval);
				break;
			case TOK_IDENTIFIER:
				printf("(TYPE: IDENTIFIER, VALUE: '%s')\n", i->value->sval);
				break;
			case TOK_OPERATOR:
				printf("(TYPE: OPERATOR,   VALUE: '%c')\n", i->value->oval);
				break;
		}
	}
}

static TokenList*
new_list() {
	TokenList* list = malloc(sizeof(TokenList));
	list->value = NULL;
	list->head = list;
	list->next = NULL;
	return list;
}

static void
append_token(LexState* lexer, Token* token) {
	if (!lexer->tokens->head->value) {
		/* no need to make a new list if it is empty */
		lexer->tokens->value = token;
	} else {
		TokenList* newlist = malloc(sizeof(TokenList));
		newlist->value = token;
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
	unsigned int line = 0;
	char* buf; /* temporary buffer that reads a string */
	char* at;  /* reader */
	size_t length; /* length of buf */
	Token* append;
	while (*scan) {
		at = scan;
		if (isspace(*at)) {
			scan++;
			continue;
		} else if (*at == '\n') {
			line++;
			scan++;
			continue;
		}

		if (ispunct(*at)) {
			append = malloc(sizeof(Token));
			append->line = line;
			append->type = TOK_OPERATOR;
			append->oval = *at;
			append_token(lexer, append);
			scan++;
		} else if (isdigit(*at)) {
			int base = (*at == '0' && at[1] == 'x') ? 16 : 10;
			/* if hex, skip over '0x' */
			if (base == 16) {
				at += 2;
				scan += 2; /* scan also must skip '0x' */
				if (!isdigit(*at) && !(*at >= 'A' && *at <= 'F')) {
					printf("expected number to follow '0x'\n");
					exit(1);
				}
			}
			length = 0;
			while (isdigit(*at) || (base == 16 && *at >= 'A' && *at <= 'F')) {
				length++;
				at++;
			}
			buf = malloc(length + 1);
			strncpy(buf, scan, length);
			buf[length] = 0;
			scan += length;
			append = malloc(sizeof(Token));
			append->line = line;
			append->type = TOK_NUMBER;
			append->nval = (uint8_t)strtol(buf, NULL, base);
			append_token(lexer, append);
			free(buf);
			buf = NULL;
		} else if (isalpha(*at) || *at == '_') {
			length = 0;
			while (isalnum(*at) || *at == '_') {
				length++;
				at++;
			}
			append = malloc(sizeof(Token));
			append->line = line;
			append->type = TOK_IDENTIFIER;
			append->sval = malloc(length + 1);
			strncpy(append->sval, scan, length);
			append->sval[length] = 0;
			append_token(lexer, append);
			scan += length;
		} else {
			scan++;
		}
	}
	
	print_token_list(lexer->tokens);
}

TokenList*
lex_file(const char* filename) {
	
	LexState* lexer = lex_init(filename);
	if (!lexer) {
		printf("couldn't open file '%s'", filename);
		exit(1);
	}

	lex_generate_tokens(lexer);

	TokenList* save = lexer->tokens;
	free(lexer);
	return save;	
}

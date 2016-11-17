#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenList* new_list();
static LexState* lex_init(const char*);
static void lex_destroy(LexState*);
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
	list->prev = NULL;
	return list;
}

static void
append_token(LexState* L, Token* token) {
	if (!L->tokens->head->value) {
		/* no need to make a new list if it is empty */
		L->tokens->value = token;
	} else {
		TokenList* newlist = malloc(sizeof(TokenList));
		newlist->value = token;
		newlist->head = L->tokens;
		newlist->next = NULL;
		TokenList* tail = L->tokens;
		while (tail->next) {
			tail = tail->next;
		}
		tail->next = newlist;
		newlist->prev = tail;
	}
}

static LexState*
lex_init(const char* filename) {
	LexState* L = malloc(sizeof(LexState));
	L->handle = fopen(filename, "rb");
	if (!L->handle) {
		free(L);
		return NULL;
	}
	unsigned long long flen;
	fseek(L->handle, 0, SEEK_END);
	flen = ftell(L->handle);
	fseek(L->handle, 0, SEEK_SET);
	L->contents = malloc(flen + 1);
	fread(L->contents, flen, 1, L->handle);
	L->contents[flen] = 0;
	L->tokens = new_list();
	return L;
}

static void
lex_destroy(LexState* L) {
	/* note L->tokens remain untouched, they are returned by lex_file */
	fclose(L->handle);
	free(L->contents);
	free(L);
}

static void
lex_generate_tokens(LexState* L) {
	/* this is where tokens are generated... */
	char* scan = L->contents;
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
			append_token(L, append);
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
			append_token(L, append);
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
			append_token(L, append);
			scan += length;
		} else {
			scan++;
		}
	}
	
	print_token_list(L->tokens);
}

TokenList*
lex_file(const char* filename) {
	
	LexState* L = lex_init(filename);
	if (!L) {
		printf("couldn't open file '%s'", filename);
		exit(1);
	}

	lex_generate_tokens(L);

	TokenList* save = L->tokens;
	lex_destroy(L);

	return save;	
}

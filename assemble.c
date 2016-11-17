#include <stdio.h>
#include <stdlib.h>
#include "assemble.h"

static AssembleState* assemble_init(TokenList*, const char*);

static AssembleState*
assemble_init(TokenList* tokens, const char* outfile) {
	AssembleState* A = malloc(sizeof(AssembleState));
	A->tokens = tokens;
	A->handle = fopen(outfile, "wb");
	if (!A->handle) {
		printf("couldn't open '%s' for writing\n", outfile);
		exit(1);
	}
	return A;
}

void 
assemble_file(const char* infile, const char* outfile) {
	AssembleState* A = assemble_init(lex_file(infile), outfile);
}

#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include "lex.h"

typedef struct AssembleState AssembleState;

struct AssembleState {
	TokenList* tokens;
	FILE* handle; /* output */
};

void assemble_file(const char*, const char*);

#endif

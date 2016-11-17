#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assemble.h"

/* a few notes on modes,
 *	1. for operands (REG, [REG]), use (REG, [REG + 0])
 */
#define MODE_0 (0x1 << 0) /* () */
#define MODE_1 (0x1 << 1) /* IMM */
#define MODE_2 (0x1 << 2) /* REG */
#define MODE_3 (0x1 << 3) /* (REG, IMM) */
#define MODE_4 (0x1 << 4) /* (REG, REG) */
#define MODE_5 (0x1 << 5) /* ([REG + IMM], IMM) */
#define MODE_6 (0x1 << 6) /* ([REG + IMM], REG) */
#define MODE_7 (0x1 << 7) /* REG, [REG + IMM] */

typedef struct Instruction {
	const char* identifier;
	uint8_t opcode;
	uint16_t valid_modes;
} Instruction;

static const Instruction instructions[] = {
	{"MOV", 0x01, MODE_1 | MODE_2 | MODE_3 | MODE_4 | MODE_5},
	{"ADD", 0x02, MODE_1 | MODE_2 | MODE_3 | MODE_4 | MODE_5}
};

static const char* registers[] = {
	[0x00] = "IP",
	[0x01] = "SP",
	[0x02] = "BP",
	[0x03] = "A",
	[0x04] = "B",
	[0x05] = "C"
};

static const size_t num_instructions = 2;
static const size_t num_registers = 6;

/* function declarations */
static AssembleState* assemble_init(TokenList*, const char*);
static void assemble_destroy(AssembleState*);
static const Instruction* get_instruction(const char*);
static uint8_t get_register(const char*);
static void determine_operands(AssembleState*, Token**, uint8_t*);

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

static void
assemble_destroy(AssembleState* A) {
	fclose(A->handle);
	/* now cleanup the tokens handed to us by the lexer */
}

static const Instruction*
get_instruction(const char* identifier) {
	for (int i = 0; i < num_instructions; i++) {
		if (!strcmp(instructions[i].identifier, identifier)) {
			return &instructions[i];
		}	
	}
	return NULL;
}

static uint8_t
get_register(const char* identifier) {
	for (int i = 0; i < num_registers; i++) {
		if (!strcmp(registers[i], identifier)) {
			return i;
		}
	}
	return 0xFF; /* err code */
}

static void
determine_operands(AssembleState* A, Token* operands[], uint8_t* mode) {
	/* expects to be on the token after instruction */
	if (A->at->value->type == TOK_IDENTIFIER) {
		if (!A->at || get_instruction(A->at->value->sval)) {
			/* mode 0 */
			*mode = 0;
		} else {
			/* modes 2, 3, 4, 7 */
			operands[0] = A->at->value;
			A->at = A->at->next; /* on comma */
			if (A->at->value->type == TOK_OPERATOR && A->at->value->oval == ',') {
				/* mode 3, 4, 7 */
				A->at = A->at->next; /* skip ',' */
				if (A->at->value->type == TOK_NUMBER) {
					/* mode 3 */
					*mode = 3;
					operands[1] = A->at->value;
				} else if (A->at->value->type == TOK_IDENTIFIER) {
					/* mode 4 */
					*mode = 4;
					operands[1] = A->at->value;
				} else {
					/* mode 7 */
					*mode = 7;
					A->at = A->at->next; /* skip '[' */
					operands[1] = A->at->value;
					A->at = A->at->next; /* skip '+' or '-' */
					A->at = A->at->next;
					operands[2] = A->at->value;
					A->at = A->at->next; /* end on ']' */
				}
			} else {
				A->at = A->at->prev;
				/* mode 2 */
				*mode = 2;
			}
		}
	} else if (A->at->value->type == TOK_OPERATOR) {
		/* modes 5, 6 */
		/* todo assert token '[' */
		A->at = A->at->next;
		operands[0] = A->at->value; /* REG */
		A->at = A->at->next;
		/* todo read sign (+) or (-) */
		A->at = A->at->next;
		operands[1] = A->at->value; /* IMM */
		A->at = A->at->next;
		/* todo assert token ']' */
		A->at = A->at->next;
		/* todo assert token ',' */
		A->at = A->at->next;
		/* now on IMM or REG */
		operands[2] = A->at->value;
		*mode = (A->at->value->type == TOK_NUMBER) ? 5 : 6;
	} else if (A->at->value->type == TOK_NUMBER) {
		/* mode 1 */
		*mode = 1;
		operands[0] = A->at->value;
	}
}

void 
assemble_file(const char* infile, const char* outfile) {
	AssembleState* A = assemble_init(lex_file(infile), outfile);
	
	for (A->at = A->tokens; A->at; A->at = A->at->next) {
		if (A->at->value->type != TOK_IDENTIFIER) {
			/* todo error here - expected identifier */
			break;
		}
		const Instruction* ins = get_instruction(A->at->value->sval);
		if (!ins) {
			/* todo error here - unknown instruction */
			break;
		}
		A->at = A->at->next;
		/* now on the first operand */
		Token* operands[3] = {NULL}; /* array of operands */
		uint8_t mode;
		determine_operands(A, operands, &mode);
		
		/* write the opcode */
		fputc(ins->opcode, A->handle);
		/* write the mode */
		fputc(mode, A->handle);
		/* write the operands */
		switch (mode) {
			case 0:
				break;
			case 1:
				fputc(operands[0]->nval, A->handle);
				break;
			case 2:
				fputc(get_register(operands[0]->sval), A->handle);
				break;
			case 3:
				fputc(get_register(operands[0]->sval), A->handle);
				fputc(operands[1]->nval, A->handle);
				break;
			case 4:
				fputc(get_register(operands[0]->sval), A->handle);
				fputc(get_register(operands[1]->sval), A->handle);
				break;
			case 5:
				fputc(get_register(operands[0]->sval), A->handle);
				fputc(operands[1]->nval, A->handle);
				fputc(operands[2]->nval, A->handle);
				break;
			case 6:
				fputc(get_register(operands[0]->sval), A->handle);
				fputc(operands[1]->nval, A->handle);
				fputc(get_register(operands[2]->sval), A->handle);
				break;
			case 7:
				fputc(get_register(operands[0]->sval), A->handle);
				fputc(get_register(operands[1]->sval), A->handle);
				fputc(operands[2]->nval, A->handle);
				break;
		}

		if (!A->at) break;

	}

	assemble_destroy(A);
}

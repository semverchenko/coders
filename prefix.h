#ifndef PREFIX_H
#define PREFIX_H
#include <string.h>
#include <stdio.h>
#include "constants.h"
#include "util.h"

typedef struct prefix_node{
	struct prefix_node *parent;
	struct prefix_node *left;
	struct prefix_node *right;
	data_t data;
} prefix_node;

typedef struct
{
	prefix_node tree[ALPHABET_SIZE * 2 - 1];
	prefix_node *index[ALPHABET_SIZE];
	path_t path[ALPHABET_SIZE];
	size_t size;
} prefix_coder;

prefix_coder *prefix_init();
void prefix_add(prefix_coder *pc, data_t data, path_t path);

void prefix_build_tree(prefix_coder *pc);
void prefix_write_tree(prefix_coder *pc, writer *output);
void prefix_read_tree(prefix_coder *pc, reader *input);

void prefix_code_char(prefix_coder *pc, data_t c, writer *output);
void prefix_code(prefix_coder *pc, FILE *input, writer *output);
data_t prefix_decode_char(prefix_coder *pc, reader *input);
void prefix_decode(reader *input, FILE *output);

void prefix_free(prefix_coder *pc);
#endif

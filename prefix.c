#include "prefix.h"
#include <stdlib.h>
#include <inttypes.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct {
	data_t data;
	path_t path;
} path_data;

prefix_coder *prefix_init()
{
	prefix_coder *pc = (prefix_coder *)calloc(1, sizeof(prefix_coder));
	return pc;
}

void prefix_free(prefix_coder *pc)
{
	free(pc);
}

void prefix_add(prefix_coder *pc, data_t data, path_t path)
{
	path |= 1; /* do not affect codes of length < 64; null path means unused char */
	pc->path[(int)data] = path;
}

int prefix_cmp_by_code(const void *first, const void *second)
{
	if (((path_data *)first)->path > ((path_data *)second)->path)
		return 1;
	else if (((path_data *)first)->path < ((path_data *)second)->path)
		return -1;
	else
		return 0;
}

void prefix_build_subtree(prefix_coder *pc, path_data *begin, path_data *end, prefix_node *node, uint8_t depth)
{
	path_data *l = begin, *r = end, *m;

	node->left = node->right = NULL;
	if (end - begin == 1) {
		node->data = begin->data;
		pc->index[(int)(begin->data)] = node;
		return;
	}

	if (begin->path == (end - 1)->path) {
		fprintf(stderr, ALGORITHM_ERROR);
		exit(1);
	}

	while (((begin->path >> (sizeof(path_t) - 1 - depth)) & 1) == 
	   (((end - 1)->path >> (sizeof(path_t) - 1 - depth)) & 1)) /* reduce tree */
		depth++;

	while (r - l > 1) {
		m = l + (r - l) / 2;

		if (((begin->path >> (sizeof(path_t) - 1 - depth)) & 1) == 
		  (((m - 1)->path >> (sizeof(path_t) - 1 - depth)) & 1))
			l = m;
		else
			r = m;
	}

	node->left = pc->tree + pc->size;
	node->right = pc->tree + pc->size + 1;
	pc->size += 2;
	node->left->parent = node;
	node->right->parent = node;

	prefix_build_subtree(pc, begin, m, node->left, depth + 1);
	prefix_build_subtree(pc, m, end, node->right, depth + 1);
}

void prefix_build_tree(prefix_coder *pc)
{
	path_data pd[ALPHABET_SIZE];
	size_t i, n = 0;

	for (i = 0; i < ALPHABET_SIZE; ++i) 
		if (pc->path[i]) {
			pd[n].data = i;
			pd[n].path = pc->path[i];
			n++;
		}

	qsort(pd, n, sizeof(path_data), prefix_cmp_by_code);
	pc->size++;
	prefix_build_subtree(pc, pd, pd + n, pc->tree, 0);
}

void pref_print_spaces(size_t count)
{
	while (count--)
		printf(" ");
}

void pref_print_tree(prefix_node *node, size_t depth)
{
	if (node->left == 0) {
		pref_print_spaces(depth);
		printf("data: %c\n", node->data);
	} else {
		pref_print_spaces(depth);
		printf("left:\n");
		pref_print_tree(node->left, depth + 1);
		pref_print_spaces(depth);
		printf("right:\n");
		pref_print_tree(node->right, depth + 1);
	}
}

typedef enum {
	PREFIX_TREE_LEFT,
	PREFIX_TREE_RIGHT,
	PREFIX_TREE_UP,
	PREFIX_TREE_DATA
} prefix_tree_cmd;

void prefix_write_cmd(prefix_tree_cmd cmd, writer *output)
{

	write_bit(output, cmd & 1);
	write_bit(output, (cmd >> 1) & 1);
}

prefix_tree_cmd prefix_read_cmd(reader *input)
{
	prefix_tree_cmd res = 0;
	
	res |= read_bit(input);
	res |= read_bit(input) << 1;

	return res;
}

void prefix_write_subtree(prefix_node *node, writer *output)
{
	if (node->left != NULL) {
		prefix_write_cmd(PREFIX_TREE_LEFT, output);
		prefix_write_subtree(node->left, output);
		prefix_write_cmd(PREFIX_TREE_UP, output);

		prefix_write_cmd(PREFIX_TREE_RIGHT, output);
		prefix_write_subtree(node->right, output);
		prefix_write_cmd(PREFIX_TREE_UP, output);
	} else {
		prefix_write_cmd(PREFIX_TREE_DATA, output);
		write_data(output, node->data);
	}
}

void prefix_write_tree(prefix_coder *pc, writer *output)
{
	prefix_write_subtree(pc->tree, output);
	prefix_write_cmd(PREFIX_TREE_UP, output);
}

void prefix_read_subtree(prefix_coder *pc, prefix_node *node, reader *input)
{
	prefix_tree_cmd cmd;

	node->left = node->right = NULL;
	do {
		cmd = prefix_read_cmd(input);
		switch(cmd) {
			case PREFIX_TREE_LEFT:
				node->left = pc->tree + pc->size;
				pc->size++;
				prefix_read_subtree(pc, node->left, input);
				break;
			case PREFIX_TREE_RIGHT:
				node->right = pc->tree + pc->size;
				pc->size++;
				prefix_read_subtree(pc, node->right, input);
				break;
			case PREFIX_TREE_DATA:
				node->data = read_data(input);
				break;
			case PREFIX_TREE_UP:
				break;
			default:
				fprintf(stderr, INVALID);
				exit(1);
		}
	} while (cmd != PREFIX_TREE_UP);
}

void prefix_read_tree(prefix_coder *pc, reader *input)
{
	pc->size = 1;
	prefix_read_subtree(pc, pc->tree, input);
}

void prefix_code_char(prefix_coder *pc, data_t c, writer *output)
{
	prefix_node *node = pc->index[(size_t)c];
	path_t path = 0;
	uint8_t len = 0;
	size_t i;

	while (node->parent != NULL) {
		path >>= 1;
		len++;
		if (node == node->parent->right)
			path |= ((path_t)1) << (8 * sizeof(path_t) - 1);
		node = node->parent;
	}

	for (i = 0; i < len; ++i)
		write_bit(output, (path >> (sizeof(path_t) * 8 - i - 1)) & 1);
}

void prefix_code(prefix_coder *pc, FILE *input, writer *output)
{
	long fptr;
	uint64_t text_size;
	int c;

	prefix_build_tree(pc);

	fptr = ftell(input);
	fseek(input, 0, SEEK_END);
	text_size = ftell(input) - fptr; /* do not work with big files on 32-bit OS */
	fseek(input, fptr, SEEK_SET);


	write_64(output, text_size);
	/* pref_print_tree(pc->tree, 0); */
	prefix_write_tree(pc, output);

	while ((c = fgetc(input)) != EOF)
		prefix_code_char(pc, (data_t)c, output);
}

data_t prefix_decode_char(prefix_coder *pc, reader *input)
{
	prefix_node *node = pc->tree;
	uint8_t bit;

	while (node->left != NULL) {
		bit = read_bit(input);

		if (bit)
			node = node->right;
		else
			node = node->left;
	}

	return node->data;
}

void prefix_decode(reader *input, FILE *output)
{
	uint64_t text_size;
	prefix_coder *pc = prefix_init();
	int c;

	text_size = read_64(input);
	prefix_read_tree(pc, input);
	/* pref_print_tree(pc->tree, 0); */

	while(text_size--) {
		c = prefix_decode_char(pc, input);
		fputc(c, output);
	}

	prefix_free(pc);
}

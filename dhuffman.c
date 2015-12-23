#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "prefix.h"
#include "dhuffman.h"
#include "util.h"
#include "constants.h"


typedef struct dhuffman_node {
	struct dhuffman_node *parent, *left, *right;
	prob_t count;
	data_t data;
} dhuffman_node_t;

typedef struct {
	uint8_t char_len;
	struct dhuffman_node tree[ALPHABET_SIZE * 2 - 1];
	struct dhuffman_node *index[ALPHABET_SIZE];
	size_t size;
} dhuffman_coder;

void node_init(dhuffman_node_t *node)
{
	node->data = 0;
	node->count = 0;
	node->left = node->right = node->parent = NULL;
}

void dh_print_spaces(size_t count)
{
	while (count--)
		printf(" ");
}

void dh_print_tree(dhuffman_coder *dc, dhuffman_node_t *node, size_t depth)
{
	dh_print_spaces(depth);
	printf("node %p\n", (void *)node);
	dh_print_spaces(depth);
	printf("count %ld\n", (long)node->count);
	if (node->left == NULL) {
		dh_print_spaces(depth);
		if (node == dc->tree + 2 * dc->size)
			printf("zero node\n");
		else
			printf("data: '%c'\n", node->data);
	} else {
		dh_print_spaces(depth);
		printf("left:\n");
		dh_print_tree(dc, node->left, depth + 1);
		dh_print_spaces(depth);
		printf("right:\n");
		dh_print_tree(dc, node->right, depth + 1);
	}
}

dhuffman_coder *dhuffman_coder_init()
{
	dhuffman_coder *res = calloc(1, sizeof(dhuffman_coder));
	node_init(res->tree);
	res->size = 0;
	return res;
}

void split_node(dhuffman_node_t *node)
{
	node->left = node + 1;
	node->right = node + 2;
	node_init(node->left);
	node_init(node->right);
	node->left->parent = node;
	node->right->parent = node;
}

dhuffman_node_t *add_data(dhuffman_coder *dc, data_t data)
{
	dhuffman_node_t *null_node;

	null_node = dc->tree + dc->size * 2;
	split_node(null_node);
	null_node->left->data = data;

	dc->index[(size_t)data] = null_node->left;

	dc->size++;
	return null_node;
}

void node_swap(dhuffman_coder *dc, dhuffman_node_t *first, dhuffman_node_t *second)
{
	dhuffman_node_t tmp, *t;

	if (first == second)
		return;

	tmp	= *first;
	*first = *second;
	*second = tmp;

	t = second->parent;
	second->parent = first->parent;
	first->parent = t;
	

	if (first->left)
		first->left->parent = first;
	if (first->right)
		first->right->parent = first;

	if (second->left)
		second->left->parent = second;
	if (second->right)
		second->right->parent = second;

	if (first->left == NULL && first != dc->tree + dc->size * 2)
		dc->index[(size_t)first->data] = first;
	if (second->left == NULL && second != dc->tree + dc->size * 2)
		dc->index[(size_t)second->data] = second;
}

void dhuffman_update(dhuffman_coder *dc, dhuffman_node_t *node)
{
	dhuffman_node_t *leftmost;
	while (node->parent != NULL) {
		leftmost = node;
		while (leftmost > dc->tree && (leftmost - 1)->count == node->count)
			leftmost--;
		if (node->parent == leftmost)
			leftmost++;
		node_swap(dc, node, leftmost);
		leftmost->count++;
		node = leftmost->parent;
	}
	node->count++;
}

int dh_code_char(dhuffman_coder *dc, data_t data, path_t *code, uint8_t *len) /* returns 1 if new symbol was added */
{
	dhuffman_node_t *res_node, *up_node;
	int res = 0;

	if (dc->index[(size_t)data] == NULL) {
		res_node = add_data(dc, data);
		up_node = res_node->left;
		res = 1;
	} else {
		up_node = res_node = dc->index[(size_t)data];
	}

	*code = 0;
	*len = 0;

	while (res_node->parent != NULL)
	{
		*code >>= 1;
		(*len)++;
		if (res_node == res_node->parent->right)
			*code |= ((path_t)1) << (sizeof(path_t) * 8 - 1);
		else if (res_node != res_node->parent->left) {
			fprintf(stderr, ALGORITHM_ERROR);
			exit(1);
		}
		res_node = res_node->parent;
	}
	dhuffman_update(dc, up_node);

	return res;
}

data_t read_char(dhuffman_coder *dc, reader *r)
{
	dhuffman_node_t *node;
	uint8_t bit;
	data_t data;

	node = dc->tree;
	while (node->left != NULL) {
		bit = read_bit(r);
		if (bit)
			node = node->right;
		else
			node = node->left;
	}
	
	if (node == dc->tree + dc->size * 2) { /* null node */
		data = read_byte(r);
		node = add_data(dc, data)->left;
	}	else {
			data = node->data;
	}

	dhuffman_update(dc, node);

	return data;
}

void dhuffman_coder_free(dhuffman_coder *dc)
{
	free(dc);
}

void dhuffman_code(FILE* fin, FILE* fout)
{
	uint8_t len;
	long fptr;
	writer *w = writer_init(fout);
	uint64_t text_size;
	dhuffman_coder *dc;
	int res;
	int c;
	size_t i;
	path_t code;

	dc = dhuffman_coder_init();

	write_byte(w, DHUFFMAN);

	fptr = ftell(fin);
	fseek(fin, 0, SEEK_END);
	text_size = ftell(fin) - fptr; /* do not work with big files on 32-bit OS */
	fseek(fin, fptr, SEEK_SET);

	write_64(w, text_size);
	while ((c = fgetc(fin)) != EOF) {
		res = dh_code_char(dc, (data_t)c, &code, &len);
		for (i = 0; i < len; ++i)
			write_bit(w, (code >> (sizeof(path_t) * 8 - i - 1)) & 1);
		if (res)
			write_byte(w, (uint8_t)c);
	}

	dhuffman_coder_free(dc);

	writer_close(w);
}

void dhuffman_decode(FILE* fin, FILE* fout)
{
	reader *r = reader_init(fin);
	uint64_t text_size;
	data_t data;
	dhuffman_coder *dc;

	dc = dhuffman_coder_init();

	text_size = read_64(r);

	while (text_size > 0)
	{
		data = read_char(dc, r);
		fputc(data, fout);
		text_size--;
	}

	dhuffman_coder_free(dc);

	reader_close(r);
}

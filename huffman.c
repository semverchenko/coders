#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "prefix.h"
#include "huffman.h"
#include "util.h"
#include "constants.h"

typedef struct huffman_node {
	data_t data;
	prob_t prob;
	struct huffman_node *parent;
	uint8_t bit;
} huffman_node_t;

prefix_coder *generate_huffman_coder(pdata_arr *prob)
{
	size_t i, n;
	huffman_node_t tree[2 * ALPHABET_SIZE - 1], *end, *mid, *l, *r, *rr, *f, *s;
	uint64_t code;
	prefix_coder *pc;

	sort_desc(prob);

	for (i = 0, n = 0; i < ALPHABET_SIZE; ++i)
		if (prob->data[i].prob > 0) {
			tree[n].data = prob->data[i].data;
			tree[n].prob = prob->data[i].prob;
			tree[n].parent = NULL;
			n++;
		}

	mid = tree + n;
	end = tree + n * 2 - 1;
	l = rr = r = mid;

	while (r != end) {
		if (rr < r && (l == tree || rr->prob < (l - 1)->prob))
			f = rr++;
		else
			f = --l;

		if (rr < r && (l == tree || rr->prob < (l - 1)->prob))
			s = rr++;
		else
			s = --l;

		f->parent = r;
		f->bit = 0;
		s->parent = r;
		s->bit = 1;
		r->prob = f->prob + s->prob;
		r->parent = NULL;
		r++;
	}

	pc = prefix_init();

	for (i = 0; i < n; ++i) {
		code = 0;

		f = tree + i;
		while (f->parent != NULL) {
			code = (code >> 1) | (((path_t)f->bit) << (sizeof(path_t) * 8 - 1));
			f = f->parent;
		}


		prefix_add(pc, prob->data[i].data, code);
	}

	return pc;
}

void huffman_code(FILE* fin, FILE* fout)
{
	pdata_arr *prob;
	long pos;
	writer *w = writer_init(fout);
	prefix_coder *pc;

	pos = ftell(fin);
	prob = calculate_prob(fin);
	fseek(fin, pos, SEEK_SET);

	pc = generate_huffman_coder(prob);

	free(prob);

	write_byte(w, HUFFMAN);

	prefix_code(pc, fin, w);

	prefix_free(pc);

	writer_close(w);
}

void huffman_decode(FILE* fin, FILE* fout)
{
	reader *r = reader_init(fin);

	prefix_decode(r, fout);

	reader_close(r);
}

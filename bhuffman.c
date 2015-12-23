#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "prefix.h"
#include "util.h"
#include "constants.h"
#include "bhuffman.h"
#include "huffman.h"

pdata_arr **calculate_prob_matrix(FILE* fin)
{
	int c, prev = EOF;
	size_t i;
	pdata_arr **res = malloc(ALPHABET_SIZE * sizeof(pdata_arr *));

	for (i = 0; i < ALPHABET_SIZE; ++i) {
		res[i] = calloc(1, sizeof(pdata_arr));
		res[i]->sum = 0;
	}
	
	while ((c = fgetc(fin)) != EOF) {
		if (prev != EOF) {
			res[prev]->data[c].data = (data_t)c;
			res[prev]->data[c].prob++;
			res[prev]->sum++;
		}
		prev = c;
	}

	return res;
}

void free_matrix(pdata_arr **matrix)
{
	size_t i;

	for (i = 0; i < ALPHABET_SIZE; ++i)
		free(matrix[i]);
	free(matrix);
}

void bhuffman_code(FILE* fin, FILE* fout)
{
	pdata_arr **prob;
	long pos;
	writer *w = writer_init(fout);
	prefix_coder *pc[ALPHABET_SIZE] = {0};
	size_t i;
	uint64_t text_size;
	int c, prev = EOF;

	pos = ftell(fin);
	prob = calculate_prob_matrix(fin);
	text_size = ftell(fin) - pos; /* do not work with big files on 32-bit OS */
	fseek(fin, pos, SEEK_SET);

	write_byte(w, BHUFFMAN);
	write_64(w, text_size);

	for (i = 0; i < ALPHABET_SIZE; ++i)
		if (prob[i]->sum > 0) {
			pc[i] = generate_huffman_coder(prob[i]);
			prefix_build_tree(pc[i]);
			write_bit(w, 1);
			prefix_write_tree(pc[i], w);
		} else {
			write_bit(w, 0);
		}
	free_matrix(prob);

	while ((c = fgetc(fin)) != EOF) {
		if (prev == EOF) {
			write_byte(w, c);
		} else {
			prefix_code_char(pc[prev], (data_t)c, w);
		}
		prev = c;
	}

	for (i = 0; i < ALPHABET_SIZE; ++i)
		if (pc[i])
			prefix_free(pc[i]);

	writer_close(w);
}

void bhuffman_decode(FILE* fin, FILE* fout)
{
	prefix_coder *pc[ALPHABET_SIZE] = {0};
	size_t i;
	uint64_t text_size;
	data_t prev;
	reader *r = reader_init(fin);

	text_size = read_64(r);
	for (i = 0; i < ALPHABET_SIZE; ++i)
		if (read_bit(r)) {
			pc[i] = prefix_init();
			prefix_read_tree(pc[i], r);
		}

	if (text_size == 0)
		goto exit;

	prev = read_byte(r);
	fputc(prev, fout);

	while (--text_size)
		fputc(prev = prefix_decode_char(pc[prev], r), fout);

exit:
	for (i = 0; i < ALPHABET_SIZE; ++i)
		if (pc[i])
			prefix_free(pc[i]);

	reader_close(r);
}

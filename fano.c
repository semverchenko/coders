#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "prefix.h"
#include "fano.h"
#include "util.h"
#include "constants.h"

uint64_t absdiff64(uint64_t left, uint64_t right)
{
	if (left >= right)
		return left - right;
	else
		return right - left;
}

void fano_generate_codes(pdata *begin, pdata *end, prefix_coder *pc, path_t code, uint8_t len)
{
	//fprintf(stderr, "%c %" PRIx64 " %" PRIu8 "\n", (char)(begin->data >> 56), code, len);
	pdata *left = begin, *right = end - 1;
	prob_t left_sum = 0, right_sum = 0;

	if (end - begin == 1) {
		prefix_add(pc, begin->data, code);
		return;
	}

	while (right - left > 1) {
		if (absdiff64(left_sum + left->prob, right_sum) < absdiff64(left_sum, right_sum + right->prob)) {
			left_sum += left->prob;
			left++;
		} else {
			right_sum += right->prob;
			right--;
		}
	}
	fano_generate_codes(begin, right, pc, code, len + 1);
	fano_generate_codes(right, end, pc, code | (((path_t)1) << (sizeof(path_t) * 8 - len - 1)), len + 1);
}

void fano_code(FILE* fin, FILE* fout)
{
	pdata_arr *prob;
	long pos;
	writer *w = writer_init(fout);
	prefix_coder *pc = prefix_init();

	pos = ftell(fin);
	prob = calculate_prob(fin);
	fseek(fin, pos, SEEK_SET);

	sort_desc(prob);
	fano_generate_codes(prob->data, prob->data + ALPHABET_SIZE, pc, 0, 0);

	write_byte(w, FANO);

	free(prob);

	prefix_code(pc, fin, w);

	prefix_free(pc);

	writer_close(w);
}

void fano_decode(FILE* fin, FILE* fout)
{
	reader *r = reader_init(fin);

	prefix_decode(r, fout);

	reader_close(r);
}

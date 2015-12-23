#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "prefix.h"
#include "shannon.h"
#include "util.h"
#include "constants.h"

void shannon_code(FILE* fin, FILE* fout)
{
	prob_t cum;
	pdata_arr *prob;
	long pos;
	size_t i;
	writer *w = writer_init(fout);
	prefix_coder *pc = prefix_init();

	pos = ftell(fin);
	prob = calculate_prob(fin);
	pdata_normalize(prob);
	fseek(fin, pos, SEEK_SET);

	sort_desc(prob);
	cum = 0;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
		if (i > 0)
			cum += prob->data[i - 1].prob;
		if (prob->data[i].prob == 0)
			continue;
		prefix_add(pc, prob->data[i].data, cum << ((sizeof(path_t) - sizeof(prob_t)) * 8));
	}

	write_byte(w, SHANNON);

	free(prob);

	prefix_code(pc, fin, w);

	prefix_free(pc);

	writer_close(w);
}

void shannon_decode(FILE* fin, FILE* fout)
{
	reader *r = reader_init(fin);

	prefix_decode(r, fout);

	reader_close(r);
}

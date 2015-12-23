#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "util.h"

typedef struct {
	prob_t low, range;
	uint64_t delayed_bits;
	prob_t cum[ALPHABET_SIZE];
	writer *w;
} ar_coder;

ar_coder *ar_coder_init(pdata_arr *prob)
{
	size_t i;
	prob_t cum = 0;
	ar_coder *res = malloc(sizeof(ar_coder));

	res->low = 0;
	res->range = HALF;
	res->delayed_bits = 0;

	if (prob != NULL)
		for (i = 0; i < ALPHABET_SIZE; ++i) {
			cum += prob->data[i].prob;
			res->cum[i] = cum;
		}

	return res;
}

void ar_coder_close(ar_coder *ar)
{
	if (ar->w != NULL)
		writer_close(ar->w);
	free(ar);
}

void resize_window_write(ar_coder *ac, writer *w)
{

	while (ac->range <= QUARTER) {
		if (ac->range == 0) {
			fprintf(stderr, ALGORITHM_ERROR);
			exit(1);
		}
		if (ac->low < HALF && ac->low + ac->range < HALF) {
				write_bit(w, 0);
				while (ac->delayed_bits--)
					write_bit(w, 1);
				ac->delayed_bits = 0;
		} else if (ac->low >= HALF) {
				write_bit(w, 1);
				while (ac->delayed_bits--)
					write_bit(w, 0);
				ac->delayed_bits = 0;
			ac->low -= HALF;
		} else {
			ac->low -= QUARTER;
			ac->delayed_bits++;
		}
		ac->low *= 2;
		ac->range *= 2;
	}
}

void resize_window_read(ar_coder *ac, reader *r)
{
	while (ac->range <= QUARTER) {
		if (ac->range == 0) {
			fprintf(stderr, ALGORITHM_ERROR);
			exit(1);
		}
		ac->low *= 2;
		ac->range *= 2;
		ac->low |= read_bit_or_zero(r);
	}
}

void ac_code_char(ar_coder *ac, writer *w, data_t data)
{
	prob_t cum, prev_cum, step;

	cum = ac->cum[(size_t)data];
	prev_cum = data ? ac->cum[(size_t)data - 1] : 0;

	step = ac->range / ac->cum[ALPHABET_SIZE - 1];
	ac->low += prev_cum * step;
	if (cum == ac->cum[ALPHABET_SIZE - 1])
		ac->range -= prev_cum * step;
	else
		ac->range = (cum - prev_cum) * step;

	resize_window_write(ac, w);
}

void arithmetic_code(FILE* fin, FILE* fout)
{
	long pos;
	pdata_arr *prob;
	uint64_t file_size;
	ar_coder* ac;
	writer *w = writer_init(fout);
	size_t i;
	int c;

	pos = ftell(fin);
	prob = calculate_prob(fin);
	file_size = ftell(fin) - pos;
	fseek(fin, pos, SEEK_SET);

	write_byte(w, ARITHMETIC);
	write_64(w, file_size);
	ac = ar_coder_init(prob);
	free(prob);
	for (i = 0; i < ALPHABET_SIZE; ++i)
		write_prob(w, ac->cum[i]);


	while ((c = fgetc(fin)) != EOF)
		ac_code_char(ac, w, (data_t)c);

	ac->range = 1;
	resize_window_write(ac, w);

	/* for (i = 0; i < sizeof(prob_t) * 8; i++)
		write_bit(w, (ac->low >> (sizeof(prob_t) * 8 - 1 - i)) & 1); */
	ar_coder_close(ac);
	writer_close(w);
}

data_t ac_decode_char(ar_coder *ac, reader *input)
{
	prob_t *l = ac->cum - 1, *r = ac->cum + ALPHABET_SIZE - 1, *m;
	data_t res;
	prob_t cum, prev_cum, step, val;

	step = ac->range / ac->cum[ALPHABET_SIZE - 1];
	val = ac->low / step;

	while (r - l > 1) {
		m = l + (r - l) / 2;
		if (*m <= val)
			l = m;
		else
			r = m;
	}

	res = l - ac->cum + 1;


	cum = ac->cum[res];
	prev_cum = res ? ac->cum[res - 1] : 0;

	ac->low -= prev_cum * step;
	if (cum == ac->cum[ALPHABET_SIZE - 1])
		ac->range -= prev_cum * step;
	else
		ac->range = (cum - prev_cum) * step;

	resize_window_read(ac, input);

	return res;
}

void arithmetic_decode(FILE* fin, FILE* fout)
{
	uint64_t text_size, i;
	reader *r = reader_init(fin);
	ar_coder* ac;

	text_size = read_64(r);

	ac = ar_coder_init(NULL);

	for (i = 0; i < ALPHABET_SIZE; ++i)
		ac->cum[i] = read_prob(r);

	for (i = 0; i < sizeof(prob_t) * 8; ++i)
		ac->low = ac->low << 1 | read_bit(r);

	while (text_size--)
		fputc(ac_decode_char(ac, r), fout);

	ar_coder_close(ac);
	reader_close(r);
}

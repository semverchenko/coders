#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "constants.h"

writer *writer_init(FILE* fout)
{
	writer *res = malloc(sizeof(writer));
	res->stream = fout;
	res->len = 0;
	res->bits = 0;
	return res;
}

void write_byte(writer *w, uint8_t byte)
{
	fputc(w->bits | (byte << w->len), w->stream);
	w->bits = byte >> (8 - w->len);
}

void write_64(writer *w, uint64_t val)
{
	write_byte(w, (uint8_t)(val      ));
	write_byte(w, (uint8_t)(val >> 8 ));
	write_byte(w, (uint8_t)(val >> 16));
	write_byte(w, (uint8_t)(val >> 24));
	write_byte(w, (uint8_t)(val >> 32));
	write_byte(w, (uint8_t)(val >> 40));
	write_byte(w, (uint8_t)(val >> 48));
	write_byte(w, (uint8_t)(val >> 56));
}

void write_prob(writer *w, prob_t prob)
{
	write_64(w, prob);
}

void write_data(writer *w, data_t data)
{
	write_byte(w, data);
}

void write_bit(writer *w, uint8_t bit)
{
	w->bits |= bit << w->len;
	w->len++;
	if (w->len == 8)
	{
		fputc(w->bits , w->stream);
		w->len = 0;
		w->bits = 0;
	}
}

void writer_close(writer *w)
{
	if (w->len)
		fputc(w->bits , w->stream);
	free(w);
}

reader *reader_init(FILE* fin)
{
	reader *res = malloc(sizeof(writer));
	res->stream = fin;
	res->len = 0;
	res->bits = 0;
	return res;
}

uint8_t read_byte(reader *r)
{
	int c;
	uint8_t res;
	if ((c = fgetc(r->stream)) == EOF)
	{
		fprintf(stderr, INVALID);
		exit(1);
	}
	res = r->bits >> (8 - r->len) | (uint8_t)c << r->len;
	r->bits = (uint8_t)c;
	r->bits &= (-1) << (8 - r->len);
	return res;
}

uint8_t read_bit(reader *r)
{
	uint8_t res;
	int c;
	if (r->len == 0)
	{
		if ((c = fgetc(r->stream)) == EOF)
		{
			fprintf(stderr, INVALID);
			exit(1);
		}
		r->bits = c;
		r->len = 8;
	}
	res = (r->bits >> (8 - r->len)) & 1;
	r->len--;
	r->bits &= (-1) << (8 - r->len);
	return res;
}

uint8_t read_bit_or_zero(reader *r)
{
	uint8_t res;
	int c;
	if (r->len == 0)
	{
		if ((c = fgetc(r->stream)) == EOF)
			return 0;
		r->bits = c;
		r->len = 8;
	}
	res = (r->bits >> (8 - r->len)) & 1;
	r->len--;
	r->bits &= (-1) << (8 - r->len);
	return res;
}

data_t read_data(reader *r)
{
	return read_byte(r);
}

uint64_t read_64(reader *r)
{
	uint64_t res = 0;
	size_t i;

	for (i = 0; i < 8; ++i)
		res |= (prob_t)read_byte(r) << (8 * i);

	return res;
}

prob_t read_prob(reader *r)
{
	return read_64(r);
}

void reader_close(reader *r)
{
	free(r);
}

pdata_arr *calculate_prob(FILE* fin)
{
	int c;
	pdata_arr *res;

	res = calloc(1, sizeof(pdata_arr));
	res->sum = 0;
	
	while ((c = fgetc(fin)) != EOF) {
		res->data[c].data = (data_t)c;
		res->data[c].prob++;
		res->sum++;
	};

	return res;
}

void pdata_normalize(pdata_arr *arr)
{
	prob_t s, t;
	size_t max = 0, i;

	s = arr->data[0].prob;
	arr->data[0].prob *= (ONE / arr->sum);
	t = arr->data[0].prob;
	for (i = 1; i < ALPHABET_SIZE; ++i) {
		s += arr->data[i].prob;
		arr->data[i].prob = s * (ONE / arr->sum) - t;
		t += arr->data[i].prob;
		if (arr->data[i].prob > arr->data[max].prob)
			max = i;
	}
	arr->data[max].prob += ~t + 1;
	arr->sum = 0;
}

int pdata_cmp_by_prob(const void *first, const void *second)
{
	if (((pdata *)first)->prob < ((pdata *)second)->prob)
		return 1;
	else if (((pdata *)first)->prob > ((pdata *)second)->prob)
		return -1;
	else
		return 0;
}

void sort_desc(pdata_arr *prob)
{
	qsort(prob->data, ALPHABET_SIZE, sizeof(pdata), pdata_cmp_by_prob);
}

#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdint.h>
#include "constants.h"

typedef struct {
	FILE* stream;
	uint8_t bits, len;
} writer;

writer *writer_init(FILE* fout);
void write_byte(writer *w, uint8_t byte);
void write_bit(writer *w, uint8_t bit);
void write_data(writer *w, data_t data);
void write_64(writer *w, uint64_t prob);
void write_prob(writer *w, prob_t prob);
void writer_close(writer *w);

typedef struct
{
	FILE* stream;
	uint8_t bits, len;
} reader;

reader *reader_init(FILE* fout);
uint8_t read_byte(reader *r);
uint8_t read_bit_or_zero(reader *r);
uint8_t read_bit(reader *r);
data_t read_data(reader *r);
uint64_t read_64(reader *r);
prob_t read_prob(reader *r);
void reader_close(reader *r);


typedef struct {
	prob_t prob;
	data_t data;
} pdata;

typedef struct {
	prob_t sum;
	pdata data[ALPHABET_SIZE];
} pdata_arr;

pdata_arr *calculate_prob(FILE* fin);
void pdata_normalize(pdata_arr *arr);
void sort_desc(pdata_arr *prob);

typedef uint64_t path_t;

#endif

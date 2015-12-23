#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <stdio.h>
#include <stdint.h>
#include "prefix.h"
#include "util.h"

prefix_coder *generate_huffman_coder(pdata_arr *prob);
void huffman_code(FILE* fin, FILE* fout);
void huffman_decode(FILE* fin, FILE* fout);

#endif //HUFFMAN_H

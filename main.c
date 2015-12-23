#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "shannon.h"
#include "fano.h"
#include "huffman.h"
#include "dhuffman.h"
#include "bhuffman.h"
#include "arithmetic.h"

void usage(char* name);

void decode(FILE *fin, FILE *FOUT);

int main(int argc, char **argv)
{
	FILE *fin, *fout;
	int res = 0;

	if (argc != 4) {
		usage(argv[0]);
		exit(1);
	}

	if (strlen(argv[1]) != 1) {
			usage(argv[0]);
			exit(1);
	}

	if (!(fin = fopen(argv[2], "rb"))) {
		fprintf(stderr, "Unable to open input file %s\n", argv[2]);
		exit(1);
	}

	if (!(fout = fopen(argv[3], "wb"))) {
		fprintf(stderr, "Unable to open output file %s\n", argv[3]);
		exit(1);
	}

	switch(argv[1][0]) {
		case 's':
			fprintf(stderr, "Shannon coding\n");
			shannon_code(fin, fout);
			break;
		case 'f':
			fprintf(stderr, "Shannon-Fano coding\n");
			fano_code(fin, fout);
			break;
		case 'h':
			fprintf(stderr, "Huffman coding\n");
			huffman_code(fin, fout);
			break;
		case 'r':
			fprintf(stderr, "Arithmetic coding\n");
			arithmetic_code(fin, fout);
			break;
		case 'a':
			fprintf(stderr, "Adaptive Huffman coding\n");
			dhuffman_code(fin, fout);
			break;
		case 'b':
			fprintf(stderr, "Bigramm Huffman coding\n");
			bhuffman_code(fin, fout);
			break;
		case 'd':
			fprintf(stderr, "Decoding\n");
			decode(fin, fout);
			break;
		default:
			usage(argv[0]);
			res = 1;
	}

	fclose(fin);
	fclose(fout);

	return res;
}

void usage(char* name)
{
	printf("Usage: %s <op> <src> <dst>\n", name);
	printf("Operations:\n");
	printf("s: Shannon coding\n");
	printf("f: Shannon-Fano coding\n");
	printf("h: Huffman coding\n");
	printf("r: Arithmetic coding\n");
	printf("a: Adaptive Huffman coding\n");
	printf("b: Bigramm Huffman coding\n");
	printf("d: Decoding\n");
}

void decode(FILE *fin, FILE *fout)
{
	int mode;
	if ((mode = fgetc(fin)) == EOF) {
		fprintf(stderr, INVALID);
		return;
	}

	switch(mode) {
		case SHANNON:
			fprintf(stderr, "Decoding Shannon\n");
			shannon_decode(fin, fout);
			break;
		case FANO:
			fprintf(stderr, "Decoding Fano\n");
			fano_decode(fin, fout);
			break;
		case HUFFMAN:
			fprintf(stderr, "Decoding Huffman\n");
			huffman_decode(fin, fout);
			break;
		case DHUFFMAN:
			fprintf(stderr, "Decoding Adaptive Huffman\n");
			dhuffman_decode(fin, fout);
			break;
		case BHUFFMAN:
			fprintf(stderr, "Decoding Bigramm Huffman\n");
			bhuffman_decode(fin, fout);
			break;
		case ARITHMETIC:
			fprintf(stderr, "Decoding Arithmetic\n");
			arithmetic_decode(fin, fout);
			break;
		default:
			fprintf(stderr, INVALID);
			return;
	}
}

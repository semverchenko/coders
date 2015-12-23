#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stdint.h>

typedef uint64_t prob_t;
typedef uint8_t data_t;
#define ALPHABET_SIZE 256
#define ONE (~((prob_t)1))
#define HALF (((prob_t)1) << (sizeof(prob_t) * 8 - 1))
#define QUARTER (((prob_t)1) << (sizeof(prob_t) * 8 - 2))

#define INVALID "ERROR: invalid file format\n"
#define ALGORITHM_ERROR "ERROR: algorithm error\n"

#define SHANNON 1
#define FANO 2
#define HUFFMAN 3
#define DHUFFMAN 4
#define BHUFFMAN 5
#define ARITHMETIC 6

#endif //CONTANTS_H

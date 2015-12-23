#ifndef DHUFFMAN_H
#define DHUFFMAN_H
#include <stdio.h>
#include <stdint.h>

void dhuffman_code(FILE* fin, FILE* fout);
void dhuffman_decode(FILE* fin, FILE* fout);

#endif //DHUFFMAN_H

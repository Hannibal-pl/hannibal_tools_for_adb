/*
Copyright (c) 2025, Konrad Rzepecki <hannibal@astral.lodz.pl>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OFMERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

unsigned remap[16][256];
uint16_t controlSum = 0;

void createRemap(unsigned remap[256], char map[9]) {
	for (unsigned i = 0; i <= 255; i++) {
		unsigned replacement = 0;
		for (unsigned j = 0; j < 8; j++) {
			if (i & (1 << j)) {
				replacement |= (1 << (map[j] - '0'));
			}
		}
		remap[i] = replacement;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Usage\n\t%s HI_ROM LO_ROM\n", basename(argv[0]));
		return -1;
	}

	FILE *hi = fopen(argv[1], "r");
	if (!hi) {
		fprintf(stderr, "Cannot open HI file '%s'.\n", argv[1]);
		return -1;
	}
	fseek(hi, 0, SEEK_END);
	unsigned size = ftell(hi);
	fseek(hi, 0, SEEK_SET);

	FILE *lo = fopen(argv[2], "r");
	if (!lo) {
		fprintf(stderr, "Cannot open LO file '%s'.\n", argv[2]);
		return -1;
	}

	fseek(lo, 0, SEEK_END);
	if (size != ftell(lo)) {
		fprintf(stderr, "Rom size differs.\n");
		return -1;
	}
	fseek(lo, 0, SEEK_SET);


	unsigned char *mem_hi = malloc(size);
	unsigned char *mem_lo = malloc(size);
	unsigned char *mem = malloc(size * 2);
	if (!mem_hi || !mem_lo || !mem) {
		fprintf(stderr, "Not enough memory.\n");
		fclose(lo);
		fclose(hi);
		return -1;
	}
	fread(mem_hi, size, 1, hi);
	fread(mem_lo, size, 1, lo);
	fclose(lo);
	fclose(hi);

//	createRemap(remap[0], "35601274");
//	createRemap(remap[1], "36501274");
//	createRemap(remap[2], "35610274");
//	createRemap(remap[3], "36510274");
//	createRemap(remap[4], "25601374");
//	createRemap(remap[5], "26501374");
//	createRemap(remap[6], "25610374");
//	createRemap(remap[7], "26510374");

	createRemap(remap[0], "35601247");
	createRemap(remap[1], "36501247");
	createRemap(remap[2], "35610247");
	createRemap(remap[3], "36510247");
	createRemap(remap[4], "25601347");
	createRemap(remap[5], "26501347");
	createRemap(remap[6], "25610347");
	createRemap(remap[7], "26510347");

	createRemap(remap[8], "35601247");
	createRemap(remap[9], "36501247");
	createRemap(remap[10], "35610247");
	createRemap(remap[11], "36510247");
	createRemap(remap[12], "25601347");
	createRemap(remap[13], "26501347");
	createRemap(remap[14], "25610347");
	createRemap(remap[15], "26510347");

	for (unsigned i = 0; i < size; i++) {
		mem[i * 2]     = mem_lo[(i & 0xFFFFFF00) + remap[(i & 0xF0) >> 4][i & 0xFF]];
		mem[i * 2 + 1] = mem_hi[(i & 0xFFFFFF00) + remap[(i & 0xF0) >> 4][i & 0xFF]];
	}

	for (unsigned i = 0; i < 0xFFFE; i++) {
		controlSum += (uint16_t)mem[i];
	}
	controlSum = ((controlSum & 0xFF00) >> 8) | ((controlSum & 0x00FF) << 8);
	if (controlSum == *(uint16_t *)&mem[0xFFFE]) {
		fprintf(stderr, "Control Sum OK.\n");
	} else {
		fprintf(stderr, "Control Sum Failed.\n");
	}

	fwrite(mem, size * 2, 1, stdout);
	fflush(stdout);
	fsync(1);

	return 0;
}

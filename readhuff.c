#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define  ALPHABET_SIZE 256
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

typedef unsigned char Byte;

int main (int argc, char **argv) {
	
	FILE *fp;
	int i;
	Byte byte;
	Byte paddingBits;
	Byte headerFreqFieldSize;
	unsigned long tmp = 0;
	unsigned long frequencies[ALPHABET_SIZE];

	if ((fp = fopen(argv[1], "rb")) == NULL) {
		printf("Erro!!!\n");
		return 1;
	}
	
	fread(&paddingBits, 1, 1, fp);
	fread(&headerFreqFieldSize, 1, 1, fp);
	for (i = 0; i < ALPHABET_SIZE; i++) {
		// É necessário zerar o valor antes, pois, caso contrário, os bytes
		// exedentes podem possuir qualquer valor e o valor final não será
		// o valor real.
		frequencies[i] = 0;
		fread(&frequencies[i], headerFreqFieldSize, 1, fp);
		
		/*if (headerFreqFieldSize == sizeof(unsigned char)) 
			frequencies[i] = (unsigned long)frequencies[i];
		else if (headerFreqFieldSize == sizeof(unsigned short)) 
			frequencies[i] = (unsigned long)frequencies[i];
		else if (headerFreqFieldSize == sizeof(unsigned int))
			frequencies[i] = (unsigned long)frequencies[i];
		else if (headerFreqFieldSize == sizeof(unsigned long))
			frequencies[i] = (unsigned long)frequencies[i];*/

		//frequencies[i] = (unsigned char)frequencies[i];
		//fread(&tmp, headerFreqFieldSize, 1, fp);
		//frequencies[i] = (unsigned char)tmp;
	}

	printf("=== HEADER ===\n");
	printf("padding bits:        %d\n", paddingBits);
	printf("freq. size in bytes: %d\n\n", headerFreqFieldSize);
	printf("byte    freq\n");
	for (i = 0; i < ALPHABET_SIZE; i++) {
		if (frequencies[i] > 0)
			printf("[%3u] - %lu\n", i, frequencies[i]);
	}
	printf("\n=== COMPRESSED FILE ===\n");

	while (fread(&byte, 1, 1, fp)) {
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
	}
	printf("\n");
	fclose(fp);
	
	return 0;
}

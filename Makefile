all: huffman readhuff

huffman: huffman.c
	gcc -o huffman huffman.c

readhuff: readhuff.c
	gcc -o readhuff readhuff.c

clean: huffman 
	rm huffman readhuff

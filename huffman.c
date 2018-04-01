// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Define o tamanho de alfabeto utilizado
#define  ALPHABET_SIZE 256

// Protótipo das estruturas
struct Node;
struct HuffmanTree;

// Protótipo das funções
struct Node *createNode(unsigned char byte, unsigned long freq);
struct Node *joinNodes(struct Node *left, struct Node *right);
struct HuffmanTree *buildHuffmanTree(unsigned long frequencies[]);
void freeHuffmanTree(struct HuffmanTree **huffman);
void compress(char *ifilename, char *ofilename);
void decompress(char *ifilename, char *ofilename);

// Função principal
int main(int argc, char **argv) {

	// Formato do comando para compressão
	// huffman c input.xxx output.huff
	// onde: 
	// 		c indica que é uma compressão
	//		input é o arquivo a ser comprimido
	//		output é o arquivo de destino da compressão
	//
	// Formato do comando para descompressão
	// huffman d input.huff output.xxx
	// onde: 
	// 		d indica que é uma descompressão
	//		input é o arquivo a ser descomprimido
	//		output é o arquivo de destino da descompressão
	//
	// Obs.: A extensão do arquivo comprimido deve ser a mesma do 
	// arquivo descomprimido, representada acima por ".xxx".
	// A extensão do destino da compressão deve ser a mesma da
	// origem da descompressão, representada anteriormente por 
	// ".huff", mas pode ser qualquer outra, desde que seja repetida.
	
	// Comando inválido
	if (argc < 4) {
		printf("número de argumentos insuficiente\n");
		exit(1);
	}

	// Comando para compressão
	else if (argv[1][0] == 'c' && argv[1][1] == '\0') {
		if (argc == 4) {
			// Comprime arquivo selecionado
			compress(argv[2], argv[3]);
		}
		else {
			printf("sintaxe incorreta\n");
			exit(1);
		}
	}

	// Comando para descompressão
	else if (argv[1][0] == 'd' && argv[1][1] == '\0') {		
		if (argc == 3) {
			// Descomprime arquivo selecionado
			decompress(argv[2], argv[3]);
		}
		else {
			printf("sintaxe incorreta\n");
			exit(1);
		}
	}
	else {
		printf("sintaxe incorreta\n");
	}

	return 0;
}

// Estrutura do nó utilizada na árvore de Huffman
struct Node {
	struct Node *parent;		// Ponteiro para o nó pai
	struct Node *left;			// Ponteiro para o filho da esquerda
	struct Node *right;			// Ponteiro para o filho da direita
	unsigned char byte;			// Valor do byte (só é válido para as folhas)
	unsigned long freq;			// Frequência do nó
};

// Estrutura da árvore de Huffman
struct HuffmanTree {
	struct Node *root;			// Ponteiro para o nó raiz da árvore
	struct Node **leaf;			// Utilizado como array de ponteiros para cada uma 
								// das folhas. Como existe a possibilidade de 256 folhas,
								// caso seja necessário acessar a folha referente ao byte
								// é so acessar leaf[byte], já que o byte varia de 0 a 255.
								// Se o byte em questão não fizer parte da árvore, o ponteiro
								// leaf[byte] aponta para NULL.
};

// Cria um nó
struct Node *createNode(unsigned char byte, unsigned long freq) {
	struct Node *n;
	n = (struct Node *) malloc(sizeof(struct Node));
	n->parent = NULL;
	n->left = NULL;
	n->right = NULL;
	n->byte = byte;
	n->freq = freq;

	return n;
}

// Junta dois nós durante a construção da árvore de Huffman
struct Node *joinNodes(struct Node *left, struct Node *right) {
	struct Node *new;
	new = (struct Node *) malloc(sizeof(struct Node));
	new->parent = NULL;
	new->left = left;
	new->right = right;
	new->byte = 0;								// Este valor só importa para as folhas...
	new->freq = left->freq + right->freq;		// Soma a frequência dos dois nós
	left->parent = new;								
	right->parent = new;
	
	return new;	
}

// Constroi a árvore de Huffman a partir de um array com as frequências para cada byte
// onde a posição do array representa o byte.
struct HuffmanTree *buildHuffmanTree(unsigned long frequencies[]) {
	int i = 0;							// Variáveis auxiliares
	int j = 0;
	int validLeafs = 0;					// Número de folhas válidas
	struct Node *tmp;					// Ponteiro para nó auxiliar
	struct Node **tmpRoots;				// Array de ponteiros (árvores temporárias)
	struct HuffmanTree *huffman;		// Árvore de Huffman

	// Aloca espaço para a árvore de huffman
	huffman = (struct HuffmanTree *) malloc(sizeof(struct HuffmanTree));
	huffman->leaf = (struct Node **) malloc(ALPHABET_SIZE * sizeof(struct Node *));
	tmpRoots = (struct Node **) malloc(ALPHABET_SIZE * sizeof(struct Node *));

	// Cria nós para as frequências que não forem nulas
	for (i = 0; i < ALPHABET_SIZE; i++) {
		// Frequência não nula... cria folha
		if (frequencies[i] > 0) {
			huffman->leaf[i] = createNode(i, frequencies[i]);
			tmpRoots[i] = huffman->leaf[i];
			validLeafs++;
		}
		// Frequência nula... não cria folha
		else {
			huffman->leaf[i] = NULL;
			tmpRoots[i] = NULL;
		}
	}

	// Ordena os nós criados através do insertion sort
	for (i = 1; i < ALPHABET_SIZE; i++) {
		tmp = tmpRoots[i];
		for (j = i - 1; j >= 0; j--) {
			if (tmp == NULL) {
				break;
			}
			else if (tmpRoots[j] == NULL) {
				tmpRoots[j+1] = tmpRoots[j];
			}
			else {
				if (tmp->freq > tmpRoots[j]->freq)
					tmpRoots[j+1] = tmpRoots[j];
				else 
					break;
			}
		}
		tmpRoots[j+1] = tmp;
	}

	// Caso só haja uma folha válida, não é necessário juntar as árvores temporárias
	if (validLeafs == 1) {
		tmp = tmpRoots[0];
		tmpRoots[0] = createNode(0, tmpRoots[0]->freq);
		tmpRoots[0]->left = tmp;
		tmp->parent = tmpRoots[0];
	}
	// Caso exista mais de uma folha válida, é preciso juntar as árvores temporárias
	for (i = validLeafs - 1; i > 0; i--) {
		// Junta os dois nós menores e coloca na posição que aparece primeiro
		tmpRoots[i-1] = joinNodes(tmpRoots[i-1], tmpRoots[i]);
		// "apaga" a posição restante
		tmpRoots[i] = NULL;

		// ordena esse novo nó criado, caso exista mais de um no array
		if (i >= 1) {
			for (j = i - 1; j > 0 ; j--) {	// conferir se é pra ser 0 ou 1 aqui...
				if (tmpRoots[j]->freq > tmpRoots[j-1]->freq) {
					tmp = tmpRoots[j];
					tmpRoots[j] = tmpRoots[j-1];
					tmpRoots[j-1] = tmp;
				}
				else
					break;
			}
		}
	}
	// A raiz da árvore está na primeira posição das raizes das ávores temporárias
	huffman->root = tmpRoots[0];
	free(tmpRoots);					
	return huffman;
}

// Desaloca árvore de Huffman
void freeHuffmanTree(struct HuffmanTree **huffman) {
	void freeTree(struct Node *n) {
		if (n != NULL) {
			freeTree(n->left);
			freeTree(n->right);
			free(n);
			n = NULL;
		}
	}

	freeTree((*huffman)->root);		// desaloca nós recursivamente
	free((*huffman)->leaf);			// desaloca array das possíveis folhas
	free((*huffman));				// desaloca o espaço para o struct da árvore
	(*huffman)->leaf = NULL;		// atribui null para o arrays das folhas
	(*huffman) = NULL;				// atribui null para a árvore
}

// Imprime tabela dos códigos da árvore de Huffman
void huffmanCodes(struct HuffmanTree *huffman) {
	void printCode(struct Node *n, struct Node *prev) {
		if (n != NULL) {
			printCode(n->parent, n);
			if ((n->left == prev) && (prev != NULL)) {
				printf("%c", '0');
			}
			if ((n->right == prev) && (prev != NULL)) {
				printf("%c", '1');
			}
		}
	}

	int i;
	printf("=== HUFFMAN TABLE ===\n");
	printf("sym   freq   code\n");
	for (i = 0; i < ALPHABET_SIZE; i++) {
		if (huffman->leaf[i] != NULL) {		// Caso a folha exista
			//printf("%3d - ", i);			// utilizar essa versão para arquivos no geral
			printf("[%c] - ", i);			// utilizar essa versão para arquivos de texto
			printf("%4lu - ", huffman->leaf[i]->freq);
			printCode(huffman->leaf[i], NULL);
			printf("\n");
		}
	}
}

// Codiga um byte. É necessário que o valor passado para code e bits sejam 0 inicialmente.
// O nó n deve ser a folha que corresponde ao byte que se quer achar o código e prev precisa
// ser NULL. Esta função percorre da folha até a raiz e então faz o caminho de volta, atribuindo
// o valor do código.
void encode(unsigned int *code, unsigned char *bits, struct Node *n, struct Node *prev) {
	if (n != NULL) {
		encode(code, bits, n->parent, n);
		if (prev != NULL) {
			if (n->left == prev) {
				// Adiciona 0
				*code <<= 1;
			}
			else if (n->right == prev) {
				// Adiciona 1
				*code <<= 1;
				*code |= 1;
			}
			(*bits)++;
		}
	}
}

// Comprime um arquivo
void compress(char *ifilename, char *ofilename) {	
	unsigned int  code = 0;
	unsigned char codeBits = 0;
	unsigned char byte = 0;
	unsigned char buffer = 0;
	unsigned char bufferBits = 0;
	unsigned char paddingBits = 0;
	unsigned char headerFreqFieldSize = 0;
	unsigned long maxFreq = 0;
	unsigned long filesize = 0;
	unsigned long frequencies[ALPHABET_SIZE];
	struct HuffmanTree *huffman;
	int i;
	int j;
	FILE *input;
	FILE *output;

	if ((input = fopen(ifilename, "rb")) == NULL) {
		printf("Erro ao abrir o arquivo de entrada\n");
		printf("Abortando programa...\n");
		exit(1);
	}

	if ((output = fopen(ofilename, "wb")) == NULL) {
		printf("Erro ao abrir o arquivo de saída\n");
		printf("Abortando programa...\n");
		exit(1);
	}
	
	// zera o array de frequências
	for (i = 0; i < ALPHABET_SIZE; i++) {
		frequencies[i] = 0;
	}
	
	// Lê cada byte no arquivo de entrada e incrementa suas frequências
	while (fread(&byte, 1, 1, input)) {
		filesize++;
		frequencies[byte]++;
		if (frequencies[byte] > maxFreq)
			maxFreq = frequencies[byte];
	}

	// Verifica o valor da maior frequência e avalia em quantos bytes
	// ela pode ser armazenada. Isso é feito para adaptar o cabeçalho
	// de acordo com esse valor.
	if (maxFreq <= UCHAR_MAX)
		headerFreqFieldSize = (unsigned char) sizeof(unsigned char);
	else if (maxFreq <= USHRT_MAX) 
		headerFreqFieldSize = (unsigned char) sizeof(unsigned short);
	else if (maxFreq <= UINT_MAX) 
		headerFreqFieldSize = (unsigned char) sizeof(unsigned int);
	else if (maxFreq <= ULONG_MAX) 
		headerFreqFieldSize = (unsigned char) sizeof(unsigned long);
	
	printf("%lu\n", maxFreq);
	printf("%d\n", headerFreqFieldSize);

	// Escreve cabeçalho no arquivo de saída.
	// O primeiro byte indica a quantidade de bits de padding no último byte
	// Inicialmente esse valor é gravado como 0, mas depois que a compressão é
	// feita e esse valor se torna conhecido, o programa volta para a primeira
	// posição do arquivo e rescreve esse valor, se necessário. O segundo byte
	// diz a quantidade de bytes que cada frequência precisa para ser armazenada 
	// corretamente no cabeçalho. Os valores subsequentes são os valores das
	// frequências para os cada um dos bytes possiveis, de 0 a 255.
	fwrite(&paddingBits, 1, 1, output);	
	fwrite(&headerFreqFieldSize, 1, 1, output);
	for (i = 0; i < ALPHABET_SIZE; i++) {
		fwrite(&frequencies[i], headerFreqFieldSize, 1, output);
	}
	
	// Volta para o inicio do arquivo de entrada
	rewind(input);
	
	code = 0;
	codeBits = 0;
	buffer = 0;
	bufferBits = 0;
	paddingBits = 8;

	//huffman = buildHuffmanTree(frequencies);
	//huffmanCodes(huffman);

	// Rotina de compressão
	while (fread(&byte, 1, 1, input)) {

		huffman = buildHuffmanTree(frequencies);
		//huffmanCodes(huffman);

		code = 0;
		codeBits = 0;
		encode(&code, &codeBits, huffman->leaf[byte], NULL);
		
		while (1) {
			if (paddingBits > codeBits) {
				buffer |= code << (paddingBits - codeBits);
				paddingBits -= codeBits;
				break;
			}
			else if (paddingBits <= codeBits) {
				buffer |= code >> (codeBits - paddingBits);
				codeBits -= paddingBits;
				// O buffer está cheio, hora de salvar
				fwrite(&buffer, 1, 1, output);
				paddingBits = 8;
				buffer = 0;
			}
		}
		
		// gera nova arvore de huffman
		frequencies[byte]--;
		//freeHuffmanTree(&huffman);
	}

	if (paddingBits < 8) {
		fwrite(&buffer, 1, 1, output);
		rewind(output);
		fwrite(&paddingBits, 1, 1, output);
	}

	//freeHuffmanTree(&huffman);
	fclose(input);
	fclose(output);
}

void decompress(char *ifilename, char *ofilename) {
	
}

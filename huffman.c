// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// Define o tamanho de alfabeto utilizado
#define  ALPHABET_SIZE 256

// Exibe mensagens durante a execução, caso esteja definada
#define VERBOSE 

// Protótipo das estruturas
struct Node;
struct HuffmanTree;

// Protótipo das funções
struct Node *createNode(unsigned char byte, unsigned long freq);
struct Node *joinNodes(struct Node *left, struct Node *right);
bool isLeaf(struct Node *n);
struct HuffmanTree *buildHuffmanTree(unsigned long frequencies[]);
void freeHuffmanTree(struct HuffmanTree **huffman);
void huffmanCodes(struct HuffmanTree *huffman);
void encode(FILE *input, FILE *output, unsigned long frequencies[]);
void compress(char *ifilename, char *ofilename);
void decode(FILE *input, FILE *output, unsigned long frequencies[]);
void decompress(char *ifilename, char *ofilename);

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
		if (argc == 4) {
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
	unsigned int validLeafs;	// Número de folhas válidas
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

// Verifica se o nó é uma folha
bool isLeaf(struct Node *n) {
	if ((n->left == NULL) && (n->right == NULL))
		return true;
	else
		return false;
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
				if (tmp->freq > tmpRoots[j]->freq) {
					tmpRoots[j+1] = tmpRoots[j];
				}
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
		if (i > 0) {
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
	huffman->validLeafs = validLeafs;
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
			printf("%3d - ", i);			// utilizar essa versão para arquivos no geral
			//printf("[%c] - ", i);			// utilizar essa versão para arquivos de texto
			printf("%4lu - ", huffman->leaf[i]->freq);
			printCode(huffman->leaf[i], NULL);
			printf("\n");
		}
	}
}

// Encodifica os bytes do arquivo
void encode(FILE *input, FILE *output, unsigned long frequencies[]) {
	// Função recursiva utilizada para obter o código de determinado byte e o tamanho desse código
	// É necessário que o valor de code e codeSize sejam zerados antes de serem passados para função.
	// O valor passado para n deve ser o da folha que se quer obter o código e prev deve ser NULL para
	// que a lógica funcione. A função percorre da folha até a raiz da árvore e depois volta para a folha,
	// obtendo o código. 
	void getCode(unsigned int *code, unsigned char *codeSize, struct Node *n, struct Node *prev) {
		if (n != NULL) {
			getCode(code, codeSize, n->parent, n);
			if (prev != NULL) {
				if (n->left == prev) {
					*code <<= 1;				// Adiciona 0 ao código
				}
				else if (n->right == prev) {
					*code <<= 1;				// Adiciona 1 ao código
					*code |= 1;
				}
				(*codeSize)++;
			}
		}	
	}

	unsigned int  code = 0;				// Código do byte lido
	unsigned char codeSize = 0;			// Tamanho do código em bits
	unsigned char byteRead = 0;			// Byte lido no arquivo de entrada
	unsigned char buffer = 0;			// Buffer utilizado para gravar no arquivo de saída
	unsigned char freeBufferBits = 8;	// Variável que indica
	struct HuffmanTree *huffman;		// Árvore de Huffman

	while (fread(&byteRead, 1, 1, input)) {
		huffman = buildHuffmanTree(frequencies);		// Constroi árvore

		code = 0;													// Zera código
		codeSize = 0;												// Zera tamanho do código
		getCode(&code, &codeSize, huffman->leaf[byteRead], NULL);	// Obtém código do byte lido
	
		while (true) {
			// Caso a quantidade de bits livres no buffer seja maior que o tamanho do código,
			// então o código pode ser copiado no buffer, mas é necessário sair do loop (por isso 
			// o break) para ler um novo byte e continuar o procedimento
			if (freeBufferBits > codeSize) {
				buffer |= code << (freeBufferBits - codeSize);
				freeBufferBits -= codeSize;
				break;
			}
			// Caso a quandidade de bits livres no buffer seja menor ou igual que o tamanho do
			// código, isso significa que o buffer será totalmente preenchido e que ele pode ser 
			// gravado no arquivo de saída
			else if (freeBufferBits <= codeSize) {
				buffer |= code >> (codeSize - freeBufferBits);
				codeSize -= freeBufferBits;
				fwrite(&buffer, 1, 1, output);
				freeBufferBits = 8;
				buffer = 0;
			}
		}

		frequencies[byteRead]--;		// Decrementa frequência do byte lido
		freeHuffmanTree(&huffman);		// Limpa árvore
	}

	// Caso após a rotina anterior o buffer seja menor do que 8, isso significa o último byte não foi
	// gravado, pois havia bits de padding. O trecho abaixo resolve isso
	if (freeBufferBits < 8) {
		fwrite(&buffer, 1, 1, output);
	}
}

// Comprime um arquivo
void compress(char *ifilename, char *ofilename) {	
	unsigned char byteRead = 0;					// Byte lido
	unsigned char headerFreqFieldSize = 0;		// Define o tamanho do campo das frequências no cabeçalho
	unsigned long maxFreq = 0;					// Frequência máxima entre os bytes
	unsigned long frequencies[ALPHABET_SIZE];	// Array com a frequencia de cada byte
	int i;
	
	FILE *input;
	FILE *output;

	if ((input = fopen(ifilename, "rb")) == NULL) {
		#ifdef VERBOSE
			printf("input file could not be opened\n");
			printf("aborting program...\n");
			exit(1);
		#endif
	}

	if ((output = fopen(ofilename, "wb")) == NULL) {
		#ifdef VERBOSE
			printf("output file could not be opened\n");
			printf("aborting program...\n");
			exit(1);
		#endif
	}
	
	// Zera o array de frequências
	for (i = 0; i < ALPHABET_SIZE; i++) {
		frequencies[i] = 0;
	}
	
	// Lê cada byte no arquivo de entrada e incrementa suas frequências
	while (fread(&byteRead, 1, 1, input)) {
		frequencies[byteRead]++;
		if (frequencies[byteRead] > maxFreq)
			maxFreq = frequencies[byteRead];
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
	
	// Escreve cabeçalho no arquivo de saída.
	// O primeiro byte indica a quantidade de bytes que cada frequência precisa 
	// para ser armazenada corretamente no cabeçalho. Os valores subsequentes
	// são os valores das frequências para os cada um dos bytes possiveis, de 0 a 255.
	
	fwrite(&headerFreqFieldSize, 1, 1, output);
	for (i = 0; i < ALPHABET_SIZE; i++) {
		fwrite(&frequencies[i], headerFreqFieldSize, 1, output);
	}
	
	#ifdef VERBOSE
		struct HuffmanTree *huffman = buildHuffmanTree(frequencies);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printf("Quantity of valid leafs:           %u\n", huffman->validLeafs);
		printf("Maximum byte frequency:            %lu\n", maxFreq);
		printf("Size of freq. field in the header: %u bytes\n\n", headerFreqFieldSize);
		huffmanCodes(huffman);		
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	#endif

	// Volta para o início do arquivo de entrada
	rewind(input);

	#ifdef VERBOSE
		printf("starting compression...\n");
	#endif

	encode(input, output, frequencies);

	#ifdef VERBOSE
		printf("compression finished...\n");
	#endif
	
	fclose(input);
	fclose(output);
}

// Decodifica os bytes do arquivo
void decode(FILE *input, FILE *output, unsigned long frequencies[]) {
	unsigned char byteRead = 0;				// Byte lido no arquivo de entrada
	unsigned char byteDecoded = 0;			// Byte decodificado para ser gravado no arquivo de saída
	unsigned char shiftedBits = 0;			// Bits deslocados
	unsigned int  validLeafs = 0;			// Quantidade de folhas validas
	struct HuffmanTree *huffman;
	struct Node *actualNode;

	huffman = buildHuffmanTree(frequencies);	// Constroi árvore de Huffman pela primeira vez
	actualNode = huffman->root;					// Atribui o nó atual a raiz da árvore
	validLeafs = huffman->validLeafs;			
	
	fread(&byteRead, 1, 1, input);				// Lê o primeiro byte 
	
	do {
		do {
			if ((byteRead >> 7 & 0x01) == 0)		// Verifica se o bit atual do byte lido é 0
				actualNode = actualNode->left;		// Se for, desloca o nó atual para esquerda
			if ((byteRead >> 7 & 0x01) == 1)		// Verifica se o bit atual do byte lido é 1
				actualNode = actualNode->right;		// Se for, desloca o nó atual para esquerda
			byteRead <<= 1;							// Desloca o byte em um bit para que o próximo 
													// bit possa ser lido na próxima iteração
			shiftedBits++;				
		} while (!isLeaf(actualNode) && (shiftedBits < 8));		// Verifica se o nó atual não é uma folha 
																// e se a quantidade de bits deslocados é
																// menor que oito

		// Caso o nó atual seja uma folha, é necessário obter o valor do byte decodificado
		if (isLeaf(actualNode)) {
			byteDecoded = actualNode->byte;
			fwrite(&byteDecoded, 1, 1, output);			// Escreve byte
			freeHuffmanTree(&huffman);					// Limpa árvore de Huffman	
			frequencies[byteDecoded]--;					// Atualiza array de frequências

			if (frequencies[byteDecoded] == 0)			// Se a frequência de determinado byte for zero...
				validLeafs--;							// Decrementa a quantidade de folhas válidas...
			if (validLeafs == 0)						// Se a quantidade de folhas válidas for zero...
				return;									// Termina a função (isso faz com que não seja
														// necessário se preocupar com os bits de padding)

			huffman = buildHuffmanTree(frequencies);	// Reconstroi árvore de Huffman	
			actualNode = huffman->root;					// Atualiza nó atual para o nó raiz da nova ávore
		}
		
		// Caso a quantidade de bits deslocados tenha sido oito, é preciso ler outro byte
		if (shiftedBits == 8) {
			shiftedBits = 0;
			fread(&byteRead, 1, 1, input);
		}

	} while ((shiftedBits < 8) && (byteRead != EOF));
}

// Descomprime arquivo
void decompress(char *ifilename, char *ofilename) {
	unsigned char headerFreqFieldSize; 			
	unsigned long frequencies[ALPHABET_SIZE];
	int i;

	FILE *input;
	FILE *output;

	if ((input = fopen(ifilename, "rb")) == NULL) {
		#ifdef VERBOSE
			printf("input file could not be opened\n");
			printf("aborting program...\n");
			exit(1);
		#endif
	}

	if ((output = fopen(ofilename, "wb")) == NULL) {
		#ifdef VERBOSE
			printf("output file could not be opened\n");
			printf("aborting program...\n");
			exit(1);
		#endif
	}
	
	#ifdef VERBOSE
		printf("reading header from input file...\n");
	#endif

	fread(&headerFreqFieldSize, 1, 1, input);
	for (i = 0; i < ALPHABET_SIZE; i++) {
		// É necessário zerar o valor antes, pois, caso contrário, os bytes
		// exedentes podem possuir qualquer valor e o valor final não será
		// o valor real.
		frequencies[i] = 0;
		fread(&frequencies[i], headerFreqFieldSize, 1, input);
	}

	#ifdef VERBOSE
		struct HuffmanTree *huffman = buildHuffmanTree(frequencies);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printf("Quantity of valid leafs:           %u\n", huffman->validLeafs);
		printf("Size of freq. field in the header: %u bytes\n\n", headerFreqFieldSize);
		huffmanCodes(huffman);		
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	#endif

	#ifdef VERBOSE
		printf("starting decompression...\n");
	#endif

	decode(input, output, frequencies);

	#ifdef VERBOSE
		printf("decompression finished...\n");
	#endif

	fclose(input);
	fclose(output);
}

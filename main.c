#pragma once 

#define _CRT_SECURE_NO_WARNINGS 1  
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
#pragma comment(lib,"pthreadVC2.lib") 
#define HAVE_STRUCT_TIMESPEC 

// Inclus�o das bibliotecas necess�rias
#include <pthread.h> // Para programa��o com threads (POSIX threads)
#include <stdio.h>   // Para entrada e sa�da padr�o (printf, scanf)
#include <stdlib.h>  // Para aloca��o de mem�ria (malloc, free), fun��es de sistema (system)
#include <math.h>    // Para a fun��o sqrt (raiz quadrada)
#include <time.h>    // Para medir o tempo de execu��o (clock)
#include <string.h>  // Para manipula��o de strings (n�o diretamente usado aqui, mas inclu�do)


// --- DEFINI��ES DE CONSTANTES ---

// Define as dimens�es da matriz.
#define MAX_MATRIX_ROWS 20000
#define MAX_MATRIX_COLS 20000
// Define o tamanho de um "macrobloco".
// Para uma paraleliza��o mais eficiente em cen�rios reais, esses valores poderiam ser maiores (ex: 16x16).
#define BLOCK_SIZE_X 1 // Tamanho do bloco em linhas
#define BLOCK_SIZE_Y 1 // Tamanho do bloco em colunas
// Calcula a quantidade total de macroblocos na matriz.
#define QTD_MACROBLOCKS (MAX_MATRIX_ROWS / BLOCK_SIZE_X) * (MAX_MATRIX_COLS / BLOCK_SIZE_Y)
// Define a quantidade de threads que ser�o criadas para a busca paralela.
#define QTD_THREADS 4
// Semente para o gerador de n�meros aleat�rios, garantindo resultados repet�veis nos testes.
#define SEED 42


// --- VARI�VEIS GLOBAIS ---

int** matrix; // Matriz global para ser acessada pelas threads.

// Calcula o n�mero de blocos por linha e por coluna.
int n_blocks_row = MAX_MATRIX_ROWS / BLOCK_SIZE_X;
int n_blocks_col = MAX_MATRIX_COLS / BLOCK_SIZE_Y;

// Vari�vel compartilhada que indica o pr�ximo bloco a ser processado pelas threads.
int next_block = 0;
// Mutex para proteger a vari�vel global 'total_primes' durante a atualiza��o.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex para proteger a vari�vel 'next_block', garantindo que cada thread pegue um bloco �nico.
pthread_mutex_t block_mutex = PTHREAD_MUTEX_INITIALIZER;

// Contador global para o n�mero total de primos encontrados na busca paralela.
int total_primes = 0;

// Fun��o para gerar um n�mero inteiro aleat�rio entre 0 e 31999.
int randint() {
	return rand() % 32000;
}

// Aloca dinamicamente a mem�ria para a matriz 2D.
int** allocate_matrix() {
	// Aloca um array de ponteiros (um para cada linha).
	int** matrix = (int**)malloc(sizeof(int*) * MAX_MATRIX_ROWS);
	if (matrix == NULL) {
		fprintf(stderr, "Erro ao alocar linhas da matriz.\n");
		exit(EXIT_FAILURE);
	}
	// Para cada linha, aloca um array de inteiros (as colunas).
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		matrix[i] = (int*)malloc(MAX_MATRIX_COLS * sizeof(int));
		if (matrix[i] == NULL) {
			fprintf(stderr, "Erro ao alocar colunas da matriz na linha %d.\n", i);
			exit(EXIT_FAILURE);
		}
	}
	return matrix;
}

// Verifica se um n�mero � primo.
int ehPrimo(int number) {
	// N�meros menores ou iguais a 1 n�o s�o primos.
	if (number <= 1) {
		return 0; // Retorna 0 para "n�o � primo".
	}

	// Otimiza��o: s� � preciso verificar divisores at� a raiz quadrada do n�mero.
	int square_root = (int)sqrt(number);
	// Verifica se existe algum divisor de 2 at� a raiz quadrada.
	for (int i = 2; i <= square_root; i++) {
		if (number % i == 0) {
			return 0; // Se encontrar um divisor, n�o � primo.
		}
	}

	// Se nenhum divisor for encontrado, o n�mero � primo.
	return 1; // Retorna 1 para "� primo".
}

// Preenche a matriz com n�meros aleat�rios usando a fun��o randint.
void insert_matrix(int** matrix) {
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			matrix[i][j] = randint();
		}
	}
}

// Executa a busca por primos de forma sequencial (single-thread).
double serial_search(int** matrix) {
	double serial_start_time = (double)clock() / CLOCKS_PER_SEC; // Inicia a contagem de tempo.
	int cont_primos = 0;
	// Itera por todos os elementos da matriz.
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			if (ehPrimo(matrix[i][j]) == 1) {
				cont_primos++; // Incrementa o contador local se o n�mero for primo.
			}
		}
	}
	double serial_end_time = (double)clock() / CLOCKS_PER_SEC; // Finaliza a contagem de tempo.
	printf("Total de numeros primos encontrados: %d\n", cont_primos);
	return serial_end_time - serial_start_time; // Retorna o tempo de execu��o.
}

// Imprime todos os elementos da matriz na tela (pode ser muito lento para matrizes grandes).
void print_matrix(int** matrix)
{
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			printf("%d ", matrix[i][j]);
		}
		printf("\n"); // Quebra de linha ao final de cada linha da matriz.
	}
}

// Imprime a matriz dividida em macroblocos, mostrando o conte�do de cada um.
void print_macroblocks() {
	printf("== IMPRESSAO DOS MACROBLOCOS DA MATRIZ ==\n");
	for (int b = 0; b < QTD_MACROBLOCKS; b++) {
		// Calcula a posi��o inicial (linha e coluna) do bloco 'b'.
		int block_row = (b / n_blocks_col) * BLOCK_SIZE_X;
		int block_col = (b % n_blocks_col) * BLOCK_SIZE_Y;

		printf("\n--- Macrobloco %d ---\n", b);
		printf("Posicao inicial na matriz: [%d][%d]\n", block_row, block_col);
		printf("Conteudo:\n");

		// Itera pelos elementos dentro do macrobloco atual.
		for (int i = block_row; i < block_row + BLOCK_SIZE_X; i++) {
			for (int j = block_col; j < block_col + BLOCK_SIZE_Y; j++) {
				printf("%6d ", matrix[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n== FIM DA IMPRESSAO DOS MACROBLOCOS ==\n");
}


// Fun��o que ser� executada por cada thread. Ela conta primos em blocos da matriz.
void* count_primes_in_block(void* arg) {
	int local_primes = 0; // Contador local de primos para evitar travar o mutex global a cada primo encontrado.

	// Loop infinito: a thread continuar� pegando e processando blocos at� que n�o haja mais nenhum.
	while (1) {
		// --- Se��o Cr�tica para obter um bloco de trabalho ---
		pthread_mutex_lock(&block_mutex); // Trava o mutex para acessar 'next_block'.
		int current_block = next_block;
		if (current_block >= QTD_MACROBLOCKS) {
			// Se n�o h� mais blocos, a thread libera o mutex e encerra seu trabalho.
			pthread_mutex_unlock(&block_mutex);
			break;
		}
		next_block++; // Incrementa o �ndice para que a pr�xima thread pegue o pr�ximo bloco.
		pthread_mutex_unlock(&block_mutex); // Libera o mutex.
		// --- Fim da Se��o Cr�tica ---

		// Calcula as coordenadas do bloco que a thread acabou de pegar.
		int block_row = (current_block / n_blocks_col) * BLOCK_SIZE_X;
		int block_col = (current_block % n_blocks_col) * BLOCK_SIZE_Y;

		// Processa todos os elementos dentro do bloco atribu�do.
		for (int i = block_row; i < block_row + BLOCK_SIZE_X; i++) {
			for (int j = block_col; j < block_col + BLOCK_SIZE_Y; j++) {
				if (ehPrimo(matrix[i][j])) {
					local_primes++; // Incrementa o contador local da thread.
				}
			}
		}
	}

	// --- Se��o Cr�tica para atualizar o contador global ---
	// A thread atualiza o contador global uma �nica vez, no final, com seu total local.
	pthread_mutex_lock(&mutex); // Trava o mutex para proteger 'total_primes'.
	total_primes += local_primes; // Adiciona a contagem local ao total global.
	pthread_mutex_unlock(&mutex); // Libera o mutex.
	// --- Fim da Se��o Cr�tica ---

	pthread_exit(NULL); // Termina a thread.
}

// Executa a busca por primos de forma paralela (multi-thread).
double parallel_search() {
	pthread_t threads[QTD_THREADS];
	// Reseta os contadores globais antes de iniciar.
	total_primes = 0;
	next_block = 0;

	// Inicializa os mutexes (embora j� inicializados estaticamente, � uma boa pr�tica em alguns contextos).
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&block_mutex, NULL);

	double start_time = (double)clock() / CLOCKS_PER_SEC; // Inicia a contagem de tempo.

	// Cria as threads. Cada uma executar� a fun��o 'count_primes_in_block'.
	for (int t = 0; t < QTD_THREADS; t++) {
		pthread_create(&threads[t], NULL, count_primes_in_block, NULL);
	}

	// Aguarda todas as threads terminarem sua execu��o.
	for (int t = 0; t < QTD_THREADS; t++) {
		pthread_join(threads[t], NULL);
	}

	double end_time = (double)clock() / CLOCKS_PER_SEC; // Finaliza a contagem de tempo.

	// Imprime o total de primos encontrado por todos os threads.
	printf("Total de numeros primos encontrados: %d\n", total_primes);

	// Libera os recursos dos mutexes.
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&block_mutex);

	return end_time - start_time; // Retorna o tempo de execu��o.
}

// Fun��o que exibe o menu de op��es para o usu�rio.
void menu(int** matrix) {
	int option;
	double time_serial = 0;
	double time_parallel = 0;
	double time_speedup = 0;
	do {
		printf("\n---------------- MENU -------------------\n");
		printf("1. Print Matrix\n");
		printf("2. Print Matrix's Blocks\n");
		printf("3. Count Prime Numbers (Serial)\n");
		printf("4. Count Prime Numbers (Parallel)\n");
		printf("5. Calculate Speedup (Runs both counters)\n");
		printf("0. Exit\n");
		printf("-----------------------------------------\n");

		if (scanf("%d", &option)) {
			// Limpa a tela do terminal
			system("cls || clear");
			printf("\n\n");
		}
		else {
			printf("** Error: Invalid option!!! **\n**  Please enter a valid option **\n");
			break;
		}

		switch (option)
		{
		case 1:
			printf("** Printing the full matrix...         **\n");
			printf("-----------------------------------------\n");
			print_matrix(matrix);
			break;

		case 2:
			printf("** Printing each block in matrix...    **\n");
			printf("-----------------------------------------\n");
			print_macroblocks();
			break;

		case 3:
			printf("** Running Serial Count...             **\n");
			printf("-----------------------------------------\n");
			time_serial = serial_search(matrix);
			printf("** Run time: %.6f seconds          **\n", time_serial);
			break;
		case 4:
			printf("** Running Parallel Count...           **\n");
			printf("-----------------------------------------\n");
			time_parallel = parallel_search();
			printf("** Run time: %.6f seconds          **\n", time_parallel);
			break;
		case 5:
			printf("** Calculating Speedup...              **\n");
			printf("-----------------------------------------\n");
			// Executa a busca serial
			printf("** Running Serial Count...             **\n");
			time_serial = serial_search(matrix);
			printf("** Run time: %.6f seconds          **\n", time_serial);
			printf("-----------------------------------------\n");
			// Executa a busca paralela
			printf("** Running Parallel Count...           **\n");
			time_parallel = parallel_search();
			printf("** Run time: %.6f seconds          **\n", time_parallel);
			printf("-----------------------------------------\n");

			// Calcula e exibe o speedup
			if (time_parallel > 0) {
				time_speedup = time_serial / time_parallel;
				printf("** Speed Up: %.6fx                     **\n", time_speedup);
			}
			else {
				printf("** Parallel time is zero, cannot calculate speedup. **\n");
			}
			break;

		case 0:
			// Caso 0, sai do loop e encerra o programa.
			break;

		default:
			printf("** Error: Invalid option!!! **\n**  Please enter a valid option **\n");
		}
	} while (option != 0); // O loop continua enquanto a op��o n�o for 0.
}


// Fun��o principal do programa.
int main(int argc, char* argv[]) {
	srand(SEED); // Define a semente do gerador de n�meros aleat�rios.

	// Aloca e preenche a matriz com valores.
	matrix = allocate_matrix();
	insert_matrix(matrix);

	// Exibe o menu interativo para o usu�rio.
	menu(matrix);

	// Libera a mem�ria alocada dinamicamente para a matriz para evitar vazamentos de mem�ria.
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]); // Libera cada linha.
	}
	free(matrix); // Libera o array de ponteiros.

	return 0; // Indica que o programa terminou com sucesso.
}
#pragma once 
#define _CRT_SECURE_NO_WARNINGS 1  
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
#pragma comment(lib,"pthreadVC2.lib") 
#define HAVE_STRUCT_TIMESPEC 

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <time.h>
#include <string.h>


#define MAX_MATRIX_ROWS 10000
#define MAX_MATRIX_COLS 10000
#define SEED 42
#define MAX_ROWS_MACROBLOCK  1
#define MAX_COLS_MACROBLOCK  1
#define N_THREADS 4 // Número de threads para processamento paralelo


int threads = N_THREADS; // Variável global para o número de threads

int** matrix; // Matriz global para ser usada pelas threads

int next_block = 0;				// Variável global para controlar o próximo bloco a ser processado
long total_primes = 0;			// Variável global para contar o total de números primos encontrados
pthread_mutex_t mtx_block;		// Mutex para proteger o acesso à variável total_primes
pthread_mutex_t mtx_count;		// Mutex para proteger o acesso ao próximo bloco
int total_blocks = 0;			// Variável global para contar o total de blocos processados
int n_blocks_row, n_blocks_col; // Variáveis para armazenar o número de blocos por linha e coluna

// Função para gerar número aleatório entre 0 e 31999
int randint() {
	return rand() % 32000;
}

// Função para alocar matriz dinamicamente
int** allocate_matrix() {
	int** matrix = (int**)malloc(sizeof(int*) * MAX_MATRIX_ROWS);
	if (matrix == NULL) {
		fprintf(stderr, "Erro ao alocar linhas da matriz.\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		matrix[i] = (int*)malloc(MAX_MATRIX_COLS * sizeof(int));
		if (matrix[i] == NULL) {
			fprintf(stderr, "Erro ao alocar colunas da matriz na linha %d.\n", i);
			exit(EXIT_FAILURE);
		}
	}
	return matrix;
}


int ehPrimo(int number) {
	// if the number is less than or equal to 1, it is not prime.
	if (number <= 1) {
		return 0;
	}

	int square_root = (int)sqrt(number);
	// Check for divisors from 2 to the square root of the number.
	// If any divisor is found, the number is not prime.
	for (int i = 2; i <= square_root; i++) {
		if (number % i == 0) {
			return 0;
		}
	}

	// If no divisors are found, the number is prime.
	return 1;
}

void insert_matrix(int** matrix) {
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			matrix[i][j] = randint();
			int numero = matrix[i][j];
			// Descomente a linha abaixo para ver os valores
			//printf("%d ", matrix[i][j]);
		}
		//printf("\n"); // Para quebrar linha entre linhas da matriz
	}
}

double serial_search(int** matrix) {
	double serial_start_time = (double)clock() / CLOCKS_PER_SEC; // Inicia o cronômetro
	int cont_primos = 0;
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			if (ehPrimo(matrix[i][j]) == 1) {
				cont_primos++;
			}
		}
	}
	double serial_end_time = (double)clock() / CLOCKS_PER_SEC; // Para o cronômetro
	/*printf("\n * * Matrix Size : %d x %d * *\n", MAX_MATRIX_COLS, MAX_MATRIX_ROWS);
	printf("Busca serial concluida.\n");
	printf("\nTempo de execucao serial: %.2f segundos\n", serial_end_time - serial_start_time);
	printf("Total de numeros primos encontrados: %d\n", cont_primos);*/
	return serial_end_time - serial_start_time; // Retorna o tempo de execução
}

void print_matrix(int** matrix)
{
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		for (int j = 0; j < MAX_MATRIX_COLS; j++) {
			// Descomente a linha abaixo para ver os valores
			printf("%d ", matrix[i][j]);
		}
		printf("\n"); // Para quebrar linha entre linhas da matriz
	}
}

void *count_primes_parallel(void* arg) {
	int block_id = *(int*)arg; // Obtém o ID do bloco a partir do argumento
	int count = 0; // Contador local de números primos neste bloco
	printf("Thread processando bloco %d\n", block_id);
	// Calcula o início e o fim do bloco
	int start_row = (block_id / n_blocks_row) * MAX_ROWS_MACROBLOCK;
	int start_col = (block_id % n_blocks_col) * MAX_COLS_MACROBLOCK;
	int end_row = start_row + MAX_ROWS_MACROBLOCK;
	int end_col = start_col + MAX_COLS_MACROBLOCK;

	// Percorre o bloco e conta os números primos
	for (int i = start_row; i < end_row && i < MAX_MATRIX_ROWS; i++) { 
		for (int j = start_col; j < end_col && j < MAX_MATRIX_COLS; j++) {
			if (ehPrimo(matrix[i][j]) == 1) {
				count++;
			}
		}
	}
	// Protege o acesso à variável total_primes com mutex
	pthread_mutex_lock(&mtx_block);
	total_primes += count;
	pthread_mutex_unlock(&mtx_block);
	return NULL;
}

double parallel_search(int** matrix) {
	double parallel_start_time = (double)clock() / CLOCKS_PER_SEC; // Inicia o cronômetro
	// Calcula o número de blocos na matriz
	n_blocks_row = (MAX_MATRIX_ROWS + MAX_ROWS_MACROBLOCK - 1) / MAX_ROWS_MACROBLOCK;
	n_blocks_col = (MAX_MATRIX_COLS + MAX_COLS_MACROBLOCK - 1) / MAX_COLS_MACROBLOCK;
	total_blocks = n_blocks_row * n_blocks_col;
	pthread_t threads[N_THREADS]; // Array de threads
	int block_ids[N_THREADS]; // IDs dos blocos para cada thread

	// Inicializa os mutexes
	pthread_mutex_init(&mtx_block, NULL);
	pthread_mutex_init(&mtx_count, NULL);

	// Cria as threads
	for (int i = 0; i < N_THREADS; i++) {
		block_ids[i] = i % total_blocks; // Atribui um bloco a cada thread
		pthread_create(&threads[i], NULL, count_primes_parallel, &block_ids[i]);
	}
	// Espera todas as threads terminarem
	for (int i = 0; i < N_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	double parallel_end_time = (double)clock() / CLOCKS_PER_SEC; // Para o cronômetro
	printf("\n * * Matrix Size : %d x %d * *\n", MAX_MATRIX_COLS, MAX_MATRIX_ROWS);
	printf("Busca paralela concluida.\n");
	printf("\nTempo de execucao paralelo: %.2f segundos\n", parallel_end_time - parallel_start_time);
	printf("Total de numeros primos encontrados: %ld\n", total_primes);

	// Libera os mutexes
	pthread_mutex_destroy(&mtx_block);
	pthread_mutex_destroy(&mtx_count);
	return parallel_end_time - parallel_start_time;
}

/* **********  MENU  ********** */
void menu(int **matrix) {
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
		
			printf("** Matrix filled with random numbers... **\n");
			print_matrix(matrix);

            break;

        case 2:
            printf("** Printing each block in matrix...    **\n");
            printf("-----------------------------------------\n");


            break;

        case 3:
            /* SERIAL PRIME COUNTING TEST */
			printf("** Running Serial Count...             **\n");
			printf("-----------------------------------------\n");
			time_serial = serial_search(matrix);
			printf("** Run time: %.6f seconds          **\n", time_serial); // Placeholder for run time
			break;
		case 4:
			/* PARALLEL PRIME COUNTING TEST */
			printf("** Running Parallel Count...           **\n");
			printf("-----------------------------------------\n");
			time_parallel = parallel_search(matrix);
			printf("** Run time: %.6f seconds          **\n", time_parallel); // Placeholder for run time
			break;
		case 5:
			/* SPEEDUP CALCULATION TEST */
			printf("** Calculating Speedup...              **\n");
			printf("-----------------------------------------\n");
			printf("** Running Serial Count...             **\n");
            printf("-----------------------------------------\n");
			time_serial = serial_search(matrix);
			time_parallel = parallel_search(matrix);
			time_speedup = time_serial / time_parallel;
            printf("** Speed Up: %.6f seconds          **\n", time_speedup);

            break;

       
        default:
            printf("** Error: Invalid option!!! **\n**  Please enter a valid option **\n");
        }
    } while (option);

}

int main(int argc, char* argv[]) {
	srand(SEED); // Define a semente do gerador de números aleatórios

	int** matrix = allocate_matrix();
	insert_matrix(matrix);
	
	menu(matrix);

	//if (argc >= 4){
	//	// Se argumentos forem passados, atualiza o número de threads
	//	threads = atoi(argv[1]);
	//	/*MAX_ROWS_MACROBLOCK = atoi(argv[2]);
	//	MAX_COLS_MACROBLOCK = atoi(argv[3]);*/
	//	/*MAX_COLS_MACROBLOCK = atoi(argv[2]);
	//	MAX_ROWS_MACROBLOCK = atoi(argv[3]);*/
	//	if (threads <= 0 || threads > N_THREADS) {
	//		fprintf(stderr, "Número de threads inválido. Usando %d threads.\n", N_THREADS);
	//		threads = N_THREADS;
	//	}
	//}

	n_blocks_row = (MAX_MATRIX_ROWS + MAX_ROWS_MACROBLOCK - 1) / MAX_ROWS_MACROBLOCK;
	n_blocks_col = (MAX_MATRIX_COLS + MAX_COLS_MACROBLOCK - 1) / MAX_COLS_MACROBLOCK;

	total_blocks = n_blocks_row * n_blocks_col;

	pthread_mutex_init(&mtx_block, NULL);
	pthread_mutex_init(&mtx_count, NULL);

	 //Libera a memória alocada
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]);
	}
	free(matrix);

	//Destroy mutexes
	pthread_mutex_destroy(&mtx_block);
	pthread_mutex_destroy(&mtx_count);

	return 0;
}
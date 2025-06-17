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


#define MAX_MATRIX_ROWS 15000
#define MAX_MATRIX_COLS 15000
#define BLOCK_SIZE_X 10 // Tamanho do bloco em linhas
#define BLOCK_SIZE_Y 10 // Tamanho do bloco em colunas
#define QTD_MACROBLOCKS (MAX_MATRIX_ROWS / BLOCK_SIZE_X) * (MAX_MATRIX_COLS / BLOCK_SIZE_Y) // Quantidade de blocos na matriz
#define QTD_THREADS 4 // Quantidade de threads a serem criadas

#define SEED 42



int** matrix; // Matriz global para ser usada pelas threads

int n_blocks_row = MAX_MATRIX_ROWS / BLOCK_SIZE_X;
int n_blocks_col = MAX_MATRIX_COLS / BLOCK_SIZE_Y;

int next_block = 0; // Índice do próximo bloco disponível
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para contagem de primos
pthread_mutex_t block_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para controle de blocos

int total_primes = 0; // Contador global de números primos encontrados

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
	printf("Total de numeros primos encontrados: %d\n", cont_primos); // Descomente para ver o total de primos encontrados
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

void* count_primes_in_block(void* arg) {
	int local_primes = 0;

	while (1) {
		pthread_mutex_lock(&block_mutex);
		int current_block = next_block;
		if (current_block >= QTD_MACROBLOCKS) {
			pthread_mutex_unlock(&block_mutex);
			break; // Todos os blocos foram atribuídos
		}
		next_block++;
		pthread_mutex_unlock(&block_mutex);

		// Calcular coordenadas do bloco
		int block_row = (current_block / n_blocks_col) * BLOCK_SIZE_X;
		int block_col = (current_block % n_blocks_col) * BLOCK_SIZE_Y;

		// Processar o bloco
		for (int i = block_row; i < block_row + BLOCK_SIZE_X; i++) {
			for (int j = block_col; j < block_col + BLOCK_SIZE_Y; j++) {
				if (ehPrimo(matrix[i][j])) {
					local_primes++;
				}
			}
		}
	}

	// Atualizar total global apenas uma vez
	pthread_mutex_lock(&mutex);
	total_primes += local_primes;
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
}

double parallel_search() {
	pthread_t threads[QTD_THREADS];
	total_primes = 0;
	next_block = 0; // Resetar índice dos blocos

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&block_mutex, NULL);

	double start_time = (double)clock() / CLOCKS_PER_SEC;

	for (int t = 0; t < QTD_THREADS; t++) {
		pthread_create(&threads[t], NULL, count_primes_in_block, NULL);
	}

	for (int t = 0; t < QTD_THREADS; t++) {
		pthread_join(threads[t], NULL);
	}

	double end_time = (double)clock() / CLOCKS_PER_SEC;

	// Imprime o total de números primos encontrados
	printf("Total de numeros primos encontrados: %d\n", total_primes);

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&block_mutex);

	return end_time - start_time;
}
/* **********  MENU  ********** */
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
			// Exemplo: imprime o macrobloco 4

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
			time_parallel = parallel_search();
			printf("** Run time: %.6f seconds          **\n", time_parallel); // Placeholder for run time
			break;
		case 5:
			/* SPEEDUP CALCULATION TEST */
			printf("** Calculating Speedup...              **\n");
			printf("-----------------------------------------\n");
			printf("** Running Serial Count...             **\n");
			printf("-----------------------------------------\n");
			time_serial = serial_search(matrix);
			time_parallel = parallel_search();
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

	matrix = allocate_matrix();
	insert_matrix(matrix);

	menu(matrix);


	//Libera a memória alocada
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]);
	}
	free(matrix);


	return 0;
}
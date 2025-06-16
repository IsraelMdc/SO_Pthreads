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


#define MAX_MATRIX_ROWS 9
#define MAX_MATRIX_COLS 9
#define BLOCK_SIZE_X 3 // Tamanho do bloco em linhas
#define BLOCK_SIZE_Y 3 // Tamanho do bloco em colunas
#define QTD_MACROBLOCKS (MAX_MATRIX_ROWS / BLOCK_SIZE_X) * (MAX_MATRIX_COLS / BLOCK_SIZE_Y) // Quantidade de blocos na matriz

#define SEED 42


int** matrix; // Matriz global para ser usada pelas threads

int n_blocks_row = MAX_MATRIX_ROWS / BLOCK_SIZE_X;
int n_blocks_col = MAX_MATRIX_COLS / BLOCK_SIZE_Y;

typedef struct {
	int id;
	int values[BLOCK_SIZE_X][BLOCK_SIZE_Y];
} MacroBlock;


typedef struct {
	int id; // ID da block queue
	MacroBlock block_info; // Informações do bloco que a thread irá processar
	int front; // Índice do próximo bloco a ser processado
	int rear;  // Índice do último bloco processado
} BlockQueue;



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

void split_into_macroblocks(int** matrix) {
	MacroBlock* blocks = malloc(sizeof(MacroBlock) * QTD_MACROBLOCKS);
	int block_id = 0;

	for (int i = 0; i < MAX_MATRIX_ROWS; i += BLOCK_SIZE_X) {
		for (int j = 0; j < MAX_MATRIX_COLS; j += BLOCK_SIZE_Y) {
			blocks[block_id].id = block_id;
			printf("Macroblock %d:\n", block_id);

			for (int rows = 0; rows < BLOCK_SIZE_X; ++rows) {
				for (int cols = 0; cols < BLOCK_SIZE_Y; ++cols) {
					blocks[block_id].values[rows][cols] = matrix[i + rows][j + cols];
					printf("%d ", blocks[block_id].values[rows][cols]);
				}
				printf("\n"); // nova linha após terminar cada linha do bloco
			}

			printf("\n");
			block_id++;
		}
	}
}

//void print_macroblock() {
//	MacroBlock* blocks = malloc(sizeof(MacroBlock) * QTD_MACROBLOCKS);
//	printf("Macrobloco %d:\n", blocks->id);
//	for (int i = 0; i < BLOCK_SIZE_X; ++i) {
//		for (int j = 0; j < BLOCK_SIZE_Y; ++j) {
//			printf("%d ", blocks[blocks->id].values[i][j]);
//		}
//		printf("\n");
//	}
//	printf("\n");
//}

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
			split_into_macroblocks(matrix);

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

			printf("** Run time: %.6f seconds          **\n", time_parallel); // Placeholder for run time
			break;
		case 5:
			/* SPEEDUP CALCULATION TEST */
			printf("** Calculating Speedup...              **\n");
			printf("-----------------------------------------\n");
			printf("** Running Serial Count...             **\n");
			printf("-----------------------------------------\n");
			time_serial = serial_search(matrix);
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


	//Libera a memória alocada
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]);
	}
	free(matrix);


	return 0;
}
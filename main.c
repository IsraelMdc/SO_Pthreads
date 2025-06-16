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


#define MAX_MATRIX_ROWS 1000
#define MAX_MATRIX_COLS 1000
#define BLOCK_SIZE_X 10 // Tamanho do bloco em linhas
#define BLOCK_SIZE_Y 10// Tamanho do bloco em colunas
#define QTD_MACROBLOCKS (MAX_MATRIX_ROWS / BLOCK_SIZE_X) * (MAX_MATRIX_COLS / BLOCK_SIZE_Y) // Quantidade de blocos na matriz

#define SEED 42


int** matrix; // Matriz global para ser usada pelas threads
int id = 0; // ID global os macroblocos

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


#define MAX_MATRIX_ROWS 1000
#define MAX_MATRIX_COLS 1000
#define BLOCK_SIZE_X 10 // Tamanho do bloco em linhas
#define BLOCK_SIZE_Y 10// Tamanho do bloco em colunas
#define QTD_MACROBLOCKS (MAX_MATRIX_ROWS / BLOCK_SIZE_X) * (MAX_MATRIX_COLS / BLOCK_SIZE_Y) // Quantidade de blocos na matriz

#define SEED 42


int** matrix; // Matriz global para ser usada pelas threads
int id = 0; // ID global os macroblocos

typedef struct {
	int id_macroblock;		   // ID do macrobloco
	int start_row; // Linha inicial do bloco
	int end_row;   // Linha final do bloco
	int start_col; // Coluna inicial do bloco
	int end_col;   // Coluna final do bloco
	int* elements; // Número de elementos no bloco (pode ser calculado como (end_row - start_row + 1) * (end_col - start_col + 1))
} BlockInfo;

typedef struct {
	int id; // ID da block queue
	BlockInfo block_info; // Informações do bloco que a thread irá processar
	int front; // Índice do próximo bloco a ser processado
	int rear;  // Índice do último bloco processado
} BlockQueue;




// Função para criar um novo bloco de informações
BlockInfo* create_block_info(int id_atual, int start_col, int end_col, int start_row, int end_row) {
	int* elements = (int*)malloc(sizeof(int));
	// Aloca memória para um novo bloco de informações
	BlockInfo* block_info = (BlockInfo*)malloc(sizeof(BlockInfo));
	if (block_info == NULL) {
		fprintf(stderr, "Erro ao alocar memória para BlockInfo.\n");
		exit(EXIT_FAILURE);
	}
	block_info->id_macroblock = id_atual;
	block_info->start_col = start_col;
	block_info->end_col = end_col;
	block_info->start_row = start_row;
	block_info->end_row = end_row;
	block_info->elements = elements; // Atribui o ponteiro de elementos ao bloco
	return block_info;
}



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

//int* vector_elements(int** matrix)
//{
//
//
//	return elements;
//}

void create_macroblocks(int** matrix, int id) {
	// Cria os macroblocos da matriz
	for (int i = 0; i < MAX_MATRIX_COLS; i += BLOCK_SIZE_Y) {
		for (int j = 0; j < MAX_MATRIX_ROWS; j += BLOCK_SIZE_X) {
			BlockInfo* block_info = create_block_info(id++, i, (i + BLOCK_SIZE_X - 1), j, (j + BLOCK_SIZE_Y - 1));
			// Aqui você pode adicionar o bloco à fila ou processá-lo conforme necessário
			// Exemplo: add_to_queue(block_info);
		}
	}
}

void print_macroblocks(int** matrix) {
	// Imprime os macroblocos da matriz
	for (int i = 0; i < MAX_MATRIX_ROWS; i += BLOCK_SIZE_X) {
		for (int j = 0; j < MAX_MATRIX_COLS; j += BLOCK_SIZE_Y) {
			printf("Macroblock ID: %d\n", id++); // Imprime o ID do macrobloco
			for (int x = i; x < i + BLOCK_SIZE_X; x++) {
				for (int y = j; y < j + BLOCK_SIZE_Y; y++) {
					printf("%d ", matrix[x][y]);
				}
				printf("\n");
			}
			printf("\n"); // Para separar os blocos
		}
	}
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
			print_macroblocks(matrix); // Imprime os macroblocos da matriz


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

	create_block_info(id, 0, BLOCK_SIZE_Y - 1, 0, BLOCK_SIZE_X - 1); // Cria um bloco de informações para o macrobloco
	create_macroblocks(matrix, id); // Cria os macroblocos da matriz

	menu(matrix);

	int id = 0; // ID dos macroblocos

	//Libera a memória alocada
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]);
	}
	free(matrix);


	return 0;
}

typedef struct {
	int id_macroblock;		   // ID do macrobloco
	int start_row; // Linha inicial do bloco
	int end_row;   // Linha final do bloco
	int start_col; // Coluna inicial do bloco
	int end_col;   // Coluna final do bloco
	int *elements; // Número de elementos no bloco (pode ser calculado como (end_row - start_row + 1) * (end_col - start_col + 1))
} BlockInfo;

typedef struct {
	int id; // ID da block queue
	BlockInfo block_info; // Informações do bloco que a thread irá processar
	int front; // Índice do próximo bloco a ser processado
	int rear;  // Índice do último bloco processado
} BlockQueue;


 

// Função para criar um novo bloco de informações
BlockInfo* create_block_info(int id_atual, int start_col, int end_col, int start_row, int end_row) {
	int* elements = (int*)malloc(sizeof(int));
	// Aloca memória para um novo bloco de informações
	BlockInfo* block_info = (BlockInfo*)malloc(sizeof(BlockInfo));
	if (block_info == NULL) {
		fprintf(stderr, "Erro ao alocar memória para BlockInfo.\n");
		exit(EXIT_FAILURE);
	}
	block_info->id_macroblock = id_atual;
	block_info->start_col = start_col;
	block_info->end_col = end_col;
	block_info->start_row = start_row;
	block_info->end_row = end_row;
	block_info->elements = elements; // Atribui o ponteiro de elementos ao bloco
	return block_info;
}



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

void create_macroblocks(int** matrix, int id) {	
	// Cria os macroblocos da matriz
	for (int i = 0; i < MAX_MATRIX_COLS; i += BLOCK_SIZE_Y) {
		for (int j = 0; j < MAX_MATRIX_ROWS; j += BLOCK_SIZE_X) {
			BlockInfo* block_info = create_block_info(id++, i, (i + BLOCK_SIZE_X - 1), j, (j + BLOCK_SIZE_Y - 1));
			// Aqui você pode adicionar o bloco à fila ou processá-lo conforme necessário
			// Exemplo: add_to_queue(block_info);
		}
	}
}

void print_macroblocks(int** matrix) {
	// Imprime os macroblocos da matriz
	for (int i = 0; i < MAX_MATRIX_ROWS; i += BLOCK_SIZE_X) {
		for (int j = 0; j < MAX_MATRIX_COLS; j += BLOCK_SIZE_Y) {
			printf("Macroblock ID: %d\n", id++); // Imprime o ID do macrobloco
			for (int x = i; x < i + BLOCK_SIZE_X; x++) {
				for (int y = j; y < j + BLOCK_SIZE_Y; y++) {
					printf("%d ", matrix[x][y]);
				}
				printf("\n");
			}
			printf("\n"); // Para separar os blocos
		}
	}
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
			print_macroblocks(matrix); // Imprime os macroblocos da matriz


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
	
	create_block_info(id, 0, BLOCK_SIZE_Y - 1, 0, BLOCK_SIZE_X - 1); // Cria um bloco de informações para o macrobloco
	create_macroblocks(matrix, id); // Cria os macroblocos da matriz

	menu(matrix);

	int id = 0; // ID dos macroblocos

	 //Libera a memória alocada
	for (int i = 0; i < MAX_MATRIX_ROWS; i++) {
		free(matrix[i]);
	}
	free(matrix);


	return 0;
}
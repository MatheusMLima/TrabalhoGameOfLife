#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define N 2048 //Tamanho do tabuleiro NxN
#define EXECUTIONS 2000 //N geracao

int vivos = 0; //global pra não ter que ficar passando por parametro esse treco

//Retorna a diferença de tempo entre "start" e "end"
float get_tempo(struct timespec *start, struct timespec *end) {
    float time_taken;
    time_taken = (end->tv_sec - start->tv_sec) + 1e-9 * (end->tv_nsec - start->tv_nsec);
    return time_taken;
}

void iniciaMatriz(float ***tabuleiro){
    *tabuleiro = (float **)calloc(N, sizeof(float *));
     int i;
    for(i = 0; i < N; i++){
        (*tabuleiro)[i] = (float *)calloc(N,sizeof(float));
    }
}

void iniciaGlider(float ***tabuleiro){
    int lin = 1, col = 1;
    (*tabuleiro)[lin][col+1] = 1.0;
    (*tabuleiro)[lin+1][col+2] = 1.0;
    (*tabuleiro)[lin+2][col] = 1.0;
    (*tabuleiro)[lin+2][col+1] = 1.0;
    (*tabuleiro)[lin+2][col+2] = 1.0;
}

void iniciaPentomino(float ***tabuleiro){
    int lin = 10, col = 30;
    (*tabuleiro)[lin][col+1] = 1.0;
    (*tabuleiro)[lin][col+2] = 1.0;
    (*tabuleiro)[lin+1][col] = 1.0;
    (*tabuleiro)[lin+1][col+1] = 1.0;
    (*tabuleiro)[lin+2][col+1] = 1.0;

}

void imprimeMatriz(float **tabuleiro){
	int i, j;
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            printf("%.1f | ", tabuleiro[i][j]);
        }
        printf("\n");
    }
}

int getNeighbors(float **tabuleiro, int lin, int col) { //utilizando topografia de toro
    int vizinhoVivos = 0;
    int dx[] = {-1, 0, 1, 0, -1, -1, 1, 1}; // Deslocamento nas direções horizontal (esquerda e direita)
    int dy[] = {0, -1, 0, 1, -1, 1, -1, 1}; // Deslocamento nas direções vertical (cima e baixo)
	int i;
    for (i = 0; i < 8; i++) {
        int newX = (lin + dx[i] + N) % N; // Garante que os índices estejam dentro dos limites da matriz
        int newY = (col + dy[i] + N) % N;

        if(newX == lin && newY == col){
            continue;
        }

        if(tabuleiro[newX][newY] == 1.0){
            vizinhoVivos++;
        }
    }

    return vizinhoVivos;
}

//Atualizando o tabuleiro com mortos e vivos
void AtualizaGrid(float **grid, float **newgrid, int lines, int j){
    int neighbors;
    neighbors = getNeighbors(grid, lines, j);
    if (grid[lines][j] == 1) {
        if (neighbors == 2 || neighbors == 3) {
            newgrid[lines][j] = 1;
            vivos++;
        } else {
            newgrid[lines][j] = 0;
        }
    } else {
        if (neighbors == 3) {
            newgrid[lines][j] = 1;
            vivos++;
        } else {
            newgrid[lines][j] = 0;
        }
    }
}

void start_game(float **grid, float **newgrid) {
    int i, j, k;
    float tempo;
	int t;
    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start); //Pega o tempo que começa

    //omp_set_dynamic(0);
    //omp_set_nested(1);

    //printf("Quantidade de threads: %d\n", omp_get_max_threads());

    for(i = 1; i <= EXECUTIONS; i++){
        vivos = 0;
        #pragma omp parallel for private(k) reduction(+:vivos) //precisa por k como privado para cada thread ter sua variavel privada e não tem problema de acessar lugar indevido
            for (j = 0; j < N; j++)                             // da mesma forma, j já é privado por padrão, coloca o reduction(+:vivos) para somar a váriavel vivos de todas as threads e retorna o valor máximo daquela geração
            {
                for (k = 0; k < N; k++)
                {
                    AtualizaGrid(grid, newgrid, j, k);
                }
            }


        // Atualiza os grids
        float **temp = grid;
        grid = newgrid;
        newgrid = temp;


        printf("Geracao %d: Celulas vivas: %d\n", i, vivos);
    }

    clock_gettime(CLOCK_REALTIME, &end); //Pegar o tempo de fim

    tempo = get_tempo(&start, &end);

    printf("O tempo levado: %0.3fs\n", tempo);
}

int main(){
    float **grid, **newGrid;
    int i, t, j;
    iniciaMatriz(&grid);
    iniciaMatriz(&newGrid);

    iniciaGlider(&grid);
    iniciaPentomino(&grid);
    omp_set_num_threads(8);
    start_game(grid, newGrid);
}
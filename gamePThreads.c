#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define N 2048 //Tamanho do tabuleiro NxN
#define EXECUTIONS 2000 //N geracao

#define NUM_THREADS 4 //N threads

int vivos = 0; //global pra não ter que ficar passando por parametro esse treco

//Struct das Threads
struct ThreadData {
    int id;
    float **grid;
    float **newgrid;
    int n;
};

//Retorna a diferença de tempo entre "start" e "end"
float get_tempo(struct timespec *start, struct timespec *end) {
    float time_taken;
    time_taken = (end->tv_sec - start->tv_sec) + 1e-9 * (end->tv_nsec - start->tv_nsec);
    return time_taken;
}

void iniciaMatriz(float ***tabuleiro){
    *tabuleiro = (float **)calloc(N, sizeof(float *));
     int i;
    for(i = 0; i < 2048; i++){
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

//Executa uma rodada do Game Of Life
void *play_game(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int id = data->id;
    float **grid = data->grid;
    float **newgrid = data->newgrid;
    int n = data->n;
    int x = id;

    int i, j;

    //Cada thread  N/NUM_THREADS + 1 linhas
    for (i = 0; i < n; i++) {
        if (x >= N)
            break;
        for (j = 0; j < N; j++) {
            AtualizaGrid(grid, newgrid, x, j);
        }
        x += NUM_THREADS;
    }

    pthread_exit(NULL);
}

void start_game(float **grid, float **newgrid) {
    int i;
    pthread_t threads[NUM_THREADS];
    struct ThreadData thread_data[NUM_THREADS];
    float tempo;
	int t;
    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start); //Pega o tempo que começa

    for (i = 1; i <= EXECUTIONS; i++) {
        vivos = 0;
        int n = (N / NUM_THREADS) + 1;
        for (t = 0; t < NUM_THREADS; t++) {
            thread_data[t].id = t;
            thread_data[t].grid = grid;
            thread_data[t].newgrid = newgrid;
            thread_data[t].n = n;
            pthread_create(&threads[t], NULL, play_game, &thread_data[t]);
        }

        for (t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
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

    start_game(grid, newGrid);
}
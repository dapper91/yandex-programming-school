#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ALIVE 'X'
#define DEAD '.'

#define next() ((rank+1)%size)
#define prev() ((rank<=0) ? (size+(rank%size)-1) : (rank-1))
#define max(a,b) ((a) > (b) ? (a) : (b))


int toindex(int row, int col, int N) {
    if (col < 0) {
        col = col + N;
    } else if (col >= N) {
        col = col - N;
    }
    return row * N + col;
}

void printgrid(char* grid, FILE* f, int N) {
    char *buf = (char*)malloc((N+1)*sizeof(char));

    for (int i = 0; i < N; ++i) {
        strncpy(buf, grid + i * N, N);
        buf[N] = 0;
        fprintf(f, "%s\n", buf);
    }

    free(buf);
}

int local_lines(int rank, int size, int N)
{
    int lines = N/size + (N%size == 0 ? 0 : 1);
    if (lines*(rank+1) > N) 
        lines = (lines*(rank+1)-N > lines) ? 0 : (lines - (lines*(rank+1)-N));

    return lines;
}

int main(int argc, char* argv[]) {    
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
        fprintf(stderr, "Usage: %s N input_file iterations output_file\n", argv[0]);
        return 1;
    }
    
    int N = atoi(argv[1]);
    int iterations = atoi(argv[3]);

    if (size > N) {
        if (rank == 0)
            fprintf(stderr, "size(%d) must be less than N(%d)\n", size, N);
        MPI_Finalize();
        return 1;
    }

    int lines = local_lines(rank, size, N);

    char* grid = NULL;
    int* sendcnts = NULL;
    int* displs = NULL;

    if (rank == 0) {
        FILE* input = fopen(argv[2], "r");
        grid = (char*)malloc(N * N * sizeof(char)+1);
        for (int i = 0; i < N; ++i) {
            fscanf(input, "%s", grid + i * N);
        }
        fclose(input);

        displs = (int*)malloc((size+1)*sizeof(int));
        for (int i = 0; i < size; i++) {
            displs[i] = i*N*lines;           
        }
        displs[size] = N*N;

        sendcnts = (int*)malloc((size)*sizeof(int));
        for (int i = 0; i < size; i++) {
            sendcnts[i] = N*local_lines(i, size, N);
        }

        for (int i = 0; i < size; i++) {
            printf("%d ", sendcnts[i]);
        }
    }    

    //printf("%d:%d\n", rank, lines);
    //MPI_Finalize();
    //return 0;

    //int sendcnts[] = {128,128,128,128,128,128,128,128};
    //int displs[] = {0,128,256,384,512,640,768,896};

    //int sendcnts[] = {160,160,160,160,160,160,64};
    //int displs[] = {0,160,320,480,640,800,960};    
    

    char* local_grid = (char*)malloc(N * (lines+2) * sizeof(char));   
    char* buf = (char*)malloc(N * (lines+2) * sizeof(char));   
    MPI_Scatterv(grid, sendcnts, displs, MPI_CHAR, &local_grid[N], N * lines, MPI_CHAR, 0, MPI_COMM_WORLD);
    

    MPI_Status status;
    for (int iter = 0; iter < iterations; iter++) {
        
        MPI_Send(&local_grid[N], N, MPI_CHAR, prev(), 0, MPI_COMM_WORLD);
        MPI_Recv(&local_grid[N*(lines+1)], N, MPI_CHAR, next(), 0, MPI_COMM_WORLD, &status);
       
        MPI_Send(&local_grid[N*lines], N, MPI_CHAR, next(), 0, MPI_COMM_WORLD);
        MPI_Recv(&local_grid[0], N, MPI_CHAR, prev(), 0, MPI_COMM_WORLD, &status);

        for (int i = 1; i < lines+1; i++) {
            for (int j = 0; j < N; j++) {
                int alive_count = 0;
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        if ((di != 0 || dj != 0) && local_grid[toindex(i + di, j + dj, N)] == ALIVE) {
                            ++alive_count;
                        }
                    }
                }
                int current = i * N + j;
                if (alive_count == 3 || (alive_count == 2 && local_grid[current] == ALIVE)) {
                    buf[current] = ALIVE;
                } else {
                    buf[current] = DEAD;
                }
            }
        }

        char* tmp = local_grid; local_grid = buf; buf = tmp;

    } 

    MPI_Gatherv(&local_grid[N], N * lines, MPI_CHAR, grid, sendcnts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        FILE* output = fopen(argv[4], "w");
        printgrid(grid, output, N);
        fclose(output);
        free(grid);
    } 

    free(local_grid);
    free(buf);

    MPI_Finalize();

    return 0;
}
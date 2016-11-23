#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <time.h>
#include "someDefinitions.h"

#define A 10000
#define B 10000
#define C 10000
#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

void multMPI(double *a, double *b, double *c, int rows, int heigth, int width){
	int i, j, k;
	for (k=0; k<width; k++){
		for (i=0; i<rows; i++){
			c[i * width + k] = 0;
			for (j=0; j<heigth; j++){
				c[i * width + k] +=  a[i * heigth + j] * b[j * width + k];
         		}
		}
	}
}

int comparar(double *mat1, double *mat_2, int heigth, int width) {
  for (int i = 0; i < heigth; i++) {
    for (int j = 0; j < width; j++) {
      if (mat1[i * width + j] != mat_2[i * width + j])
        return 0;
    }
  }
  return 1;
}

int main(int argc, char** argv) {
  
  int     numtasks,              /* number of tasks in partition */
        taskid,                /* a task identifier */
        numworkers,            /* number of worker tasks */
        source,                /* task id of message source */
        dest,                  /* task id of message destination */
        mtype,                 /* message type */
        rows,                  /* columnas de los dos vectores a enviar */
        avecol, extra, offset, /* used to determine rows sent to each worker */
        i, j, k, rc;           /* misc */

clock_t start, end;
double time_used;        
int size1 = A * B* sizeof(double); // Tamaño de la matriz 1
int size2 = B * C* sizeof(double); // Tamaño de la matriz 2
int size3 = A * C* sizeof(double); // Tamaño de la matriz resultado

double *a, *b, *c, *cMPI;

MPI_Status status;

MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
if (numtasks < 2 ) {
	printf("Need at least two MPI tasks. Quitting...\n");
  MPI_Abort(MPI_COMM_WORLD, rc);
  exit(1);
  }
numworkers = numtasks-1;


/**************************** master task ************************************/
   if (taskid == MASTER)
   {
	a = (double*)malloc(size1);
	b = (double*)malloc(size2);
	c = (double*)malloc(size3);
	cMPI = (double*)malloc(size3);

      printf("Matriz multiplication has started with %d tasks.\n",numtasks);
      printf("Initializing arrays...\n");

      for (i=0; i<A; i++){
        for (j=0; j<B; j++){
            a[i * B + j] = 2;
	 }
      }
       
      for (i=0; i<B; i++){
	for (j=0; j<C; j++){
            b[i * C + j] = 2;
	 }
      }


	  /* Send matrix data to the worker tasks */
      avecol = A/numworkers;
      extra = A%numworkers;
      offset = 0;
      mtype = FROM_MASTER;
	start = clock();
      for (dest=1; dest<=numworkers; dest++)
      {
         rows = (dest <= extra) ? avecol+1 : avecol;
         printf("Sending %d cols to task %d offset=%d\n",rows,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&a[offset * B], rows* B, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
         MPI_Send(b, B*C, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rows;
		
	  }
	mtype = FROM_WORKER;
      for (i=1; i<=numworkers; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&c[offset * C], rows*C,MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
	//MPI_Recv(&cMPI[offset * C], rows*C,MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
         printf("Received results from task %d\n",source);
      }
     /*
      printf("******************************************************\n");
      printf("Result Matrix:\n");
      for (i=0; i<A; i++)
      {
         printf("\n"); 
         for (j=0; j<C; j++) 
            printf("%6.2f   ", c[i*C+j]);
      }
     */
      printf("\n******************************************************\n");
      printf ("Done.\n");
	end = clock();
	time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Tiempo de ejecución: %.10f\n", time_used);
      }
    
   if (taskid > MASTER)
   {
		mtype = FROM_MASTER;
		
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		a = (double*)malloc(rows * B* sizeof(double)); 
                b = (double*)malloc(B * C* sizeof(double));
                c = (double*)malloc(rows * C * sizeof(double));
               // cMPI = (double*)malloc(rows * C * sizeof(double));
		MPI_Recv(a, rows*B, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(b, B*C, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

		cudaCall(A, B, C, a, b, c);
		//multMPI(a, b, cMPI, rows, B, C);
		//Fin multiplicacion con GPU
		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(c, rows * C, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
		//MPI_Send(cMPI, rows * C, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
		
		}

 		MPI_Finalize();
		free(a);
                free(b);
                free(c);

                //free(cMPI);

}


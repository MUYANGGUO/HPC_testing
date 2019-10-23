#include "dmv.h"
#include "dmv_impl.h"

int DenseMatVec_2dPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, 
                            int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  MPI_Comm comm = args->comm;
  MPI_Comm *rowComm, *colComm;
  int      size, rank, err, row, col;
  int num_cols, num_rows;
  double *vecRight = NULL;
  int *nOffsets = NULL, *rLocals;
  int rOffset = rStart, rLocal = rEnd - rStart, lLocal = lEnd - lStart, nGlobal = nEnd - nStart;
  
  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  
  /* TODO: implement a matrix-vector multiplication on a 2d matrix partition */
  /* HINT:
   * 1. Use DMVCommGetRankCoordinates2D() to get the coordinates of the current rank in a 2d grid of MPI processes.
   * 2. Use the coordinates as colorings to split the communicator into row and column communicators.
   * 3. Use MPI_Allgatherv() on the column communicator so that every process has the correct entries to multiply against its local portion of the matrix ([nStart, nEnd]).
   *      Look at DenseMatVec_RowPartition_Allgatherv() in dmv_matvec_row.c for an example of using MPI_Algatherv() in this way, but adapt it to the column communicator.
   *      You will need to allocate temporary space for the result of the allgatherv, and free it when you are done with it.
   * 4. Compute the local portion of the matrix vector product.
   * 5. Use MPI_Reduce_scatter() on the row communicator to sum all of the row contributions to vecLeftLocal.
   *      Look at DenseMatVec_ColPartition() in dmv_matvec_col.c for an example of use MPI_Reduce_scatter() in this wary, but adapt it to the row communicator.
   */
  DMVCommGetRankCoordinates2D(comm, &num_rows, &row, &num_cols, &cols);
  err = MPI_Comm_split(comm, row, rank, rowComm); MPI_CHK(err);
  err = MPI_Comm_split(comm, col, rank, colComm); MPI_CHK(err);
  vecRight = (double *) malloc(nGlobal * sizeof(*vecRight));
  nLocals = (int *) malloc(size * sizeof(*nOffsets));
  err = MPI_Allgather(&nLocal, 1, MPI_INT, nLocals, 1, MPI_INT, comm); MPI_CHK(err);
  nOffsets = (int *) malloc((size + 1) * sizeof(*nOffsets));
  nOffsets[0] = 0;
  for (int q = 0; q < size; q++) {
    nOffsets[q + 1] = nOffsets[q] + nLocals[q];
  }
  err = MPI_Allgatherv(vecRightLocal, rLocal, MPI_DOUBLE, vecRight, nGlobal, nOffsets, MPI_DOUBLE, colComm); MPI_CHK(err);
 
    
  return 0;
}

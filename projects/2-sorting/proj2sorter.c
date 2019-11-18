#include <math.h>
#include <string.h>
#include "proj2sorter.h"
#include "proj2sorter_impl.h"

int Proj2SorterCreate(MPI_Comm comm, Proj2Sorter *sorter_p)
{
  Proj2Sorter sorter = NULL;
  int err;
  // create subcomm when creating sorter
  int rank, size, group, cnt=0, numComms=1;
  MPI_Comm tempComm, tempSubComm; 
  
  err = PROJ2MALLOC(1,&sorter); PROJ2CHK(err);
  memset(sorter, 0, sizeof(*sorter));
  
  //get rank&size
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  err = MPI_Comm_size(comm, &size); MPI_CHK(err);  
  //calculate # of required comms 
  while(size>1)
  {
    numComm++;
    size = size/2; 
  }
  //create subComm
  sorter->subComms = (MPI_Comm *)malloc(numComms * sizeof(MPI_Comm));
  sorter->comm = comm;
  //assign
  err = MPI_Comm_size(comm, &size);MPI_CHK(err);
  while(1)
  {
    sorter->subComms[cnt]=tempComm;
    cnt++; 
    if size<=1
      break;
    else:
    {
      err = MPI_Comm_rank(tempComm, &rank); MPI_CHK(err);
      group = (rank >= (size / 2));
      err = MPI_Comm_split(tempComm, group, rank, &tempSubComm); PROJ2CHK(err);
      err = MPI_Comm_size(tempSubComm, &size); MPI_CHK(err);
      tempComm = tempSubComm;
    }
  }

  *sorter_p = sorter;
  return 0;
}

int Proj2SorterDestroy(Proj2Sorter *sorter_p)
{
  Proj2Sorter sorter = *sorter_p;
  Proj2MemLink next;
  int err;

  if (sorter->inUse) PROJ2ERR(MPI_COMM_SELF,2,"Work array still in use at exit.\n");
  next = sorter->avail;
  while (next) {
    Proj2MemLink nnext = next->next;
    err = PROJ2FREE(&(next->array)); PROJ2CHK(err);
    err = PROJ2FREE(&next); PROJ2CHK(err);
    next = nnext;
  }
  err = PROJ2FREE(sorter_p); PROJ2CHK(err);
  //free sub
  PROJ2FREE(sorter->subComms);

  return 0;
}

int Proj2SorterGetWorkArray(Proj2Sorter sorter, size_t num, size_t size, void *workArray_p)
{
  Proj2MemLink next = sorter->avail;
  int err;

  if (!(num*size)) {
    *((void **) workArray_p) = NULL;
    return 0;
  }

  if (!next) {
    err = PROJ2MALLOC(1,&next); PROJ2CHK(err);
    memset(next, 0, sizeof(*next));
  } else { /* pop next from available */
    sorter->avail = next->next;
  }
  if (next->size < num * size) {
    err = PROJ2FREE(&(next->array)); PROJ2CHK(err);
    err = PROJ2MALLOC(num*size,&(next->array)); PROJ2CHK(err);
  }
  /* push next on inUse */
  next->next = sorter->inUse;
  sorter->inUse = next;
  *((void **) workArray_p) = (void *) next->array;
  return 0;
}

int Proj2SorterRestoreWorkArray(Proj2Sorter sorter, size_t num, size_t size, void *workArray_p)
{
  Proj2MemLink *prev = &(sorter->inUse);
  Proj2MemLink next = sorter->inUse;
  void *workArray = *((void **) workArray_p);

  if (!(num*size)) {
    return 0;
  }

  while (next) {
    if ((void *) next->array == workArray) break;
    prev = &(next->next);
    next = next->next;
  }
  if (!next) PROJ2ERR(MPI_COMM_SELF, 3, "Unable to locate restoring work array.\n");
  *prev = next->next;
  next->next = sorter->avail;
  sorter->avail = next;
  *((void **) workArray_p) = NULL;
  return 0;
}

int Proj2SorterSort(Proj2Sorter sorter, size_t numKeysLocal, int uniform, uint64_t *keys)
{
  int size;
  int err;

  err = MPI_Comm_size(sorter->comm, &size); PROJ2CHK(err);
  if (size == 1) { err = Proj2SorterSortLocal(sorter, numKeysLocal, keys, PROJ2SORT_FORWARD); PROJ2CHK(err);
  } else if (uniform && (1 << ((int) log2(size))) == size) {
    err = Proj2SorterSortBitonic(sorter, numKeysLocal, uniform, keys); PROJ2CHK(err);
  } else {
    err = Proj2SorterSort_quicksort(sorter, numKeysLocal, uniform, keys); PROJ2CHK(err);
  }

  return 0;
}

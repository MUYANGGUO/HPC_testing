#!/bin/sh

#PBS -N hpl_example
### request one node with 28 cores
#PBS -l nodes=1:ppn=28,walltime=02:00:00
#PBS -q coc-ice

module load cse6230/core
module load mkl/19.0
cd "$HOME"/cse6230-hw/final/hpl/hpl-2.3/bin/Linux/
### run example with 8 cores
cp HPL_example.dat HPL.dat
mpirun -np 28 ./xhpl

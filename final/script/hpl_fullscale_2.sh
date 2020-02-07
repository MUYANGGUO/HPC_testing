#!/bin/sh

#PBS -N hpl_fullscale
### request four node with 112 cores
#PBS -l nodes=4:ppn=28,walltime=00:30:00
#PBS -q coc-ice-multi

module load cse6230/core
module load mkl/19.0

cd "$HOME"/cse6230-hw/final/hpl/hpl-2.3/bin/Linux/
### run example with 112 cores
cp HPL_fullscale2.dat HPL.dat
mpirun -np 112 ./xhpl | tee "$HOME"/cse6230-hw/final/fullscale.txt


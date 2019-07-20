#!/bin/bash

#SBATCH -J mpi_pi
#SBATCH -p cpu
#SBATCH --nodes=4
#SBATCH --mail-user=xuehzh95@qq.com
#SBATCH --mail-type=ALL
#SBATCH --ntasks-per-node=16
#SBATCH --output=%J.out
#SBATCH --error=%J.err

module load mpich/3.2
mpicxx -o ./pi pi.cpp
time mpirun ./pi 1000 > mpi_4nodes_1000.txt
time mpirun ./pi 10000 > mpi_4nodes_10000.txt
time mpirun ./pi 100000 > mpi_4nodes_100000.txt
time mpirun ./pi 1000000 > mpi_4nodes_1000000.txt
time mpirun ./pi 10000000 > mpi_4nodes_10000000.txt
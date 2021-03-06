#!/bin/bash
#SBATCH --partition=cpsc424_gpu
# set total number of MPI processes
#SBATCH --ntasks=8
# set number of MPI processes per node
# (number of nodes is calculated by Slurm)
#SBATCH --ntasks-per-node=8
# set number of cpus per MPI process
#SBATCH --cpus-per-task=1
# set memory per cpu
#SBATCH --mem-per-cpu=6100mb
#SBATCH --job-name=MPI_RUN
#SBATCH --time=15:00

module load Langs/Intel/15 MPI/OpenMPI/2.1.1-intel15
pwd
# echo some environment variables
echo $SLURM_JOB_NODELIST
echo $SLURM_NTASKS_PER_NODE
# Do a clean build
make clean
# My MPI program is task2
make
# The following mpirun command will pick up required info on nodes and cpus from Slurm.
# You can use mpirun's -n option to reduce the number of MPI processes started on the cpus. (At most 1 MPI proc per Slurm task.)
# You can use mpirun options to control the layout of MPI processes---e.g., to spread processes out onto multiple nodes
# In this example, we've asked Slurm for 4 tasks (2 each on 2 nodes), but we've asked mpirun for two MPI procs, which will go onto 1 node.
# (If "-n 2" is omitted, you'll get 4 MPI procs (1 per Slurm task)
export nodes=8192
export edges=741455
time mpirun -n 1 ./parallel_dijkstra $nodes $edges 10
time mpirun -n 2 ./parallel_dijkstra $nodes $edges 10
time mpirun -n 4 ./parallel_dijkstra $nodes $edges 10
time mpirun -n 8 ./parallel_dijkstra $nodes $edges 10

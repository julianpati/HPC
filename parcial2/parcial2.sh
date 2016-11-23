#!/bin/bash
#
#SBATCH --job-name=parcial2
#SBATCH --output=parcial2.out
#SBATCH --nodes=2
#SBATCH --tasks=2
#SBATCH --gres=gpu:1

echo $CUDA_VISIBLE_DEVICES
mpirun parcial2

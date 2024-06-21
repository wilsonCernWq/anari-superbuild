#!/bin/bash -l

# Swap NVHPC with GNU compilers
module swap PrgEnv-nvhpc PrgEnv-gnu
module load nvhpc-mixed

export CC=$(which cc)
export CXX=$(which CC)
export FTN=$(which ftn)

module load cray-hdf5-parallel
module load libfabric

export CRAY_ACCEL_TARGET=nvidia80

export HTTP_PROXY="http://proxy.alcf.anl.gov:3128"
export HTTPS_PROXY="http://proxy.alcf.anl.gov:3128"
export http_proxy="http://proxy.alcf.anl.gov:3128"
export https_proxy="http://proxy.alcf.anl.gov:3128"
export ftp_proxy="http://proxy.alcf.anl.gov:3128"
export no_proxy="admin,polaris-adminvm-01,localhost,*.cm.polaris.alcf.anl.gov,polaris-*,*.polaris.alcf.anl.gov,*.alcf.anl.gov"

script_dir=$(dirname "${BASH_SOURCE[0]}")
env enable_mpi=ON enable_fortran=ON raja_enable_vectorization=OFF enable_tests=OFF build_ascent=false ${script_dir}/build_ascent_cuda.sh

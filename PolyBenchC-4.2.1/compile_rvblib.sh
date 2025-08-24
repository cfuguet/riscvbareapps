#!/bin/bash

benchs=(
linear-algebra/blas/gemm
linear-algebra/blas/gemver
linear-algebra/blas/gesummv
linear-algebra/blas/symm
linear-algebra/blas/syr2k
linear-algebra/blas/syrk
linear-algebra/blas/trmm
linear-algebra/kernels/2mm
linear-algebra/kernels/3mm
linear-algebra/kernels/atax
linear-algebra/kernels/bicg
linear-algebra/kernels/doitgen
linear-algebra/kernels/mvt
linear-algebra/solvers/cholesky
linear-algebra/solvers/durbin
linear-algebra/solvers/gramschmidt
linear-algebra/solvers/lu
linear-algebra/solvers/ludcmp
linear-algebra/solvers/trisolv
stencils/adi
stencils/fdtd-2d
stencils/heat-3d
stencils/jacobi-1d
stencils/jacobi-2d
stencils/seidel-2d
medley/deriche
medley/floyd-warshall
medley/nussinov
datamining/correlation
datamining/covariance
)

clean=0
while [[ $# -gt 0 ]] ; do
  case $1 in
    -c) clean=1 ; shift ;;
    *) shift ;;
  esac
done

if [[ ${clean} -eq 1 ]] ; then
  echo "info: cleaning up"
  rm -rf build
  exit 0
fi

DEFS="-DRVBLIB"
DEFS="${DEFS} -DPOLYBENCH_TIME -DPOLYBENCH_CYCLE_ACCURATE_TIMER"
DEFS="${DEFS} -DPOLYBENCH_CACHE_SIZE_KB=128"
DEFS="${DEFS} -DSMALL_DATASET"
EXTRA_CFLAGS="-O3 ${DEFS}"
for b in ${benchs[@]} ; do
  echo "info: compiling $b"
  TARGET=$(basename $b)
  OBJS="build/utilities/polybench.o build/$b/${TARGET}.o"
  make -f ${RVB_BUILD}/makefile.include \
    EXTRA_CFLAGS="${EXTRA_CFLAGS}" \
    EXTRA_INCLUDES="-Iutilities -I$b" \
    EXTRA_LIBS="-lm" \
    OBJS="${OBJS}" \
    TARGET=$b/${TARGET} \
    all

  exit_status=$?
  if [[ ${exit_status} -ne 0 ]] ; then
    echo "error: while compiling $b"
    exit 1;
  fi
done

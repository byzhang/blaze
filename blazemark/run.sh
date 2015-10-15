#!/bin/bash
for benchmark in bin/*; do
  OMP_NUM_THREADS=8 $benchmark -only-blaze | tee -a bench-avx.log
done

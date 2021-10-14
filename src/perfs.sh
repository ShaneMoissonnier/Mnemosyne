#!/bin/bash
# bash program to show the running time execution of the allocator
declare -a tests=("test_base" "test_init" "test_base_bzero" "test_cheese" "test_fusion" "test_frag" "test_fusion_bzero" "test_cheese_bzero" "test_unit" )

ts=$(date +%s%N);

for test in ${tests[@]}; do
   ./$test
done; 
tt=$((($(date +%s%N) - $ts)/1000000)) ; echo "Elapsed time: $tt milliseconds"
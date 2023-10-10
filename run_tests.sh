#!/bin/bash

echo "runing all tests:"
for i in $(ls -1 tests/ | sed -e 's/\.cmm$//')
do
  printf "\t$i: "
  mkdir -p test-results
  ./parser ./tests/$i.cmm ./test-results/$i.ir
  if [ $? -eq 0 ]; 
  then 
    printf "success\n"
  fi
done

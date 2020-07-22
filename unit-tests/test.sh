#!/bin/bash

dirs="code-pattern data-sync"
if [ $# -gt 0 ]; then
  dirs=$@
fi

### check installation of pLiner
clangdir=`clang -print-resource-dir`
if [ ! -d $clangdir ]; then
  echo -e "\e[31mError: clang not found\e[0m"
  exit -1
fi
mypLiner=`which pLiner`
if [ ! -x $mypLiner ]; then
  echo -e "\e[31mError: pLiner not found\e[0m"
  exit -1
fi
echo -e "pLiner - \e[32minstalled\e[0m"

compflags="-I $clangdir/include"

clean(){
  rm -f func-list.json pLiner-*.json *_ori *_trans
}

whole_transformation(){
  test=$1
  pLiner $test.c --po -- $compflags &>/dev/null
  if [ $? -ne 0 ]; then
    echo -e "\e[31mError: $test.c -- print function failed\e[0m"
    clean
    return 1
  fi
  pLiner $test.c -r func-list.json --whole -o ${test}_wholetrans.c &>/dev/null -- $compflags
  if [ $? -ne 0 ]; then
    echo -e "\e[31mError: $test.c -- print function failed\e[0m"
    clean
    return 2
  fi
  clean
  return 0
}

compile_and_compare(){
  ori=$1
  trans=$2
  test=`echo $ori | cut -d '.' -f 1`

  clang -o test_ori $compflags $ori &>/dev/null
  clang -o test_trans $compflags $trans &>/dev/null

  if [ $? -ne 0 ]; then
    echo -e "\e[31mError: $trans -- compilation failed\e[0m"
    clang -o test_trans $compflags $trans
    clean
    return 1
  fi
  
  ./test_ori > result_ori 2>/dev/null
  ./test_trans > result_trans 2>/dev/null

  if [ $? -ne 0 ]; then
    echo -e "\e[31mError: $trans -- execution failed\e[0m"
    ./test_trans
    clean
    return 2
  fi

  dif=`diff result_ori result_trans`
  
   if [ "$dif" != "" ]; then
    echo -e "\e[31mError: $ori and $trans -- comparison failed (inconsistent result)\e[0m"
    echo $dif
    clean
    return 3
  fi

  clean
  return 0
}


### test function transformation
fail=0
pass=0
echo -e "\ntesting pLiner - function transformation"
for dir in $dirs; do
  echo "  $dir"
  cd $dir
  tests=`ls test_[0-9].c`
  for test in $tests; do
    test=`echo $test | cut -d '.' -f 1`
    echo "    $test"
    whole_transformation $test
    if [ $? -eq 0 ]; then
      echo -e "      - \e[32mtransformation completed\e[0m"
    else 
      echo -e "      - \e[31mtransformation failed\e[0m"
    fi

    compile_and_compare $test.c ${test}_wholetrans.c  
    if [ $? -eq 0 ]; then
      echo -e "      - \e[32mcompile&compare completed\e[0m"
      pass=$((pass+1))
    else 
      echo -e "      - \e[31mcompile&compare failed\e[0m"
      fail=$((fail+1))
    fi
  done
  rm *_wholetrans.c
  cd ..
done
echo -e "\n  $fail tests failed; $pass tests pass"

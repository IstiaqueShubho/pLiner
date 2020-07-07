# pLiner

**pLiner** is a framework that helps programmers identify locations in the source of numerical code that are highly affected by compiler optimizations.  

Compiler optimizations can alter significantly the numerical results of scientific computing applications. When numerical results differ significantly between compilers, optimization levels, and floating-point hardware, these numerical inconsistencies can impact programming productivity. **pLiner** is a framework that helps programmers identify locations in the source code that are highly affected by compiler optimizations. **pLiner** uses a novel approach to identify such code locations by enhancing the floating-point precision of variables and expressions. Using a guided search to locate the most significant code regions, **pLiner** can report to users such locations at different granularities, file, function, and line of code.

# Getting Started

## Requirements to use pLiner
- pLiner is implemented as a clang tool. Installing clang/LLVM compiler is a prerequisite to use pLiner. So far, we have tested pLiner on clang/LLVM 9.0.1. Please follow the instructions below for building and installing clang/LLVM.
- pLiner uses [nlohmann::json](https://github.com/nlohmann/json) to parse json files in C/C++. Download file `json.hpp` from [https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp](https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp) (version 3.5.0) and place it in the directory `pLiner-sc20/clang-tool` before using pLiner.
- So far pLiner only supports C/C++.

## Building clang/LLVM and pLiner
  1. Building clang/LLVM 9.0.1:
  ```
  git clone https://github.com/llvm/llvm-project.git clang-llvm
  git checkout llvmorg-9.0.1
  cd ~/clang-llvm
  mkdir build && cd build
  cmake -G Ninja ../llvm -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_BUILD_TESTS=ON
  ninja
  ninja check       # Test LLVM only.
  ninja clang-test  # Test Clang only.
  ninja install
  ```

  Note: Refer to https://clang.llvm.org/docs/LibASTMatchersTutorial.html in case you need instructions for installing `cmake` and/or `ninja`.
  
  2. Clone pLiner in the clang-tools-extra directory and build it:
  ```
  cd ../clang-tools-extra
  git clone https://github.com/ucd-plse/pLiner-sc20.git
  echo "add_subdirectory(pLiner-sc20/clang-tool)" >> CMakeLists.txt
  cd ../build
  ninja
  ```
  3. Export path to pLiner (this command may differ depending on shell):
  ```
  export PATH=$PATH-TO-CLANG-LLVM/build/bin:$PATH
  ```

## Using pLiner

### Example

We use a simple C program `vtest.c` to show how to use pLiner. This program was generated by a floating-point random program generator, [Varity](https://www.osti.gov/biblio/1581779-varit), and it produces inconsistent results when compiled with `gcc -O3 -ffast-math` compared to `gcc -O0`. pLiner isolated a line of the code in the source (Line 25) as the origin of the compiler-induced inconsistency. pLiner also provided a transformed version of the program `vtest_trans.c`, in which the isolated line of the code has been transformed to higher precision, and the transformed program produces consistent results between `gcc -O3 -ffast-math` and `gcc -O0`.

  1. Change to the `example` directory:
  ```
  cd ../clang-tools-extra/pLiner-sc20/example
  ```
  
  2. Compile the original `vtest.c` program with both `gcc -O3 -ffast-math` and `gcc -O0`, and compare the results:
  ```
  make
  ./cmp.sh vtest
  ```
   
  `vtest_O0` corresponds to the executable generated by compiling `vtest.c` with `gcc -O0`, and `vtest_O3` corresponds to the executable generated by compiling `vtest.c` with `gcc -O3 -ffast-math`.  
   
 The expected output when comparing the results should be:
 
 ```
 ./vtest_O0 +1.8768E-306 5 -1.3896E-307 +1.2460E-307 -1.4722E306 -0.0 -1.7470E-322 +1.7072E-307 -1.9009E-307 +1.6022E137 +1.3969E306 +1.8813E34 -1.9422E-99 -1.2666E305 +0.0 -1.2316E-314 +1.5805E-323 +1.7072E208 +1.9220E-307 +1.6811E-306
1.7071999999999999e+208
./vtest_O3 +1.8768E-306 5 -1.3896E-307 +1.2460E-307 -1.4722E306 -0.0 -1.7470E-322 +1.7072E-307 -1.9009E-307 +1.6022E137 +1.3969E306 +1.8813E34 -1.9422E-99 -1.2666E305 +0.0 -1.2316E-314 +1.5805E-323 +1.7072E208 +1.9220E-307 +1.6811E-306
-1.8508999968058596e-316
 ```
 
 Note that the difference between `vtest_O0` and `vtest_O3` is very large. We will use pLiner to diagnose the root cause of the numerical difference.  

  3. Run pLiner:
  ```
  python ../scripts/search.py vtest.c "--"
  ```
  
  The first argument `vtest.c` is the input program; the second argument `"--"` indicates that to compile the input program there are no header files/librares to specify in the compilation command. Additionally, if there are any such compilation options such as "-I $PATH-TO-HEADERS", specify them following "--" in the second argument, e.g., "-- -I $PATH-TO-HEADERS". Use `--help` to check for the details of the arguments.

  Following is the output.
  ```
  ...
  The following areas are transformed to high precision:
compute :
   20 -> 22
   23 -> 23
failed
The following areas are transformed to high precision:
compute :
   25 -> 25
   26 -> 26
   28 -> 28
success
The following areas are transformed to high precision:
compute :
   25 -> 25
success
Search for lines:
The following areas are transformed to high precision:
compute :
   25 -> 25
success


 Bug area:
compute :
  line  25

  ```


   * pLiner found the root cause of the inconsistency (function `compute`, line 25):  
    ` compute :`  
    `    line 25`    

   * pLiner generated a transformed program `vtest_trans.c`.
   ```
   ls *.c
   vtest.c vtest_trans.c
   ```
   
   4. Compile the transformed `vtest_trans.c` program with both `gcc -O3 -ffast-math` and `gcc -O0`, and compare the results:
   ```
   make
   ./cmp.sh vtest_trans
   ```
The expected output when comparing the results should be:
```
./vtest_trans_O0 +1.8768E-306 5 -1.3896E-307 +1.2460E-307 -1.4722E306 -0.0 -1.7470E-322 +1.7072E-307 -1.9009E-307 +1.6022E137 +1.3969E306 +1.8813E34 -1.9422E-99 -1.2666E305 +0.0 -1.2316E-314 +1.5805E-323 +1.7072E208 +1.9220E-307 +1.6811E-306
1.7071999999999999e+208
./vtest_trans_O3 +1.8768E-306 5 -1.3896E-307 +1.2460E-307 -1.4722E306 -0.0 -1.7470E-322 +1.7072E-307 -1.9009E-307 +1.6022E137 +1.3969E306 +1.8813E34 -1.9422E-99 -1.2666E305 +0.0 -1.2316E-314 +1.5805E-323 +1.7072E208 +1.9220E-307 +1.6811E-306
1.7071999999999999e+208
```

Both `vtest_trans_O0` and `vtest_trans_O3` produce `1.7071999999999999e+208`. The results are consistent with `vtest_O0`.

### Use pLiner for your programs

You can follow the instructions as shown in the example to run pLiner for your own programs. Specifically, 

* Specify the compiler, compilation options that induce inconsistent results, and an error threshold in `run.sh`  
 In the example above, those are specified in `run.sh` as
 ```
 CC="/usr/bin/gcc"
 CFLAGS=" -O0 -g -std=c99"
 CFLAGS_trouble=" -O3 -ffast-math -g -std=c99"
 THRESHOLD=8 
 ```
 `CC` specifies the compiler; `CFLAGS` specifies the compilation options that are used to produce ground-truth results and `CFLAGS_trouble` specifies the compilation options that induce inconsistent results; lastly, `THRESHOLD` specifies the number of digits that are required to be same with the ground truth for consistency check.
 
* Specify your program file in the first argument, and the compilation options needed to compile the program in the second argument such as 
`python pLiner/scripts/search.py vtest.c "--"` in the example.

 ## License

pLiner is distributed under the terms of the Apache-2.0 with LLVM-exception license. All new contributions must be made under this license.
 
 See LICENSE and NOTICE for details.
 
 LLNL-CODE-812209

# Timing-Analyzer-with-False-Path-Detection

## How to Compile?
```shell
$ ls
Ans       Makefile  README.md lib       src       testdata
$ make
g++ -std=c++11 -g -Wall -c -o .out/Graph.o src/Graph.cpp
g++ -std=c++11 -g -Wall -c -o .out/LibTable.o src/LibTable.cpp
g++ -std=c++11 -g -Wall -c -o .out/NetlistParser.o src/NetlistParser.cpp
g++ -std=c++11 -g -Wall -c -o .out/Types.o src/Types.cpp
g++ -std=c++11 -g -Wall -c -o .out/main.o src/main.cpp
g++ -std=c++11 -g -Wall -o 0856090.o .out/Graph.o .out/LibTable.o .out/NetlistParser.o .out/Types.o .out/main.o
$ ls
0856090.o Ans       Makefile  README.md Results   lib       src       testdata
```

## How to Run?
```shell
$ ./0856090.o
usage: ./0856090.o netlist_file [-h] [-p input.pat] [-l testlib.lib] 
$ ./0856090.o testdata/c17.v -p testdata/c17.pat -l lib/test_lib.lib
```
Or you could also use `make test` in simply
```shell
$ make test
./0856090.o testdata/c17.v -p testdata/c17.pat -l lib/test_lib.lib
./0856090.o testdata/c432.v -p testdata/c432.pat -l lib/test_lib.lib
./0856090.o testdata/example.v -p testdata/example.pat -l lib/test_lib.lib
diff Results/0856090_c17.txt Ans/c17.txt
2c2
< N3 -> n10 -> n12 -> N22
---
> N6 -> n10 -> n12 -> N22
13c13
< N3 -> n10 -> n12 -> N22
---
> N6 -> n10 -> n12 -> N22
diff Results/0856090_c432.txt Ans/c432.txt
diff Results/0856090_example.txt Ans/module.txt
```


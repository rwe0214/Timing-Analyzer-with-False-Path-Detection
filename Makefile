SRCDIR = src
OUTDIR ?= .out
INPUT_DIR = testdata
CC	= g++
CFLAGS	=-std=c++11 -g -Wall

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OUTDIR)/%.o, $(SRCS)) 
EXE = hw2.o
NETLIST1 = testdata/c17.v
NETLIST2 = testdata/c432.v
NETLIST3 = testdata/example.v
PATTERN1 = testdata/c17.pat
PATTERN2 = testdata/c432.pat
PATTERN3 = testdata/example.pat
ANS1 = Results/c17.txt
ANS2 = Results/c432.txt
ANS3 = Results/module.txt
CELLLIB = lib/test_lib.lib
OUTPUT1 = 0856090_c17.txt
OUTPUT2 = 0856090_c432.txt
OUTPUT3 = 0856090_example.txt

OUTPUTS = 0856090_*

all: $(EXE)

$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
	
SHELL_HACK := $(shell mkdir -p $(OUTDIR))

.PHONY: all clean run debug OUT

format:
	clang-format -i src/*.cpp src/*.h

test:
	./$(EXE) $(NETLIST1) -p $(PATTERN1) -l $(CELLLIB)
	./$(EXE) $(NETLIST2) -p $(PATTERN2) -l $(CELLLIB)
	./$(EXE) $(NETLIST3) -p $(PATTERN3) -l $(CELLLIB)
	@echo 'diff $(OUTPUT1) $(ANS1)'
	@diff $(OUTPUT1) $(ANS1) || true
	@echo 'diff $(OUTPUT2) $(ANS2)'
	@diff $(OUTPUT2) $(ANS2) || true
	@echo 'diff $(OUTPUT3) $(ANS3)'
	@diff $(OUTPUT3) $(ANS3)

debug:
	gdb -q --args $(EXE) $(NETLIST1) -p $(PATTERN1) -l $(CELLLIB)

clean:
	rm -f $(OUTDIR)/* ./$(EXE) ./$(OUTPUTS)
	rmdir ./$(OUTDIR)

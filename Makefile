.PHONY: all clean

all: cachegrind.out main.ll

clean:
	rm -f new-string cachegrind.out main.ll

new-string: main.c string_data.h template_string.h template_string_piece.h
	gcc -O0 -g main.c -o new-string

cachegrind.out: new-string
	valgrind --tool=cachegrind --cachegrind-out-file=./cachegrind.out ./new-string
	cg_annotate --show=Dr,D1mr,DLmr --sort=D1mr ./cachegrind.out

main.ll: main.c
	clang -S -emit-llvm -O3 main.c -o main.ll
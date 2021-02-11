.PHONY: all clean

all: cachegrind.out main.ll

clean:
	rm -f new-string cachegrind.out main.ll

new-string: main.c string_data.h segmented_string.h segmented_string_piece.h
	gcc -O0 -g main.c -o new-string

new-string.s: main.c string_data.h segmented_string.h segmented_string_piece.h
	gcc -O0 -S -fverbose-asm main.c -o new-string.s

new-string.asm-annotated: new-string
	objdump -d -S new-string > new-string.asm-annotated

cachegrind.out: new-string
	valgrind --tool=cachegrind --cachegrind-out-file=./cachegrind.out ./new-string
	cg_annotate --show=Dr,D1mr,DLmr --sort=D1mr ./cachegrind.out

main.ll: main.c
	clang -S -emit-llvm -O3 main.c -o main.ll
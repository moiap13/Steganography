GCC=gcc -g -Wall -Wextra -std=gnu11 
LIBS=-lm -lpthread

encode: encode.o encode_lib.o ppm.o alloc.o files.o
	$(GCC) $^ -o $@ $(LIBS)
encode.o: encode.c
	$(GCC) $< -c
encode_lib.o: encode_lib.c encode_lib.h
	$(GCC) $< -c
ppm.o: ../libs/ppm.c ../libs/ppm.h
	$(GCC) $< -c
alloc.o: ../libs/alloc.c ../libs/alloc.h
	$(GCC) $< -c
files.o: ../libs/files.c ../libs/files.h
	$(GCC) $< -c
run: encode
	./encode
clean:
	rm -f *.o encode; clear

rebuild: 
	clean encode

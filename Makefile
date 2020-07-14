CC=gcc
CFLAGS="-Wall"

debug:clean
	$(CC) $(CFLAGS) -g -o test1 main.c `pkg-config sdl2 --libs --cflags`
stable:clean
	$(CC) $(CFLAGS) -o test1 main.c `pkg-config sdl2 --libs --cflags`
clean:
	rm -vfr *~ test1
run:stable
	./test1

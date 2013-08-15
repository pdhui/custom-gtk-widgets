all: test-scrolling

test-scrolling: main.c
	gcc -o test-scrolling main.c gb-animation.c gb-frame-source.c $(shell pkg-config --cflags --libs gtksourceview-3.0)

clean:
	rm -f test-scrolling

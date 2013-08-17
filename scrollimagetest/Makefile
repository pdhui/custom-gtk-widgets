all: scrollimagetest

scrollimagetest: *.c *.h
	$(CC) -o $@ -Wall *.c $(shell pkg-config --cflags --libs gtk+-3.0)

clean:
	rm -f scrollimagetest

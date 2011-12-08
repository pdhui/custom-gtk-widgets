all: gb-scrolled-window

OBJECTS =
OBJECTS += gb-animation.o
OBJECTS += gb-timeout-interval.o
OBJECTS += gb-frame-source.o
OBJECTS += gb-scrolled-window.o
OBJECTS += main.o

PKGS =
PKGS += gtk+-3.0

%.o: %.c
	$(CC) -g -Wall -Werror -o $@ -c $^ $(shell pkg-config --cflags $(PKGS))

gb-scrolled-window: $(OBJECTS)
	$(CC) -g -Wall -Werror -o $@ $(OBJECTS) $(shell pkg-config --libs $(PKGS))

clean:
	rm -f *.o gb-scrolled-window

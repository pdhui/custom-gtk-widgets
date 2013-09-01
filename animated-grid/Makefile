all: simply-chat

FILES =
FILES += chat-avatar.c
FILES += chat-avatar.h
FILES += chat-grid.c
FILES += chat-grid.h
FILES += gb-animation.c
FILES += gb-animation.h
FILES += gb-frame-source.c
FILES += gb-frame-source.h
FILES += main.c

simply-chat: $(FILES) Makefile
	$(CC) -o $@ -g $(shell pkg-config --cflags --libs gtk+-3.0) $(FILES)


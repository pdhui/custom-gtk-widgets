all: simply-chat

FILES =
FILES += chat-avatar.c
FILES += chat-avatar.h
FILES += chat-grid.c
FILES += chat-grid.h
FILES += ppg-animation.c
FILES += ppg-animation.h
FILES += ppg-frame-source.c
FILES += ppg-frame-source.h
FILES += ppg-timeout-interval.c
FILES += ppg-timeout-interval.h
FILES += main.c

simply-chat: $(FILES) Makefile
	$(CC) -o $@ -g $(shell pkg-config --cflags --libs gtk+-3.0) $(FILES)


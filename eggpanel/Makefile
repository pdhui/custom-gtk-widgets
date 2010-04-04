all: eggpanel

SOURCES = main.c egg-panel.c
HEADERS = egg-panel.h
WARNINGS =								\
	-Wall								\
	-Werror								\
	-Wold-style-definition						\
	-Wdeclaration-after-statement					\
	-Wredundant-decls						\
	-Wmissing-noreturn						\
	-Wshadow							\
	-Wcast-align							\
	-Wwrite-strings							\
	-Winline							\
	-Wformat-nonliteral						\
	-Wformat-security						\
	-Wswitch-enum							\
	-Wswitch-default						\
	-Winit-self							\
	-Wmissing-include-dirs						\
	-Wundef								\
	-Waggregate-return						\
	-Wmissing-format-attribute					\
	-Wnested-externs

eggpanel: $(SOURCES) $(HEADERS)
	$(CC) -o $@ -g $(WARNINGS) $(SOURCES) $(shell pkg-config --libs --cflags gtk+-2.0)

clean:
	rm -rf eggpanel

test: eggpanel
	./eggpanel

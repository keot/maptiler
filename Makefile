CC=gcc
CFLAGS=-I. -std=c99
LIBS=-lvlc -lconfig -lSDL -lSDL_ttf -lSDL_image
DEPS=configuration.h experiment.h movie.h strings.h tiles.h constants.h eyeserver.h sdl_helper.h spinner.h text.h
OBJS=configuration.o experiment.o movie.o tiles.o eyeserver.o sdl_helper.o spinner.o text.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

maptiler: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~

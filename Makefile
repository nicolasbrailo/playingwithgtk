CFLAGS=-Wall -Wextra -Wc++0x-compat -pedantic -ggdb -std=c++0x
GTK_LD=`pkg-config --libs gtk+-2.0`
GTK_I=`pkg-config --cflags gtk+-2.0`
MAGICPP_I=`pkg-config --cflags Magick++`
MAGICPP_LD=`pkg-config --libs Magick++`
OBJ_FILES=main.o image_cache.o

all: app

%.o: %.cpp
	g++ -c $(CFLAGS) $(GTK_I) $(MAGICPP_I) $< -o$@

app: $(OBJ_FILES)
	g++ $(OBJ_FILES) -o app $(GTK_LD) $(MAGICPP_LD)

.PHONY: clean run

run: app
	./app

clean:
	rm -f $(OBJ_FILES)



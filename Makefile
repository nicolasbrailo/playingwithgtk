CFLAGS=-Wall -Wextra -Wc++0x-compat -pedantic -ggdb -std=c++0x
LDFLAGS=-lpthread
GTK_LD=`pkg-config --libs gtk+-2.0`
GTK_I=`pkg-config --cflags gtk+-2.0`
MAGICPP_I=`pkg-config --cflags Magick++`
MAGICPP_LD=`pkg-config --libs Magick++`

BUILD_DIR=build/
CPP_FILES=$(shell find . -type f -name \*.cpp | sed 's/^\.\///g')
OBJ_FILES=$(addprefix $(BUILD_DIR), $(patsubst %.cpp,%.o,$(CPP_FILES)) )
DEPS_FILE=build/deps

all: $(DEPS_FILE) app

-include $(DEPS_FILE)

$(BUILD_DIR)%.o: %.cpp
	$(shell mkdir -p `dirname $@`)
	g++ -c $(CFLAGS) $(GTK_I) $(MAGICPP_I) $< -o$@

app: $(OBJ_FILES)
	g++ $(OBJ_FILES) -o app $(GTK_LD) $(MAGICPP_LD) $(LDFLAGS)

# Build a deps file with makefile format dependencies of all the cpp files
# Including it will cause each .o to be recompiled if a header was changed
$(DEPS_FILE):
	@# This will never be rebuilt; to rebuild deps a make clean is needed
	@# otherwise we'll have to scan all the files in the projects one more
	@# extra time.

	$(shell mkdir -p `dirname $@`)

	@# pattern = '*.o: ' -> (in awk format) -> /\..: /
	@# awk 'if line matches pattern {print build/$line; break} else {print $line}'
	g++ -MM $(CPP_FILES) | \
		awk '$$0 ~ /\..: / {print "$(BUILD_DIR)"$$0; next} 1' > $@


.PHONY: clean run ctags

run: app
	./app

clean:
	rm -f $(OBJ_FILES) $(DEPS_FILE)

ctags:
	ctags -R -o ./ctags .


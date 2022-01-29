COMPILER = clang

BUILD_OPTIONS = -g3 -Wpedantic -I../vendor/include/ -D_DEBUG

SOURCE_LIBS = -I../vendor/sources/

OSX_OPT = -L../vendor/lib/macOS/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL ../vendor/lib/macOS/libraylib.a
OSX_OUT = -o "bin/build_osx"

CFILES = *.c

build_osx:
	$(COMPILER) $(BUILD_OPTIONS) $(CFILES) $(SOURCE_LIBS) $(OSX_OUT) $(OSX_OPT)



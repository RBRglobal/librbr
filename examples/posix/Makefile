CFLAGS := -I../../include \
          -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -Wwrite-strings \
          -std=c99 \
          -g
LDFLAGS := -L../../bin
LDLIBS := -lRBR

example: posix-download \
         posix-fetch \
         posix-parse-download \
         posix-parse-file \
         posix-postprocessing \
         posix-stream \
         posix-stream-sdl

posix-download: posix-shared.o posix-download.o ../../bin/libRBR.a

posix-fetch: posix-shared.o posix-fetch.o ../../bin/libRBR.a

posix-parse-download: posix-shared.o posix-parse-download.o ../../bin/libRBR.a

posix-parse-file: posix-shared.o posix-parse-file.o ../../bin/libRBR.a

posix-postprocessing: posix-shared.o posix-postprocessing.o ../../bin/libRBR.a

posix-stream: posix-shared.o posix-stream.o ../../bin/libRBR.a

posix-stream-sdl: LDLIBS += -lSDL2
posix-stream-sdl: posix-shared.o posix-stream-sdl.o ../../bin/libRBR.a

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		posix-download \
		posix-fetch \
		posix-parse-download \
		posix-parse-file \
		posix-postprocessing \
		posix-stream \
		posix-stream-sdl

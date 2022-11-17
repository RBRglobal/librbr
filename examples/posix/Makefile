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
LDLIBS := -lRBR -lm

all: exampledynamiccorrection example

example: posix-download \
         posix-download-wifi \
         posix-fetch \
         posix-parse-download \
         posix-parse-download-dataset \
         posix-parse-file \
         posix-parse-file-dynamiccorrection \
         posix-postprocessing \
         posix-pauseresume \
         posix-stream \
         posix-stream-sdl \

exampledynamiccorrection: posix-parse-file-dynamiccorrection \
           posix-stream-dynamiccorrection \

posix-download: posix-shared.o posix-download.o ../../bin/libRBR.a

posix-download-wifi: posix-shared.o posix-download-wifi.o ../../bin/libRBR.a

posix-fetch: posix-shared.o posix-fetch.o ../../bin/libRBR.a

posix-parse-download: posix-shared.o posix-parse-download.o ../../bin/libRBR.a

posix-parse-download-dataset: posix-shared.o posix-parse-download-dataset.o ../../bin/libRBR.a

posix-parse-file: posix-shared.o posix-parse-file.o ../../bin/libRBR.a

posix-parse-file-dynamiccorrection: posix-shared.o posix-parse-file-dynamiccorrection.o ../../bin/libRBR.a ../../bin/libRBRDynamicCorrection.a

posix-postprocessing: posix-shared.o posix-postprocessing.o ../../bin/libRBR.a

posix-pauseresume: posix-shared.o posix-pauseresume.o ../../bin/libRBR.a 

posix-stream: posix-shared.o posix-stream.o ../../bin/libRBR.a

posix-stream-dynamiccorrection: posix-shared.o posix-stream-dynamiccorrection.o ../../bin/libRBR.a ../../bin/libRBRDynamicCorrection.a

posix-stream-sdl: LDLIBS += -lSDL2
posix-stream-sdl: posix-shared.o posix-stream-sdl.o ../../bin/libRBR.a

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		posix-download \
		posix-download-wifi \
		posix-fetch \
		posix-parse-download \
        posix-parse-download-dataset \
		posix-parse-file \
		posix-parse-file-dynamiccorrection \
		posix-postprocessing \
        posix-pauseresume \
		posix-stream \
		posix-stream-dynamiccorrection \
		posix-stream-sdl

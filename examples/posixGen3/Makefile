CFLAGS := -I../../include \
          -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -Wdouble-promotion \
          -Wwrite-strings \
          -std=c99 \
          -g
LDFLAGS := -L../../bin
LDLIBS := -lRBR -lm

ALWAYS_EXAMPLES := \
    posix-download \
    posix-download-wifi \
    posix-fetch \
    posix-parse-download \
    posix-parse-download-dataset \
    posix-parse-file \
    posix-parse-file-dynamiccorrection \
    posix-pauseresume \
    posix-postprocessing \
    posix-stream

SDL_EXAMPLES := \
    posix-stream-sdl

DYNAMICCORRECTION_EXAMPLES := \
    posix-parse-file-dynamiccorrection \
    posix-stream-dynamiccorrection

all: $(ALWAYS_EXAMPLES) $(SDL_EXAMPLES) $(DYNAMICCORRECTION_EXAMPLES)

nosdl: $(ALWAYS_EXAMPLES) $(DYNAMICCORRECTION_EXAMPLES)

nomalloc: CFLAGS += -DRBR_LIB_NODYNAMICMEMORYALLOCATION
nomalloc: $(ALWAYS_EXAMPLES) $(DYNAMICCORRECTION_EXAMPLES)

posix-download: posix-shared.o posix-download.o ../../bin/libRBR.a

posix-download-wifi: posix-shared.o posix-download-wifi.o ../../bin/libRBR.a

posix-fetch: posix-shared.o posix-fetch.o ../../bin/libRBR.a

posix-parse-download: posix-shared.o posix-parse-download.o ../../bin/libRBR.a

posix-parse-download-dataset: posix-shared.o posix-parse-download-dataset.o ../../bin/libRBR.a

posix-parse-file: posix-shared.o posix-parse-file.o ../../bin/libRBR.a

posix-parse-file-dynamiccorrection: posix-shared.o posix-parse-file-dynamiccorrection.o ../../bin/libRBR.a ../../bin/libRBRDynamicCorrection.a

posix-pauseresume: posix-shared.o posix-pauseresume.o ../../bin/libRBR.a

posix-postprocessing: posix-shared.o posix-postprocessing.o ../../bin/libRBR.a

posix-stream: posix-shared.o posix-stream.o ../../bin/libRBR.a

posix-stream-dynamiccorrection: posix-shared.o posix-stream-dynamiccorrection.o ../../bin/libRBR.a ../../bin/libRBRDynamicCorrection.a

posix-stream-sdl: LDLIBS += -lSDL2
posix-stream-sdl: posix-shared.o posix-stream-sdl.o ../../bin/libRBR.a

../../bin/libRBR.a:
	$(MAKE) -C ../../ lib

../../bin/libRBRDynamicCorrection.a:
	$(MAKE) -C ../../ libdynamiccorrection

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		$(ALWAYS_EXAMPLES) \
		$(SDL_EXAMPLES) \
		$(DYNAMICCORRECTION_EXAMPLES)

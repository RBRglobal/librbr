CFLAGS := -I../../include \
          -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -Wwrite-strings \
          -std=c99 \
          -g 

CFLAGS += -Wno-sizeof-array-argument -Werror
CFLAGS += -Wno-sizeof-pointer-div -Werror

LDFLAGS := -L../../bin
LDLIBS := -lRBR -lm

example: posix-test

posix-download-datablock-bySchedule: posix-download-datablock-bySchedule.o posix-shared.o ../../bin/libRBR.a
posix-footprint: posix-footprint.o posix-shared.o ../../bin/libRBR.a
posix-generation: posix-generation.o posix-shared.o ../../bin/libRBR.a
posix-multiScheduleDiffConfig: posix-multiScheduleDiffConfig.o posix-shared.o ../../bin/libRBR.a
posix-poll: posix-poll.o posix-shared.o ../../bin/libRBR.a
posix-singleScheduleSingleConfig: posix-singleScheduleSingleConfig.o posix-shared.o ../../bin/libRBR.a
posix-stream: posix-stream.o posix-shared.o ../../bin/libRBR.a

posix-test: posix-test.o ../../bin/libRBR.a


.PHONY: clean
clean:
	rm -Rf \
		*.o \
        posix-download-datablock-bySchedule \
        posix-footprint \
        posix-generation \
        posix-multiScheduleDiffConfig \
        posix-poll \
        posix-singleScheduleSingleConfig \
        posix-stream \
        posix-parse-file \
        posix-test








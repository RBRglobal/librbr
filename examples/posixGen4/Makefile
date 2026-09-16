CFLAGS := -I../../include \
          -Werror \
          -Wall \
          -Wextra \
          -Wdouble-promotion \
          -pedantic \
          -pedantic-errors \
          -Wwrite-strings \
          -std=c99 \
          -g
CPPFLAGS += -MMD -MP
LDFLAGS := -L../../bin
LDLIBS := -lRBR -lm

## \brief Gen4 example programs.
EXAMPLES := \
    posix-communications \
    posix-download-datablock-bySchedule \
    posix-footprint \
    posix-generation \
    posix-multiScheduleDiffConfig \
    posix-poll \
    posix-singleScheduleSingleConfig \
    posix-stream

all: $(EXAMPLES)

posix-communications: posix-shared.o posix-communications.o ../../bin/libRBR.a

posix-download-datablock-bySchedule: posix-shared.o posix-download-datablock-bySchedule.o ../../bin/libRBR.a

posix-footprint: posix-shared.o posix-footprint.o ../../bin/libRBR.a

posix-generation: posix-shared.o posix-generation.o ../../bin/libRBR.a

posix-multiScheduleDiffConfig: posix-shared.o posix-multiScheduleDiffConfig.o ../../bin/libRBR.a

posix-poll: posix-shared.o posix-poll.o ../../bin/libRBR.a

posix-singleScheduleSingleConfig: posix-shared.o posix-singleScheduleSingleConfig.o ../../bin/libRBR.a

posix-stream: posix-shared.o posix-stream.o ../../bin/libRBR.a

../../bin/libRBR.a:
	$(MAKE) -C ../../ lib

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		*.d \
		$(EXAMPLES)

-include $(wildcard *.d)

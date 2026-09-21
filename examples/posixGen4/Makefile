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
    posix-download \
    posix-enable \
    posix-enable-multiconfig \
    posix-footprint \
    posix-generation \
    posix-poll \
    posix-stream

all: $(EXAMPLES)

posix-communications: posix-shared.o posix-communications.o ../../bin/libRBR.a

posix-download: posix-shared.o posix-download.o ../../bin/libRBR.a

posix-enable: posix-shared.o posix-enable.o ../../bin/libRBR.a

posix-enable-multiconfig: posix-shared.o posix-enable-multiconfig.o ../../bin/libRBR.a

posix-footprint: posix-shared.o posix-footprint.o ../../bin/libRBR.a

posix-generation: posix-shared.o posix-generation.o ../../bin/libRBR.a

posix-poll: posix-shared.o posix-poll.o ../../bin/libRBR.a

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

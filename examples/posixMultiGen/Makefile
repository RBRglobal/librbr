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

## \brief Example programs built against both generation APIs.
EXAMPLES := posix-detect

all: $(EXAMPLES)

posix-detect: posix-shared.o posix-detect.o ../../bin/libRBR.a

../../bin/libRBR.a:
	$(MAKE) -C ../../ lib

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		*.d \
		$(EXAMPLES)

-include $(wildcard *.d)

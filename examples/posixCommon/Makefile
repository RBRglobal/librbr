# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

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

## \brief Examples which use the Gen3 and Gen4 APIs together. They link
## against a libRBR.a built with both generations enabled (the default).
EXAMPLES := \
    posix-shared-buffers

all: $(EXAMPLES)

posix-shared-buffers: posix-shared-buffers.o ../../bin/libRBR.a

../../bin/libRBR.a:
	$(MAKE) -C ../../ lib

.PHONY: clean
clean:
	rm -Rf \
		$(EXAMPLES) \
		*.d \
		*.o

-include $(wildcard *.d)

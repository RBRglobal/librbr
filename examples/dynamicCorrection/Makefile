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
LDLIBS := -lm

example: dynamiccorrection-example 

dynamiccorrection-example: dynamiccorrection-example.o ../../bin/libRBRDynamicCorrection.a


.PHONY: clean
clean:
	rm -Rf \
		*.o \
		dynamiccorrection-example \


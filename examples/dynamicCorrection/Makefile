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
LDLIBS := -lm

example: dynamicCorrection-example 

dynamicCorrection-example: dynamicCorrection-example.o ../../bin/libRBRDynamicCorrection.a


.PHONY: clean
clean:
	rm -Rf \
		*.o \
		dynamicCorrection-example \


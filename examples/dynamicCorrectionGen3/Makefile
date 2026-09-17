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
CPPFLAGS += -MMD -MP
LDFLAGS := -L../../bin
LDLIBS := -lm

example: dynamicCorrection-example 

dynamicCorrection-example: dynamicCorrection-example.o ../../bin/libRBRDynamicCorrection.a

../../bin/libRBRDynamicCorrection.a:
	$(MAKE) -C ../../ libdynamiccorrection

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		*.d \
		dynamicCorrection-example \

-include $(wildcard *.d)

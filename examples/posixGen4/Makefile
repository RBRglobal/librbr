# Unlike the library and the Gen3 examples, these examples are not yet built
# with -Wdouble-promotion, and -Wformat-truncation is disabled: the example
# code predates those policies. Both relaxations should go away with the
# example alignment planned for libRBR 2.0 (SYS-1194).
CFLAGS := -I../../include \
          -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -Wwrite-strings \
          -Wno-format-truncation \
          -std=c99 \
          -g
LDFLAGS := -L../../bin
LDLIBS := -lRBR -lm

## \brief Gen4 example programs.
##
## Three examples are not listed and do not currently build; re-enabling them
## is part of the example alignment planned for libRBR 2.0 (SYS-1194):
##
## - posix-multiScheduleDiffConfig predates the late-2024 API alignment (it
##   still refers to the WiFi and old group/config pool interfaces);
## - posix-parse-file calls RBRParserGen4_destroy(), which is declared but
##   not implemented (the Gen4 sample parser is incomplete);
## - posix-profilingFloat configures a `regimes` schedule; the `regimes` and
##   `ddsampling` modes are not modelled (SYS-1234). It also uses download
##   and dataset interfaces which have since been reworked, so it needs
##   realignment beyond the regimes modelling.
EXAMPLES := \
    posix-communications \
    posix-download-datablock-bySchedule \
    posix-footprint \
    posix-generation \
    posix-poll \
    posix-singleScheduleSingleConfig \
    posix-stream \
    posix-test

all: $(EXAMPLES)

posix-communications: posix-shared.o posix-communications.o ../../bin/libRBR.a

posix-download-datablock-bySchedule: posix-shared.o posix-download-datablock-bySchedule.o ../../bin/libRBR.a

posix-footprint: posix-shared.o posix-footprint.o ../../bin/libRBR.a

posix-generation: posix-shared.o posix-generation.o ../../bin/libRBR.a

posix-poll: posix-shared.o posix-poll.o ../../bin/libRBR.a

posix-profilingFloat: posix-shared.o posix-profilingFloat.o ../../bin/libRBR.a

posix-singleScheduleSingleConfig: posix-shared.o posix-singleScheduleSingleConfig.o ../../bin/libRBR.a

posix-stream: posix-shared.o posix-stream.o ../../bin/libRBR.a

posix-test: posix-test.o ../../bin/libRBR.a

../../bin/libRBR.a:
	$(MAKE) -C ../../ lib

.PHONY: clean
clean:
	rm -Rf \
		*.o \
		$(EXAMPLES)

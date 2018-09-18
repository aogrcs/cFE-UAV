# frama-c-path.mk contains variables which are specific to each
# user and should not be versioned, such as the path to the
# frama-c binaries (e.g. FRAMAC and FRAMAC_GUI).
# It is an optional include, unnecessary if frama-c is in the PATH.
-include frama-c-path.mk

FRAMAC_CONFIG ?= frama-c-config
include $(shell $(FRAMAC_CONFIG) -print-share-path)/analysis-scripts/frama-c.mk

# Define global parameters
CPPFLAGS     = -I /home/talos/cFE/osal/src/os/inc -I /home/talos/cFE/cfe/fsw/cfe-core/src/inc -I /home/talos/cFE/psp/fsw/inc -I /home/talos/cFE/psp/fsw/pc-linux/inc -I /home/talos/cFE/apps/inc -I /home/talos/cFE/apps/hello/fsw/src -I /home/talos/cFE/build/mission_inc -I /home/talos/cFE/build/cpu1/inc
FCFLAGS     += -machdep gcc_x86_64 -main helloMain
EVAFLAGS    += -slevel 10 -eva-equality-domain -val-split-return full
VALBUILTINS +=

# Export environment variable for Frama-C
export FRAMA_C_MEMORY_FOOTPRINT = 8

TARGETS=hello
all: $(TARGETS:%=%.eva)
help:
	@echo "targets: $(TARGETS)"
loop: $(TARGETS:%=%.parse.loop) $(TARGETS:%=%.eva.loop)
parse: $(TARGETS:%=%.parse)
stats: $(TARGETS:%=%.parse) $(TARGETS:%=%.eva)
	../show_stats.sh "$(notdir $(CURDIR))" $^

hello.parse: $(sort $(wildcard *.c))

# hello.eva: \
  # EVAFLAGS += $(shell cat hello.slevel | tr '\\' ' ')

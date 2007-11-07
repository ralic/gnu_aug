ifndef TOP_SRCDIR
$(error please define TOP_SRCDIR)
endif

CFLAGS = -I$(TOP_SRCDIR)/src/c
CXXFLAGS = $(CFLAGS) -I$(TOP_SRCDIR)/src/cpp

CXXMODULES = daugintro

daugintro_OBJS = daugintro.o

all: all-aug

clean: clean-aug

include $(TOP_SRCDIR)/etc/aug.mk

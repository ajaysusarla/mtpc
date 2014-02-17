VERSION=0.1
PROGRAM=mtpc

## Silent by default
V =
ifeq ($(strip $(V)),)
        E = @echo
        Q = @
else
        E = @\#
        Q =
endif
export E Q

CC=gcc
DEBUG =-ggdb
CFLAGS=-Wall -Wno-pointer-sign $(DEBUG)
INSTALL=install
PKGCONFIG=pkg-config
GLIB_GENMARSHAL=/usr/bin/glib-genmarshal
NULL=

# Locations
# Change 'prefix' variable to suit your needs
RUN_IN_PLACE=1
EXTRA_FLAGS =
prefix=/opt/partha/mtpc
BINDIR=$(prefix)/bin
DATADIR=$(prefix)/share
UIDIR=$(prefix)/ui
ICONDIR=$(prefix)/icons
INCLUDEDIR =
LIBDIR = lib
SRCDIR = src
CWD =  $(shell pwd)

# Resources
ifeq ($(RUN_IN_PLACE), 1)
ui_dir = $(CWD)/ui
icon_dir = $(CWD)/icons
EXTRA_FLAGS += -DRUN_IN_PLACE
else
ui_dir = $(UIDIR)
icon_dir = $(ICONDIR)
endif


# Dependencies
LIBMTP = $(shell $(PKGCONFIG) --libs libmtp)
LIBGTK = $(shell $(PKGCONFIG) --libs gtk+-3.0 glib-2.0)
LIBGTHREAD = $(shell $(PKGCONFIG) --libs gthread-2.0)

LIBMTPFLAGS = $(shell $(PKGCONFIG) --cflags libmtp)
GLIB2CFLAGS = $(shell $(PKGCONFIG) --cflags glib-2.0)
GTKCFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0)
GTHREADCFLAGS = $(shell $(PKGCONFIG) --cflags gthread-2.0)


ifeq ($(UNAME), linux)
	OSSUPPORT = linux
	OSSUPPORT_CFLAGS = $(GTKCFLAGS)
endif

LIBS = $(LIBMTP) $(LIBGTK) $(LIBGTHREAD)
LDFLAGS =

EXTRA_FLAGS +=	$(GTKCFLAGS) $(GLIB2CFLAGS) $(GTHREADCFLAGS) \
		$(LIBMTPCFLAGS) -DVERSION_STRING='"$(VERSION)"' \
		-DMTPC_UI_DIR='"$(ui_dir)"' -DMTPC_ICON_DIR='"$(icon_dir)"'

# Platform
UNAME := $(shell $(CC) -dumpmachine 2>&1 | grep -E -o "linux|darwin")

# Object files
OBJS =  main.o \
	mtpc-app.o \
	gtk-utils.o \
	glib-utils.o \
	mtpc-window.o \
	mtpc-actions-callbacks.o \
	$(NULL)

# Headers
HDRS = 	main.h \
	mtpc-app.h \
	gtk-utils.h \
	glib-utils.h \
	mtpc-window.h \
	mtpc-actions-callbacks.h \
	$(NULL)

DEPS = $(wildcard .dep/*.dep)

$(PROGRAM): $(OBJS) $(HDRS)
	$(E) '             LD' $@
	$(Q)$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

%.o: %.c
	$(E) '             CC' $<
	$(Q)mkdir -p .dep
	$(Q)$(CC) $(CFLAGS) $(EXTRA_FLAGS) -MD -MF .dep/$@.dep -c -o $@ $<

mtpc-marshal.o: mtpc-marshal.c mtpc-marshal.h

mtpc-marshal.h:
	$(E) 'GLIB_GENMARSHAL' $<
	$(Q)$(GLIB_GENMARSHAL) mtpc-marshal.list --header --prefix=mtpc_marshal > $@

mtpc-marshal.c:
	$(E) 'GLIB_GENMARSHAL' $<
	$(E) "#include \"mtpc-marshal.h\"" > $@
	$(Q)$(GLIB_GENMARSHAL) mtpc-marshal.list --body --prefix=mtpc_marshal >> $@

clean:
	$(E) '   RM $(OBJS) $(PROGRAM)'
	$(Q)rm -f $(OBJS) *~ $(PROGRAM) po/*~
	$(Q)rm -rf share .dep
	$(Q)rm -rf mtpc-marshal.h mtpc-marshal.c

-include $(DEPS)

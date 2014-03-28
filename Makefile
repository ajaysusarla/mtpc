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

# Platform
UNAME := $(shell $(CC) -dumpmachine 2>&1 | grep -E -o "linux|darwin")

ifeq ($(UNAME), linux)
	OSSUPPORT = linux
	OSSUPPORT_CFLAGS =
else ifeq ($(UNAME), darwin)
	OSSUPPORT = darwin
	OSSUPPORT_CFLAGS += -DDARWIN
endif

# Check Versions
LIBMTP_REQ = 1.1.5
GLIB_REQ = 2.38.0
GTK_REQ = 3.10.0
GTHREAD_REQ = 2.38.0
LIBMTP_VERSION = $(shell $(PKGCONFIG) --modversion libmtp)
GLIB_VERSION = $(shell $(PKGCONFIG) --modversion glib-2.0)
GTK_VERSION = $(shell $(PKGCONFIG) --modversion gtk+-3.0)
GTHREAD_VERSION = $(shell $(PKGCONFIG) --modversion gthread-2.0)

# Dependencies
LIBMTP = $(shell $(PKGCONFIG) --libs libmtp)
LIBGTK = $(shell $(PKGCONFIG) --libs gtk+-3.0 glib-2.0)
LIBGTHREAD = $(shell $(PKGCONFIG) --libs gthread-2.0)

LIBMTPFLAGS = $(shell $(PKGCONFIG) --cflags libmtp)
GLIB2CFLAGS = $(shell $(PKGCONFIG) --cflags glib-2.0)
GTKCFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0)
GTHREADCFLAGS = $(shell $(PKGCONFIG) --cflags gthread-2.0)

LIBS = $(LIBMTP) $(LIBGTK) $(LIBGTHREAD)
LDFLAGS =

EXTRA_FLAGS +=	$(GTKCFLAGS) $(GLIB2CFLAGS) $(GTHREADCFLAGS) \
		$(LIBMTPCFLAGS) -DVERSION_STRING='"$(VERSION)"' \
		-DMTPC_UI_DIR='"$(ui_dir)"' -DMTPC_ICON_DIR='"$(icon_dir)"' \
		$(OSSUPPORT_CFLAGS)

# Object files
OBJS =  main.o \
	mtpc-marshal.o \
	gtk-utils.o \
	glib-utils.o \
	gio-utils.o \
	mtpc-app.o \
	mtpc-window.o \
	mtpc-actions-callbacks.o \
	mtpc-device.o \
	mtpc-devicelist.o \
	mtpc-statusbar.o \
	mtpc-folder-tree.o \
	mtpc-file-data.o \
	$(NULL)

# Headers
HDRS = 	main.h \
	mtpc-marshal.h \
	gtk-utils.h \
	glib-utils.h \
	gio-utils.h \
	mtpc-app.h \
	mtpc-window.h \
	mtpc-actions-callbacks.h \
	mtpc-device.h \
	mtpc-devicelist.h \
	mtpc-statusbar.h \
	mtpc-folder-tree.h \
	mtpc-file-data.h \
	$(NULL)

DEPS = $(wildcard .dep/*.dep)

all: $(PROGRAM)

$(PROGRAM): $(OBJS) $(HDRS)
	$(E) '             LD' $@
	$(Q)$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

%.o: %.c
	$(E) '             CC' $<
	$(Q)mkdir -p .dep
	$(Q)$(CC) $(CFLAGS) $(EXTRA_FLAGS) -MD -MF .dep/$@.dep -c -o $@ $<

mtpc-marshal.o: mtpc-marshal.c mtpc-marshal.h

mtpc-marshal.h: mtpc-marshal.list
	$(E) '             GENMARSHAL' $<
	$(Q)$(GLIB_GENMARSHAL) mtpc-marshal.list --header --prefix=mtpc_marshal > $@

mtpc-marshal.c: mtpc-marshal.list
	$(E) '             GENMARSHAL' $<
	$(E) "#include \"mtpc-marshal.h\"" > $@
	$(Q)$(GLIB_GENMARSHAL) mtpc-marshal.list --body --prefix=mtpc_marshal >> $@

clean:
	$(E) '   RM $(OBJS) $(PROGRAM)'
	$(Q)rm -f $(OBJS) *~ $(PROGRAM) po/*~
	$(Q)rm -rf share .dep
	$(Q)rm -rf mtpc-marshal.h mtpc-marshal.c

check:
	$(E) 'Found libmtp version  : $(LIBMTP_VERSION), Required >= $(LIBMTP_REQ)'
	$(E) 'Found gtk version     : $(GTK_VERSION), Required >= $(GTK_REQ)'
	$(E) 'Found glib version    : $(GLIB_VERSION), Required >= $(GLIB_REQ)'
	$(E) 'Found gthread version : $(GTHREAD_VERSION), Required >= $(GTHREAD_REQ)'

-include $(DEPS)

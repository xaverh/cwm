# cwm makefile for BSD make and GNU make
# uses pkg-config, DESTDIR and PREFIX

PROG=		cwm

PREFIX?=	/usr/local

SRCS=calmwm.cxx group.cxx reallocarray.cxx strlcat.cxx util.cxx xutil.cxx client.cxx  kbfunc.cxx screen.cxx strlcpy.cxx xevents.cxx y.tab.cxx conf.cxx menu.cxx search.cxx strtonum.cxx xmalloc.cxx

OBJS=$(subst .cxx,.o,$(SRCS))

PKG_CONFIG?=	pkg-config

CPPFLAGS+=	`${PKG_CONFIG} --cflags x11 xft xrandr`

CFLAGS?=	-Wall -O2 -g -D_GNU_SOURCE
CXXFLAGS?=  -Wall -O2 -g -D_GNU_SOURCE
CXXFLAGS+=  -std=c++20

LDFLAGS+=	`${PKG_CONFIG} --libs x11 xft xrandr`

MANPREFIX?=	${PREFIX}/share/man

all: ${PROG}

clean:
	rm -f ${OBJS} ${PROG}

${PROG}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} ${LDFLAGS} -o ${PROG}

%.o: %.cxx
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $<

install: ${PROG}
	install -d ${DESTDIR}${PREFIX}/bin ${DESTDIR}${MANPREFIX}/man1 ${DESTDIR}${MANPREFIX}/man5
	install -m 755 cwm ${DESTDIR}${PREFIX}/bin
	install -m 644 cwm.1 ${DESTDIR}${MANPREFIX}/man1
	install -m 644 cwmrc.5 ${DESTDIR}${MANPREFIX}/man5

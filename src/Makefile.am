#
# Makefile.am
#
# Written by: Francesco Salvestrini <f DOT salvestrini AT nextworks DOT it>
#

EXTRA_DIST                         =
DISTCLEANFILES                     =
bin_PROGRAMS                       =
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT =

rina_tgen_SOURCES  =				\
	simple_ap.cc		simple_ap.h	\
	server.cc		server.h	\
	client.cc		client.h	\
        timeutils.cc		timeutils.h	\
        main.cc

rina_tgen_LDADD    = $(LIBRINA_LIBS)		\
		     -lrt			\
		     $(BOOST_LDFLAGS)		\
		     $(BOOST_THREAD_LIB)	\
		     $(BOOST_SYSTEM_LIB)

rina_tgen_CPPFLAGS =				\
	$(LIBRINA_CFLAGS)			\
	$(CPPFLAGS_EXTRA)			\
        $(BOOST_CPPFLAGS)			\
	-I$(srcdir)/tclap

rina_tgen_CXXFLAGS =				\
	$(CPPFLAGS_EXTRA)

bin_PROGRAMS                       += rina-tgen
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT += rina-tgen

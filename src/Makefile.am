#
# Makefile.am
#
# Written by: Francesco Salvestrini <f DOT salvestrini AT nextworks DOT it>
#

EXTRA_DIST                         =
DISTCLEANFILES                     =
bin_PROGRAMS                       =
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT =

irina_traffic_generator_SOURCES  =				\
	application.cc         application.h		\
	server.cc              server.h			\
	client.cc              client.h			\
	main.cc
rina_tgen_LDADD    = $(LIBRINA_LIBS) -lrt
rina_tgen_CPPFLAGS =				\
	$(LIBRINA_CFLAGS)			\
	$(CPPFLAGS_EXTRA)			\
	-I$(srcdir)/tclap
rina_tgen_CXXFLAGS =				\
	$(CPPFLAGS_EXTRA)

bin_PROGRAMS                       += rina-tgen
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT += rina-tgen

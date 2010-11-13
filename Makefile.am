AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = myservices
myservices_SOURCES = src/main.c src/service_info.c src/utility.c foreign/dictionary.c foreign/iniparser.c 
noinst_HEADERS = src/myservices.h src/messages.h src/utility.h foreign/dictionary.h foreign/iniparser.h 

pkgsysconfdir = $(sysconfdir)/$(PACKAGE)
dist_pkgsysconf_DATA = conf/myservices.ini

dist-hook:
	/bin/cp myservices.spec $(distdir)/myservices.spec
	/bin/mkdir $(distdir)/scripts
	/bin/cp scripts/myservices.init $(distdir)/scripts

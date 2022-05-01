CC = cl
CFLAGS = -DHAVE_ATLSTR_H=1\
	-DHAVE_LIBIDN=1\
	-DHAVE_LIBINTL_H=1\
	-DNDEBUG=1\
	-DPDC_EXP_EXTRAS=1\
	-DPDC_NCMOUSE=1\
	-DTOAST_NOTIFICATIONS=1\
	-DWIN32=1\
	-DWIN32_LEAN_AND_MEAN=1\
	-MD\
	-O2\
	-W3\
	-nologo

CXX = $(CC)
CXXFLAGS = $(CFLAGS)\
	-DUNICODE=1\
	-EHsc\
	-ZW

# Versions
CURL_VERSION = 7.83.0
IDN_VERSION = 1.36
INTL_VERSION = 0.19.8.1
LIBRESSL_VERSION = 3.5.2
PDCURSES_VERSION = 3.9

LOCALES_SNAP = 20220430

# E and Q
E = @echo
Q = @

MACHINE = x64
NAME_libcrypto = crypto-49
NAME_libidn = libidn-12
NAME_libintl = libintl-8
NAME_libssl = ssl-52

LDFLAGS = -LIBPATH:curl-$(CURL_VERSION)/$(MACHINE)\
	-LIBPATH:idn-$(IDN_VERSION)/$(MACHINE)\
	-LIBPATH:intl-$(INTL_VERSION)/$(MACHINE)\
	-LIBPATH:libressl-$(LIBRESSL_VERSION)-windows/$(MACHINE)\
	-LIBPATH:pdcurses-$(PDCURSES_VERSION)/$(MACHINE)\
	-NODEFAULTLIB:MSVCRTD

LDLIBS = $(NAME_libcrypto).lib\
	$(NAME_libidn).lib\
	$(NAME_libintl).lib\
	$(NAME_libssl).lib\
	advapi32.lib\
	bcrypt.lib\
	kernel32.lib\
	libcurl.lib\
	pdcurses.lib\
	runtimeobject.lib\
	user32.lib\
	ws2_32.lib

RM = @del /q
# TEMPFILES = *.obj

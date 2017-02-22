# src/events/w32.mk

!include ../../w32_def.mk

extra_flags=
include_dirs=-I../include -I../libressl-$(LIBRESSL_VERSION)-windows/include
library_dirs=
log_file=stdout.log

OBJS=channel.obj error.obj misc.obj motd.obj names.obj        \
     noop.obj notice.obj ping.obj welcome-w32.obj welcome.obj \
     whois.obj privmsg.obj invite.obj list.obj banlist.obj

.c.obj:
	$(E) ^ ^ CC^ ^ ^ ^ ^ ^ $@
	$(Q) $(CC) $(include_dirs) $(CFLAGS) $(extra_flags) -c $*.c 1>>$(log_file)

all: $(OBJS)

channel.obj:
error.obj:
misc.obj:
motd.obj:
names.obj:
noop.obj:
notice.obj:
ping.obj:
welcome-w32.obj:
welcome.obj:
whois.obj:
privmsg.obj:
invite.obj:
list.obj:
banlist.obj:

clean:
	$(E) ^ ^ CLEAN
	$(RM) $(TEMPFILES) $(log_file)

# EOF

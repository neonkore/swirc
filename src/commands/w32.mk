# src/commands/w32.mk

!include ../../w32_def.mk

extra_flags=
include_dirs=-I../include
library_dirs=
log_file=stdout.log

OBJS=connect.obj jp.obj misc.obj msg.obj say.obj nick.obj topic.obj me.obj

.c.obj:
	$(E) ^ ^ CC^ ^ ^ ^ ^ ^ $@
	$(Q) $(CC) $(include_dirs) $(CFLAGS) $(extra_flags) -c $*.c 1>>$(log_file)

all: $(OBJS)

connect.obj:
jp.obj:
misc.obj:
msg.obj:
say.obj:
nick.obj:
topic.obj:
me.obj:

clean:
	$(E) ^ ^ CLEAN
	$(RM) $(TEMPFILES) $(log_file)

# EOF

# Common make defs

PKG = swirc

MSGMERGE = msgmerge
MMFLAGS = --backup=none\
	--sort-by-file\
	--update

XGETTEXT = xgettext
XGTFLAGS = --add-comments\
	--c++\
	--copyright-holder="Markus Uhlin"\
	--default-domain=$(PKG)\
	--foreign-user\
	--keyword=N_\
	--keyword=_\
	--msgid-bugs-address="https://github.com/uhlin/swirc/issues"\
	--output=$(PKG).pot\
	--sort-by-file

INPUTFILES = $(COMMANDS_DIR)colormap.cpp\
	$(EVENTS_DIR)account.cpp\
	$(EVENTS_DIR)away.cpp\
	$(EVENTS_DIR)channel.cpp\
	$(EVENTS_DIR)invite.cpp\
	$(EVENTS_DIR)misc.cpp\
	$(SRC_DIR)include/commandhelp.h\
	$(SRC_DIR)io-loop.c\
	$(SRC_DIR)log.c\
	$(SRC_DIR)main.cpp\
	$(SRC_DIR)network.cpp\
	$(SRC_DIR)readline.c\
	$(SRC_DIR)sig-w32.c\
	$(SRC_DIR)statusbar.cpp

POFILES = de/$(PKG).po\
	fi/$(PKG).po\
	fr/$(PKG).po\
	sv/$(PKG).po
MOFILES = de$(SLASH_SYM)$(PKG).mo\
	fi$(SLASH_SYM)$(PKG).mo\
	fr$(SLASH_SYM)$(PKG).mo\
	sv$(SLASH_SYM)$(PKG).mo

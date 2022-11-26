#include "common.h"

#include "../dataClassify.h"
#include "../network.h"
#include "../printtext.h"
#include "../strHand.h"

#include "ban.h"

#ifdef UNIT_TESTING
#undef  ACTWINLABEL
#define ACTWINLABEL "#chatzone"
#endif

#define NO_MASK			"no mask"
#define NOT_AN_IRC_CHANNEL	"active window isn't an irc channel"

/*
 * usage: /ban <mask>
 */
void
cmd_ban(const char *data)
{
	static const char cmd[] = "/ban";

	if (strings_match(data, "")) {
		printtext_print("err", "%s: %s", cmd, NO_MASK);
		return;
	} else if (!is_irc_channel(ACTWINLABEL)) {
		printtext_print("err", "%s: %s", cmd, NOT_AN_IRC_CHANNEL);
		return;
	}

	(void) net_send("MODE %s +b %s", ACTWINLABEL, data);
}

/*
 * usage: /unban <mask>
 */
void
cmd_unban(const char *data)
{
	static const char cmd[] = "/unban";

	if (strings_match(data, "")) {
		printtext_print("err", "%s: %s", cmd, NO_MASK);
		return;
	} else if (!is_irc_channel(ACTWINLABEL)) {
		printtext_print("err", "%s: %s", cmd, NOT_AN_IRC_CHANNEL);
		return;
	}

	(void) net_send("MODE %s -b %s", ACTWINLABEL, data);
}

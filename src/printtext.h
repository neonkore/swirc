#ifndef PRINTTEXT_H
#define PRINTTEXT_H

#include "irc.h"
#include "mutex.h"
#include "window.h"

#include <string.h> /* memset() */

#ifdef __cplusplus
extern "C" {
#endif

/* text decoration */
enum {
    BLINK     = '\035',
    BOLD      = '\002',
    COLOR     = '\003',
    NORMAL    = '\017',
    REVERSE   = '\026',
    UNDERLINE = '\037'
};

enum message_specifier_type {
    TYPE_SPEC1,
    TYPE_SPEC2,
    TYPE_SPEC3,
    TYPE_SPEC1_SPEC2,
    TYPE_SPEC1_FAILURE,
    TYPE_SPEC1_SUCCESS,
    TYPE_SPEC1_WARN,
    TYPE_SPEC_NONE
};

struct printtext_context {
    PIRC_WINDOW			window;
    enum message_specifier_type spec_type;
    bool                        include_ts;

    char timestamp[64];

#ifdef __cplusplus
    printtext_context() {
	this->window = NULL;
	this->spec_type = TYPE_SPEC_NONE;
	this->include_ts = false;
	memset(this->timestamp, 0, sizeof this->timestamp);
    }

    printtext_context(
	PIRC_WINDOW window,
	enum message_specifier_type spec_type,
	bool include_ts) {
	this->window = window;
	this->spec_type = spec_type;
	this->include_ts = include_ts;
	memset(this->timestamp, 0, sizeof this->timestamp);
    }
#endif
};

#if defined(UNIX)
extern pthread_mutex_t g_puts_mutex;
#elif defined(WIN32)
extern HANDLE g_puts_mutex;
#endif

/*lint -printf(2, printtext, swirc_wprintw) */

char		*squeeze_text_deco (char *buffer);
short int	 color_pair_find   (short int fg, short int bg);
void		 print_and_free    (const char *msg, char *cp);
void		 printtext         (struct printtext_context *, const char *fmt, ...) PRINTFLIKE(2);
void		 printtext_puts    (WINDOW *, const char *buf, int indent, int max_lines, int *rep_count);
void		 set_timestamp     (char *dest, size_t destsize, const struct irc_message_compo *) PTR_ARGS_NONNULL;
void		 swirc_wprintw     (WINDOW *, const char *fmt, ...) PRINTFLIKE(2);
void		 vprinttext        (struct printtext_context *, const char *fmt, va_list);

#ifdef __cplusplus
}
#endif

#endif

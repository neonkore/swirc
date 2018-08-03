#ifndef IRC_H
#define IRC_H

#include "window.h"

#ifdef __cplusplus
extern "C" {
#endif

struct irc_message_compo {
    int year;
    int month;
    int day;

    int hour;
    int minute;
    int second;
    int precision;

    char *prefix;
    char *command;
    char *params;
};

typedef void (*event_handler_fn)(struct irc_message_compo *);

enum to_window {
    STATUS_WINDOW,
    ACTIVE_WINDOW,
    NO_WINDOW
};

enum message_concat_state {
    CONCAT_BUFFER_IS_EMPTY,
    CONCAT_BUFFER_CONTAIN_DATA
};

extern char	*g_server_hostname;
extern char	*g_my_nickname;
extern bool	 g_alt_nick_tested;

void irc_init(void);
void irc_deinit(void);

bool has_server_time                (const struct irc_message_compo *);
void irc_extract_msg                (struct irc_message_compo *, PIRC_WINDOW, int ext_bits, bool is_error);
void irc_handle_interpret_events    (char *recvbuffer, char **message_concat, enum message_concat_state *);
void irc_set_my_nickname            (const char *nick);
void irc_set_server_hostname        (const char *srv_host);
void irc_unsuccessful_event_cleanup (void);

#ifdef __cplusplus
}
#endif

#endif

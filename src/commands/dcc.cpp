#include "common.h"

#include "../config.h"
#include "../errHand.h"
#include "../tls-server.h"

#include "dcc.h"

void
cmd_dcc(const char *data)
{
	UNUSED_PARAM(data);
}

void
dcc_init(void)
{
	if (config_bool("dcc", true)) {
		struct integer_context intctx("dcc_port", 1024, 65535, 8080);

		tls_server::begin(config_integer(&intctx));
	}
}

void
dcc_deinit(void)
{
	tls_server::end();
}

void
dcc_handle_incoming_conn(SSL *ssl)
{
	switch (SSL_accept(ssl)) {
	case 0:
		debug("%s: SSL_accept: The TLS/SSL handshake was not "
		    "successful", __func__);
		return;
	case 1:
		debug("%s: SSL_accept: The TLS/SSL handshake was successfully "
		    "completed", __func__);
		break;
	default:
		debug("%s: SSL_accept: The TLS/SSL handshake was not "
		    "successful because a fatal error occurred", __func__);
		return;
	}
}

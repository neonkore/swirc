#ifndef IDENT_DAEMON_HPP
#define IDENT_DAEMON_HPP
/* identd.hpp
   Copyright (C) 2022 Markus Uhlin. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   - Neither the name of the author nor the names of its contributors may be
     used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#if defined(UNIX)
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#elif defined(WIN32)
#include <ws2tcpip.h>
#endif

#include <cstring>

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#if defined(_lint) && !defined(NORETURN)
#define NORETURN
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#if UNIX
typedef int SOCKET;
#endif

#if defined(UNIX)
#define xclosesocket(sock) static_cast<void>(close(sock))
#elif defined(WIN32)
#define xclosesocket(sock) static_cast<void>(closesocket(sock))
#endif

class ident_client {
public:
	ident_client()
	{
		this->sock = INVALID_SOCKET;
		memset(&this->addr, 0, sizeof this->addr);
	}

	ident_client(const SOCKET clisock, const struct sockaddr_in &cliaddr)
	{
		this->sock = clisock;
		this->addr = cliaddr;
	}

	const char *
	get_ip(void) const
	{
		static char buf[INET_ADDRSTRLEN];

		if (inet_ntop(AF_INET, &this->addr.sin_addr, &buf[0],
		    sizeof buf) == NULL)
			return "";
		return &buf[0];
	}

	SOCKET
	get_sock(void) const
	{
		return this->sock;
	}

private:
	SOCKET sock;
	struct sockaddr_in addr;
};

namespace identd
{
	extern SOCKET		 sock;
	extern bool		 listening;
	extern bool		 loop;
	extern char		 fakename[];
	extern const char	*name;

	void	start(const int);
	void	stop(void);

	void	com_with_client(ident_client *);
	void	enter_loop(ident_client *);
	NORETURN void
		exit_thread(void);
	void	listen_on_port(const int);
	void	send_err_resp(const char *, const char *, ident_client *);
	void	send_response(const char *, const char *, ident_client *);
	void	set_reuseaddr(SOCKET);
}

#endif
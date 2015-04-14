#include <trfb.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


trfb_server_t *trfb_server_create(size_t width, size_t height)
{
	trfb_server_t *S;

	if (width > 0xffff || height > 0xffff) {
		trfb_msg("Width or height is more then 0xFFFF");
		return NULL;
	}

	S = calloc(1, sizeof(trfb_server_t));
	if (!S) {
		return NULL;
	}
	
	S->sock = -1;
	S->state = TRFB_STATE_STOPPED;
	
	trfb_msg("I:try to create framebuffer");
	
	S->fb = trfb_framebuffer_create(width, height, 32);
	if (!S->fb) {
		trfb_msg("Can't create framebuffer");
		free(S);
		return NULL;
	}

	S->clients = NULL;
	return S;
}

void trfb_server_destroy(trfb_server_t *server)
{
	if (trfb_server_get_state(server) & TRFB_STATE_WORKING) {
		trfb_server_stop(server);
	}

	close(server->sock);
	trfb_framebuffer_free(server->fb);

	/* TODO: remove all clients */

	free(server);
}

unsigned trfb_server_get_state(trfb_server_t *S)
{
	unsigned res;

	res = S->state;

	return res;
}

int trfb_server_set_socket(trfb_server_t *server, int sock)
{
	if (server->state != TRFB_STATE_STOPPED) {
		trfb_msg("Server is working now");
		return -1;
	}

	server->sock = sock;

	return 0;
}

int trfb_server_bind(trfb_server_t *server, const char *host, const char *port)
{
	struct addrinfo hints;
	struct addrinfo *addr;
	struct addrinfo *addrs;
	int sock;
	int res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	res = getaddrinfo(host, port, &hints, &addrs);
	if (res) {
		trfb_msg("getaddrinfo failed: %s", gai_strerror(res));
		return -1;
	}
	
	for (addr = addrs; addr; addr = addr->ai_next) {
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sock < 0)
			continue;

		if (bind(sock, addr->ai_addr, addr->ai_addrlen) == 0) {
			freeaddrinfo(addrs);

			trfb_msg("I:trfb_server_set_socket(%p, %d)", server, sock);
			return trfb_server_set_socket(server, sock);
		}

		close(sock);
	}

	freeaddrinfo(addrs);
	trfb_msg("Can't bind to %s:%s", host, port);

	return -1;
}


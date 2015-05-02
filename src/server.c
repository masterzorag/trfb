#include "trfb.h"
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int server(void *srv_in);

static void xsleep(unsigned ms)
{
	usleep(ms * 1000);
}

int trfb_server_start(trfb_server_t *srv)
{
	if (srv->clients || srv->state != TRFB_STATE_STOPPED) {
		trfb_msg("E: server is always started but trfb_server_start called!");
		return -1;
	}

	if (!srv->fb || srv->sock < 0) {
		trfb_msg("Server parameters is not set. Invalid trfb_server content.");
		return -1;
	}

	/*
		call main processing function, we does not return
		callchain:
			server,
			 connection, negotiate
			  loop into connection
	*/
	int res = server(srv);
	
	/* not reached */
	for (;;) {
		if (srv->state == TRFB_STATE_WORKING) {
			trfb_msg("I:server started!");
			return 0;
		}

		if (srv->state != TRFB_STATE_STOPPED) {
			trfb_msg("E:invalid server state");
			return -1;
		}

		xsleep(1);
	}
	return 0; 
}

static void stop_all_connections(trfb_server_t *srv);

int server(void *srv_in)
{
	trfb_server_t *srv = srv_in;
	trfb_connection_t *con;
	fd_set fds;
	struct timeval tv;
	int rv;
	struct sockaddr addr;
	socklen_t addrlen;
	int sock;

#define EXIT_THREAD(s) \
	do { \
		stop_all_connections(srv); \
		srv->state = s; \
		return 0; \
	} while (0)

	trfb_msg("I:starting server...");
	srv->state = TRFB_STATE_WORKING;

	if (listen(srv->sock, 8)) {
		trfb_msg("listen failed");		
		EXIT_THREAD(TRFB_STATE_ERROR);	
	}

	for (;;) {
		if (srv->state == TRFB_STATE_STOP) {
			trfb_msg("I:server stopped");
			EXIT_THREAD(TRFB_STATE_STOPPED);
		}

		FD_ZERO(&fds);
		FD_SET(srv->sock, &fds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		rv = select(srv->sock + 1, &fds, NULL, NULL, &tv);
		
		trfb_msg("I:select[%d]", rv);
		if (rv < 0) {
			trfb_msg("select failed");
			EXIT_THREAD(TRFB_STATE_ERROR);
		}

		if (rv > 0) {
			sock = accept(srv->sock, &addr, &addrlen);
			trfb_msg("I:new client! sock[%d]", sock);
			
			if (sock >= 0) {
				trfb_msg("I:finally! sock[%d]", sock);
				
				con = trfb_connection_create(srv, sock, &addr, addrlen); // --> jump into
				if (!con) {
					trfb_msg("W:can not create new connection");
				} else {
					puts("exception");
					con->next = srv->clients;
					srv->clients = con;
				}
			} else {
				trfb_msg("E:%s\n", strerror(errno));
			}
		}

	}

	return 0;
}

int trfb_server_stop(trfb_server_t *srv)
{
	int res = 0;
	srv->state = TRFB_STATE_STOP;

	while (trfb_server_get_state(srv) == TRFB_STATE_STOP) {
		xsleep(2);
	}
	
	return 0;
}

static void stop_all_connections(trfb_server_t *srv)
{
	trfb_connection_t *con;
	trfb_connection_t *connections;
	int work, res;

	trfb_msg("I:waiting all clients to stop...");

	connections = srv->clients;
	srv->clients = NULL;

	for (con = connections; con; con = con->next) {
		if (con->state == TRFB_STATE_WORKING)
			con->state = TRFB_STATE_STOP;
	}

	while (connections) {
		work = 0;
		for (con = connections; con; con = con->next) {
			if (con->state == TRFB_STATE_STOP) {
				++work;
			}
		}

		if (work) {
			xsleep(1);
		} else {
			for (con = connections; con; con = con->next) {
//				thrd_join(con->thread, &res);
			}

			while (connections) {
				con = connections;
				connections = con->next;
				trfb_connection_free(con);
			}
		}
	}

	trfb_msg("I:all clients have been stoped...");
	return;
}

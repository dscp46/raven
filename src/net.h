#ifndef __NET_H
#define __NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void *get_in_addr(struct sockaddr *sa);
int connect_to_host(const char *host, struct addrinfo **res_out, char *ipstr_out, const char *portnum);

#endif	/* __NET_H */

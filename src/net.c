#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int connect_to_host(const char *host, struct addrinfo **res_out, char *ipstr_out, const char *portnum) {
    struct addrinfo hints, *res, *p;
    int sockfd;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, portnum, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr = get_in_addr((struct sockaddr *)p->ai_addr);
        inet_ntop(p->ai_family, addr, ipstr_out, INET6_ADDRSTRLEN);
        printf("Trying to connect to %s...\n", ipstr_out);

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect");
            close(sockfd);
            continue;
        }

        *res_out = res; // Return the addrinfo for reuse
        return sockfd;
    }

    freeaddrinfo(res);
    return -1;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "allowlist.h"
#include "aprs.h"
#include "nagios.h"
#include "net.h"


#define PORT "14580"
#define BUFFER_SIZE 1480
#define RECONNECT_DELAY 5  // seconds

int main(int argc, char *argv[]) {
    const char host[] = "euro.aprs2.net";
    struct addrinfo *res = NULL;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd;
    char aprsis_user[] = "F4HOF-R", \
         aprsis_pass[] = "-1", \
         aprsis_filter[] = "r/45.4/4.5/60";
    allowlist_t *allowed_callsigns = allowlist_init();
    allowed_callsigns->add( allowed_callsigns, "F1ZCK-14");

    nagios_svc_ret_t status;
    char *message[] = {
        "OK - On AC power",
        "WARNING - On battery power",
        "CRITICAL - Unable to communicate with power supply",
        "UNKNOWN - Invalid status value",
    };
    struct timeval tv = {40, 0};

    while (1) {
        sockfd = connect_to_host(host, &res, ipstr, PORT);
        if (sockfd == -1) {
            fprintf(stderr, "Connection failed. Retrying in %d seconds...\n", RECONNECT_DELAY);
            sleep(RECONNECT_DELAY);
            continue;
        }

        printf("Connected to %s\n", ipstr);

        setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

	// Send login banner
        char login_msg[80];
        snprintf( login_msg, 80, "user %s pass %s vers Raven 1.0 filter %s\r\n", aprsis_user, aprsis_pass, aprsis_filter);

        ssize_t sent = send(sockfd, login_msg, strlen(login_msg), 0);
        if (sent == -1) {
            perror("send failed");
            close(sockfd);
            freeaddrinfo(res);
            sleep(RECONNECT_DELAY);
            continue;
        }

        char buffer[BUFFER_SIZE];
        char callsign[CALLSIGN_SZ+1];
        char *payload;
        char sym_table, sym_code;

        size_t len;
	time_t recv_time;
	int is_local;
	char *mesg;

        while(1) {
            ssize_t received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0)
                break;

            buffer[received] = '\0';
            //printf("Received:\n%s\n", buffer);
       	    len = strlen( buffer);

            if( len == 0 || buffer[0] == '#' )
                continue;

            if( buffer[len-2] != '\r' && buffer[len-1] != '\n' )
                continue;

            buffer[ len-2 ] = 0;

            if( aprsis_parse( buffer, callsign, &payload) != 0 )
                continue;

            if( aprs_parse_symbol( payload, &sym_table, &sym_code) != 0)
                continue;

            recv_time = time(NULL);
	    //if( aprs_parse_time( payload, &recv_time, &is_local) != 0 )
	    //	continue;

	    if ( sym_table == 'P' && sym_code == 'W' )
	    {
		    mesg = message[0];
		    status = SVC_OK;
	    }
	    else if ( sym_table == 'P' && sym_code == '0' )
	    {
		    mesg = message[1];
		    status = WARNING;
	    }
	    else if ( sym_table == 'P' && sym_code == 'n' )
	    {
		    mesg = message[2];
		    status = CRITICAL;
	    }
	    else
	    {
		    mesg = message[3];
		    status = UNKNOWN;
	    }

	    if ( allowed_callsigns->find( allowed_callsigns, callsign) != NULL )
		    nagios_send_svc_check( "-", &recv_time, callsign, "Power", status, mesg);
	    else
		    printf( "Ignored entry for %s\n", callsign);

	    //printf( "Src: %s, Logo: %c%c\n", callsign, sym_table, sym_code);

        }

        // Close connection to simulate broken session (remove to keep it open)
        close(sockfd);
        freeaddrinfo(res);

        printf("Connection closed. Reconnecting in %d seconds...\n", RECONNECT_DELAY);
        sleep(RECONNECT_DELAY);
    }

    return 0;
}


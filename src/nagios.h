#ifndef __NAGIOS_H
#define __NAGIOS_H

#include <time.h>

typedef enum nagios_svc_ret {
	SVC_OK		= 0,
	WARNING		= 1,
	CRITICAL	= 2,
	UNKNOWN		= 3
} nagios_svc_ret_t;

typedef enum nagios_host_status {
	HOST_OK		= 0,
	DOWN		= 1,
	UNREACHABLE	= 2
} nagios_host_status_t;

// Send passive check result.
// 	If the time_t argument is NULL, the current timestamp will be used.
int nagios_send_svc_check( char *fname, time_t *t, char *hostname, char *svcname, nagios_svc_ret_t retcode, char *output);
int nagios_send_host_check( char *fname, time_t *t, char *hostname, nagios_host_status_t status, char *output);

#endif	/* __NAGIOS_H */

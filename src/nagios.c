#include "nagios.h"

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

int nagios_send_svc_check( char *fname, time_t *t, char *hostname, char *svcname, nagios_svc_ret_t retcode, char *output)
{
	FILE *hFile;
	if ( strcmp( fname, "-") != 0 )
		hFile = fopen( fname, "a");
	else
		hFile = stdout;

	if( !hFile)
		return errno;

	time_t when = (t == NULL) ? time( NULL) : *t;

	fprintf( hFile, "[%jd] PROCESS_SERVICE_CHECK_RESULT;%s;%s;%d;%s\n", (intmax_t) when, hostname, svcname, retcode, output);

	if ( strcmp( fname, "-") != 0 )
		fclose( hFile);
	return 0;
}

int nagios_send_host_check( char *fname, time_t *t, char *hostname, nagios_host_status_t status, char *output)
{
	FILE *hFile;
	if ( strcmp( fname, "-") != 0 )
		hFile = fopen( fname, "a");
	else
		hFile = stdout;

	if( !hFile)
		return errno;

	time_t when = (t == NULL) ? time( NULL) : *t;

	fprintf( hFile, "[%jd] PROCESS_HOST_CHECK_RESULT;%s;%d;%s\n", (intmax_t) when, hostname, status, output);

	if ( strcmp(fname, "-" ) != 0 )
		fclose( hFile);
	return 0;
}

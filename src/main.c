#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "aprs.h"
#include "nagios.h"

int main( int argc, char *argv[], char *envp[])
{
	char str[] = "F1ZCK-14>APEKM1,TCPIP*,qAC,T2UKRAINE:;F1ZCK-14 *100917z4523.66NP00424.12EWEDF OK, EurakMon\r\n";
	char callsign[CALLSIGN_SZ+1];
	char *payload;
	char sym_table, sym_code;

	size_t len = strlen( str);
	if( str[len-2] != '\r' && str[len-1] != '\n' )
		return 1;

	str[ len-2 ] = 0;

	if( aprsis_parse( str, callsign, &payload) != 0 )
		return 1;

	if( aprs_parse_symbol( payload, &sym_table, &sym_code) != 0)
		return 1;

	printf( "Src: %s, Logo: %c%c\n", callsign, sym_table, sym_code);
	return 0;
}

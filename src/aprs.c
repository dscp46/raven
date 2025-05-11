
#include "aprs.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int aprsis_parse( char *str, char *callsign, char **payload)
{
	char *src_end = strchr( str, '>');
	if( src_end == NULL )
		return EBADMSG;
	
	*payload = strchr( str, ':');
	if( *payload == NULL )
		return EBADMSG;

	size_t src_sz = src_end - str;
	if( src_sz > CALLSIGN_SZ )
		return ERANGE;

	++*payload;

	memcpy( callsign, str, src_sz);
	callsign[src_sz] = '\0';
	return 0;
}

int aprs_parse_symbol( char *str, char *tid, char *code)
{
	size_t len = strlen( str);

	switch( str[0] )
	{
	case ';':
		// Compressed reports can be 31 bytes, but are not (yet) supported.
		// if ( !( (len >= 31 && str[18] == '/') || len >= 37 ) )
		if( len < 37 )
			return EBADMSG;
		
		// Skip non-live objects.
		if( str[10] != '*' )
			return EINVAL;
		
		*tid = str[26];
		*code = str[36];
		break;

	case '!':
	case '=':
		if ( len < 20 )
			return EBADMSG;

		*tid = str[9];
		*code = str[19];

		break;

	case '/':
	case '@':
		if ( len < 27 )
			return EBADMSG;

		*tid = str[16];
		*code = str[26];
		break;

	default:
		return EBADMSG;
	}

	return 0;
}

int aprs_parse_time( char *payload, time_t *t, int *is_local)
{
	if( t == NULL || payload == NULL || is_local == NULL )
		return EINVAL;

	size_t len = strlen( payload);
	time_t t_now = time( NULL);
	struct tm *now = gmtime(&t_now);
	int cur_day = now->tm_mday;

	switch( payload[0])
	{
	case '/':
	case '@':
		if ( len < 8 )
			return EBADMSG;
		
		switch ( payload[7] )
		{
		case 'z':
			// DHM, zulu time
			*is_local = 0;
			if( !sscanf( payload+1, "%2d%2d%2d", &(now->tm_mday), &(now->tm_hour), &(now->tm_min)))
				return EBADMSG;
			now->tm_sec = 0;

			break;

		case '/':
			// DHM, local timezone
			*is_local = 1;
			if( !sscanf( payload+1, "%2d%2d%2d", &(now->tm_mday), &(now->tm_hour), &(now->tm_min)))
				return EBADMSG;
			now->tm_sec = 0;
			break;

		case 'h':
			// HMS, zulu time
			*is_local = 0;
			if( !sscanf( payload+1, "%2d%2d%2d", &(now->tm_hour), &(now->tm_min), &(now->tm_sec)))
				return EBADMSG;
			break;

		default:
			return EBADMSG;
		}

		if ( now->tm_mday > cur_day )
			--(now->tm_mon);

		if ( now->tm_mon == 0 )
		{
			now->tm_mon = 12;
			--(now->tm_year);
		}
		
		*t = mktime( now);
		break;

	default:
		return EBADMSG;
	}
	return 0;
}

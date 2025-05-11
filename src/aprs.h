#ifndef __APRS_H
#define __APRS_H

#define	CALLSIGN_SZ	9

#include <time.h>
int aprsis_parse( char *str, char *callsign, char **payload);
int aprs_parse_symbol( char *str, char *tid, char *code);
int aprs_parse_time( char *payload, time_t *t, int *is_local);

#endif	/* APRS_H */

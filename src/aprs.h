#ifndef __APRS_H
#define __APRS_H

#define	CALLSIGN_SZ	9

int aprsis_parse( char *str, char *callsign, char **payload);
int aprs_parse_symbol( char *str, char *tid, char *code);

#endif	/* APRS_H */

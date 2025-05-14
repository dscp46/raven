#ifndef __SETTINGS_H
#define __SETTINGS_H

#include "allowlist.h"

typedef struct app_ctx_t app_ctx_t;
typedef struct settings_t settings_t;

struct app_ctx_t {
	int sock_aprsis_fd;
	int sock_cfg_srv_fd;
	settings_t *settings;
};

struct settings_t {
	char *aprsis_fqdn;
	char *aprsis_port;
	char *aprsis_user;
	char *aprsis_passcode;
	char *aprsis_filter;
	allowlist_t *allowed_callsigns;
	int debug;
	
	int (*load)( struct settings_t *self, const char* filename);
	int (*save)( struct settings_t *self, const char* filename);
	void (*cleanup) ( struct settings_t *self);
};

struct settings_t *settings_init(void);
void settings_reload( app_ctx_t *ctx, const char* filename);

#endif	/* __SETTINGS_H */

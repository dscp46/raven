#include "settings.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libconfig.h>

/***** Local declarations *****/
typedef struct allowed_qrz_t allowed_qrz_t;

struct allowed_qrz_t {
	char *qrz;
	struct allowed_qrz_t *next;
};

int settings_load( struct settings_t *self, const char* filename);
int settings_save( struct settings_t *self, const char* filename);
void settings_cleanup( struct settings_t *self);

/***** Implementations *****/
struct settings_t *settings_init(void)
{
	struct settings_t *instance = malloc( sizeof(settings_t));
	if ( instance == NULL )
	{
		fprintf( stderr, "config_init(): malloc failed\n");
		return NULL;
	}
	
	// Set method pointers
	instance->load = settings_load;
	instance->save = settings_save;
	instance->cleanup = settings_cleanup;
	
	// Initialize allowlist
	instance->allowed_callsigns = allowlist_init();
	if ( instance->allowed_callsigns )
	{
		instance->cleanup( instance);
		return NULL;
	}
	
	// Initialize other attributes
	instance->aprsis_fqdn = NULL;
	instance->aprsis_user = NULL;
	instance->aprsis_passcode = NULL;
	instance->aprsis_filter = NULL;
	instance->debug = 0;
}

void settings_cleanup( struct settings_t *self)
{
	if ( self == NULL ) return;
	
	if ( self->allowed_callsigns != NULL )
		self->allowed_callsigns->cleanup( self->allowed_callsigns);
	
	if ( instance->aprsis_fqdn != NULL )
		free(instance->aprsis_fqdn);
	
	if( instance->aprsis_user != NULL )
		free(instance->aprsis_user);
	
	if( instance->aprsis_passcode != NULL )
		free(instance->aprsis_passcode);
	
	if( instance->aprsis_filter != NULL )
		free(instance->aprsis_filter);
	
	free( instance);
}

int settings_load( struct settings_t *self, const char* filename)
{
	if ( self == NULL || filename == NULL )
		return;
	
	return 0;
}

int settings_save( struct settings_t *self, const char* filename)
{
	if ( self == NULL || filename == NULL )
		return;
	
	return 0;
}

void settings_reload( app_ctx_t *ctx, const char* filename)
{
	if ( ctx == NULL || filename == NULL )
		return;
}

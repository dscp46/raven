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
char *strdupalloc( const char *str);
void setting_lookup_string( config_t *cfg, const char *node_name, char **str);
void setting_load_allowlist( config_t *cfg, const char *node_name, allowlist_t *list);

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
	if ( instance->allowed_callsigns == NULL )
	{
		instance->cleanup( instance);
		return NULL;
	}
	
	// Initialize other attributes
	instance->aprsis_fqdn = NULL;
	instance->aprsis_user = NULL;
	instance->aprsis_port = NULL;
	instance->aprsis_passcode = NULL;
	instance->aprsis_filter = NULL;
	instance->nagios_cmd_fname = NULL;
	instance->debug = 0;
	
	return instance;
}

void settings_cleanup( struct settings_t *self)
{
	if ( self == NULL ) return;
	
	if ( self->allowed_callsigns != NULL )
		self->allowed_callsigns->cleanup( self->allowed_callsigns);
	
	if ( self->aprsis_fqdn != NULL )
		free(self->aprsis_fqdn);
	
	if( self->aprsis_user != NULL )
		free(self->aprsis_user);
	
	if( self->aprsis_port != NULL )
		free(self->aprsis_port);
	
	if( self->aprsis_passcode != NULL )
		free(self->aprsis_passcode);
	
	if( self->aprsis_filter != NULL )
		free(self->aprsis_filter);

	if( self->nagios_cmd_fname != NULL )
		free(self->nagios_cmd_fname);
	
	free( self);
}

int settings_load( struct settings_t *self, const char* filename)
{
	if ( self == NULL || filename == NULL )
		return EINVAL;
	
	config_t cfg;
	config_init(&cfg);
	
	if ( !config_read_file(&cfg, filename) )
	{
		fprintf(stderr, "Error reading config: %s:%d - %s\n",
			config_error_file(&cfg),
			config_error_line(&cfg),
			config_error_text(&cfg)
		);
		
		config_destroy(&cfg);
		return ECANCELED;
	}
	
	setting_lookup_string( &cfg, "app.aprsis.fqdn", &self->aprsis_fqdn);
	setting_lookup_string( &cfg, "app.aprsis.port", &self->aprsis_port);
	setting_lookup_string( &cfg, "app.aprsis.user", &self->aprsis_user);
	setting_lookup_string( &cfg, "app.aprsis.passcode", &self->aprsis_passcode);
	setting_lookup_string( &cfg, "app.aprsis.filter", &self->aprsis_filter);
	setting_lookup_string( &cfg, "app.nagios.filename", &self->nagios_cmd_fname);
	config_lookup_int( &cfg, "app.debug", &self->debug);
	
	setting_load_allowlist( &cfg, "app.allowlist", self->allowed_callsigns);
	
	config_destroy(&cfg);
	return 0;
}

int settings_save( struct settings_t *self, const char* filename)
{
	if ( self == NULL || filename == NULL
		|| self->aprsis_fqdn == NULL
		|| self->aprsis_port == NULL
		|| self->aprsis_user == NULL
		|| self->aprsis_passcode == NULL
		|| self->aprsis_filter == NULL
		|| self->nagios_cmd_fname == NULL
	)
		return EINVAL;
		
	config_t cfg;
    config_setting_t *root, *app, *aprsis, *nagios;
    config_init(&cfg);
    
    root = config_root_setting(&cfg);
    app = config_setting_add(root, "app", CONFIG_TYPE_GROUP);
    aprsis = config_setting_add(app, "aprsis", CONFIG_TYPE_GROUP);
    nagios = config_setting_add(app, "nagios", CONFIG_TYPE_GROUP);
    
    config_setting_add(aprsis, "fqdn", CONFIG_TYPE_STRING)->value.sval = (char *)self->aprsis_fqdn;
    config_setting_add(aprsis, "port", CONFIG_TYPE_STRING)->value.sval = (char *)self->aprsis_port;
    config_setting_add(aprsis, "user", CONFIG_TYPE_STRING)->value.sval = (char *)self->aprsis_user;
    config_setting_add(aprsis, "passcode", CONFIG_TYPE_STRING)->value.sval = (char *)self->aprsis_passcode;
    config_setting_add(aprsis, "filter", CONFIG_TYPE_STRING)->value.sval = (char *)self->aprsis_filter;

    config_setting_add(nagios, "filename", CONFIG_TYPE_STRING)->value.sval = (char *)self->nagios_cmd_fname;
    
    config_setting_add(app, "debug", CONFIG_TYPE_INT)->value.ival = self->debug;
    
    if (!config_write_file(&cfg, filename)) {
    	fprintf(stderr, "Error writing config file\n");
    	config_destroy(&cfg);
    	return EACCES;
    }
    
    config_destroy(&cfg);
	return 0;
}

void settings_reload( app_ctx_t *ctx, const char* filename)
{
	if ( ctx == NULL || filename == NULL )
		return;
}

// Allocate a string of the same size of str, then duplicate its content.
char *strdupalloc( const char *str)
{
	if ( str == NULL ) return NULL;
	
	size_t cstr_sz = strlen( str)+1;
	char *instance = malloc(cstr_sz);
	
	if ( instance == NULL )
		return NULL;
	
	memcpy( instance, str, cstr_sz);
	instance[cstr_sz] = '\0';
	
	return instance;
}

// Look up for a string in a config tree. If it exists, update the content, with necessary re-allocs.
void setting_lookup_string( config_t *cfg, const char *node_name, char **str)
{
	if ( cfg == NULL || node_name == NULL || str == NULL )
		return;
	
	config_setting_t *setting;	
	setting = config_lookup( cfg, node_name);
	
	if( !setting )
		return;
	
	if ( *str != NULL )
		free( *str );
	
	*str = strdupalloc( config_setting_get_string(setting));
}

void setting_load_allowlist( config_t *cfg, const char *node_name, allowlist_t *list)
{
	if( cfg == NULL || node_name == NULL || list == NULL )
		return;

	config_setting_t *setting;
	const char *value;
	int count;

	setting = config_lookup( cfg, node_name);
	if( setting == NULL || !config_setting_is_array(setting) )
		return;

	count = config_setting_length( setting);

	// TODO: Purge the allowlist content

	while ( count-- > 0 )
	{
		if( ( value = config_setting_get_string_elem( setting, count)) != NULL )
			list->add( list, value);
	}
}


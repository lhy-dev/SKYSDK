#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <syslog.h>
#include "sky.h"

int sky_log_level = SKY_LOG_LEVEL_WARN;

bool sky_debug_init ( void )
{
	char * log_level;

	SKY_LOG_TRACE( "%s\n", __func__ );

	openlog( sky_process_get_short_name(), 0, LOG_USER );

	log_level = getenv( SKY_LOG_LEVEL_ENVNAME );
	if ( log_level != NULL )
	{
		if ( sky_str_casecmp( log_level, "FATAL" ) == 0 )
		{
			sky_log_level = SKY_LOG_LEVEL_FATAL;
		}
		else if ( sky_str_casecmp( log_level, "ERROR" ) == 0 )
		{
			sky_log_level = SKY_LOG_LEVEL_ERROR;
		}
		else if ( sky_str_casecmp( log_level, "WARN" ) == 0 )
		{
			sky_log_level = SKY_LOG_LEVEL_WARN;
		}
		else if ( sky_str_casecmp( log_level, "TRACE" ) == 0 )
		{
			sky_log_level = SKY_LOG_LEVEL_TRACE;
		}
		else if ( sky_str_casecmp( log_level, "DEBUG" ) == 0 )
		{
			sky_log_level = SKY_LOG_LEVEL_DEBUG;
		}
	}

	setlogmask( LOG_UPTO( sky_log_level ) );

	return true;
}

void sky_debug_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	closelog();
}

#define SYSLOG(prio) \
	do \
	{ \
		va_list args; \
		va_start( args, fmt ); \
		if ( sky_process_is_run_in_background() ) \
		{ \
			vsyslog( prio, fmt, args ); \
		} \
		else \
		{ \
			SKY_SYSTEM_TIME local_time; \
			if ( sky_time_get_localtime( &local_time ) ) \
			{ \
				printf( \
					"%02d:%02d:%02d.%03d ", \
					local_time.hour, \
					local_time.minute, \
					local_time.second, \
					local_time.milliseconds ); \
			} \
			vprintf( fmt, args ); \
		} \
		va_end(args); \
	} while ( 0 )

SKY_API int sky_log_debug ( char const * fmt, ... )
{
	SYSLOG( SKY_LOG_LEVEL_DEBUG );
	return 0;
}

SKY_API int sky_log_trace ( char const * fmt, ... )
{
	SYSLOG( SKY_LOG_LEVEL_TRACE );
	return 0;
}

SKY_API int sky_log_warn ( char const * fmt, ... )
{
	SYSLOG( SKY_LOG_LEVEL_WARN );
	return 0;
}

SKY_API int sky_log_error ( char const * fmt, ... )
{
	SYSLOG( SKY_LOG_LEVEL_ERROR );
	return 0;
}

SKY_API int sky_log_fatal ( char const * fmt, ... )
{
	SYSLOG( SKY_LOG_LEVEL_FATAL );
	return 0;
}

SKY_API int sky_log_print_buf ( void * buf, uint32_t len )
{
	char output[ 1024 ];
	char * d;
	uint8_t * s;

	s = (uint8_t*) buf;
	d = output;

	while ( len-- && d - output < sizeof( output ) )
	{
		sky_str_printf( d, 10, "%02x ", *s++ );
		d += 3;
	}

	*d = 0;

	if ( sky_process_is_run_in_background() )
	{
		syslog( SKY_LOG_LEVEL_TRACE, "%s\n", output );
	}
	else
	{
		printf( "%s\n", output );
	}

	return 0;
}

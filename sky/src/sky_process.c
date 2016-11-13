#include "sky.h"
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>

static volatile sig_atomic_t use_default_handler = 1;

static SKY_HANDLE process_exit = SKY_INVALID_HANDLE;
static bool run_in_background = false;

extern char * program_invocation_name;
extern char * program_invocation_short_name;

bool sky_process_init ( void )
{
	struct sigaction sa;
    sigset_t sst;

	SKY_LOG_TRACE( "%s\n", __func__ );

	sigemptyset( &sst );

	sky_mem_clear( &sa, sizeof( sa ) );
	sa.sa_mask = sst;
	sa.sa_flags = 0;

	RFOF( sigaction( SIGTERM, &sa, NULL ) == 0 );
	RFOF( sigaction( SIGINT , &sa, NULL ) == 0 );
	RFOF( sigaction( SIGQUIT, &sa, NULL ) == 0 );

	return true;
}

void sky_process_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	SKY_SAFE_CLOSE_HANDLE( process_exit );
}

SKY_API bool sky_process_is_first_instance ( char const * proc_name )
{
	char pid_file_name[ SKY_PATH_MAX + 1 ];
	int pid_file;
	int ret;

	sky_str_printf(
		pid_file_name,
		sizeof( pid_file_name ),
		"/tmp/sky_proc_%s.pid",
		proc_name );

	pid_file = open(
		pid_file_name,
		O_CREAT | O_RDWR,
		0666 );
	if ( -1 == pid_file )
	{
		SKY_LOG_WARN(
			"%s: fail to open file %s, error = %d\n",
			__func__,
			pid_file_name,
			errno );
		return false;
	}

	ret = flock( pid_file, LOCK_EX | LOCK_NB );
	if ( 0 == ret )
	{
		// this is the first instance
		return true;
	}
	else
	{
		if ( EWOULDBLOCK == errno )
		{
			SKY_LOG_WARN(
				"%s: another instance is running with name %s\n",
				__func__,
				proc_name );
		}
		else
		{
			SKY_LOG_ERROR(
				"%s: flock return errno = %d with name %s\n",
				__func__,
				errno,
				proc_name );
		}
	}

	return false;
}


SKY_API char const * sky_process_get_name ( void )
{
	return program_invocation_name;
}

SKY_API char const * sky_process_get_short_name ( void )
{
	return program_invocation_short_name;
}

SKY_API bool sky_process_set_run_in_background ( void )
{
	if ( daemon( 0, 0 ) != 0 )
	{
		return false;
	}

	run_in_background = true;

	return true;
}

SKY_API bool sky_process_is_run_in_background ( void )
{
	return run_in_background;
}

SKY_API char * sky_process_get_env ( char const * name )
{
	return getenv( name );
}

SKY_API int sky_process_set_env (
	char const * name,
	char const * value,
	int overwrite )
{
	return setenv( name, value, overwrite );
}

SKY_API int sky_process_clear_env ( void )
{
	return clearenv();
}

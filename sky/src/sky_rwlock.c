#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>
#include <pthread.h>
#include "sky.h"

#define RWLOCK_MEMORY_TAG				"SKYRWLOCK"
#define RWLOCK_NAME_PATTERN				"/tmp/SKY_RWLOCK_%s"

struct sky_handle_rwlock
{
	uint16_t cb;
	uint16_t type;
	pthread_rwlock_t rwlock;
	int fd_rwlock;
	char * file_name;
};
typedef struct sky_handle_rwlock  		SKY_HANDLE_RWLOCK;

void sky_abstime_after_msec (
	uint32_t milliseconds,
	struct timespec *abs_time );

bool sky_rwlock_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_rwlock_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_rwlock_create ( char const * name )
{
	SKY_HANDLE_RWLOCK * p;
	int rc;

	p = sky_mem_alloc( sizeof( SKY_HANDLE_RWLOCK ) );
	RVON( p, SKY_INVALID_HANDLE );
	p->cb = sizeof( SKY_HANDLE_RWLOCK );
	p->type = SKY_HANDLE_TYPE_RWLOCK;
	p->fd_rwlock = -1;

	if ( name != NULL )
	{
		p->file_name = sky_mem_alloc( SKY_NAME_MAX + 1 );
		if ( NULL == p->file_name )
		{
			SKY_MEM_SAFE_FREE( p );
			return SKY_INVALID_HANDLE;
		}

		rc = sky_str_printf(
			p->file_name,
			SKY_NAME_MAX,
			RWLOCK_NAME_PATTERN,
			name );
		if ( rc <= 0 )
		{
			SKY_MEM_SAFE_FREE( p->file_name );
			SKY_MEM_SAFE_FREE( p );
			return SKY_INVALID_HANDLE;
		}

		p->fd_rwlock = open(
			p->file_name,
			O_CREAT | O_EXCL | O_RDWR,
			S_IRWXU );
		if ( -1 == p->fd_rwlock )
		{
			SKY_LOG_TRACE(
				"%s: fail to create file %s, error = %d\n",
				__func__,
				p->file_name,
				errno );
			SKY_MEM_SAFE_FREE( p->file_name );
			SKY_MEM_SAFE_FREE( p );
			return SKY_INVALID_HANDLE;
		}

		return p;
	}
	else
	{
		rc = pthread_rwlock_init( &p->rwlock, NULL );
		if ( rc != 0 )
		{
			SKY_MEM_SAFE_FREE( p );
			return SKY_INVALID_HANDLE;
		}

		return p;
	}
}

SKY_API SKY_HANDLE sky_rwlock_open ( char const * name )
{
	SKY_HANDLE_RWLOCK * p;
	char rwlock_name[ SKY_NAME_MAX + 1 ];

	RVON( name, SKY_INVALID_HANDLE );

	RVOF( sky_str_len( name ) > 0, SKY_INVALID_HANDLE );
	RVOF( sky_str_printf(
			rwlock_name,
			SKY_NAME_MAX,
			RWLOCK_NAME_PATTERN,
			name ) > 0,
			SKY_INVALID_HANDLE );

	p = sky_mem_alloc( sizeof( SKY_HANDLE_RWLOCK ) );
	RVON( p, SKY_INVALID_HANDLE );
	p->cb = sizeof( SKY_HANDLE_RWLOCK );
	p->type = SKY_HANDLE_TYPE_RWLOCK;

	p->fd_rwlock = open( rwlock_name, O_RDWR );
	if ( -1 == p->fd_rwlock )
	{
		SKY_LOG_TRACE(
			"%s: fail to create file %s, error = %d\n",
			__func__,
			rwlock_name,
			errno );
		SKY_MEM_SAFE_FREE( p );
		return SKY_INVALID_HANDLE;
	}

	return p;
}

void sky_rwlock_destroy ( SKY_HANDLE rwlock )
{
	SKY_HANDLE_RWLOCK * p = (SKY_HANDLE_RWLOCK*) rwlock;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_RWLOCK, rwlock );

	if ( p != NULL )
	{
		if ( p->fd_rwlock != -1 )
		{
			close( p->fd_rwlock );
			p->fd_rwlock = -1;
			if ( p->file_name != NULL )
			{
				remove( p->file_name );
				SKY_MEM_SAFE_FREE( p->file_name );
			}
		}
		else
		{
			pthread_rwlock_destroy( &p->rwlock );
		}
	}

	SKY_MEM_SAFE_FREE( p );
}

SKY_API bool sky_rwlock_wrlock ( SKY_HANDLE rwlock, uint32_t timeout )
{
	SKY_HANDLE_RWLOCK * p = (SKY_HANDLE_RWLOCK*) rwlock;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_RWLOCK, rwlock );
	struct timespec ts;
	int rc = -1;

	if ( p != NULL )
	{
		if ( p->fd_rwlock != -1 )
		{
			if ( SKY_INFINITE == timeout )
			{
				rc = flock( p->fd_rwlock, LOCK_EX );
			}
			else if ( 0 == timeout )
			{
				rc = flock( p->fd_rwlock, LOCK_EX | LOCK_NB );
			}
			else
			{
				uint32_t tick = sky_systick_get();

				do
				{
					rc = flock( p->fd_rwlock, LOCK_EX | LOCK_NB );
					if ( rc != 0 )
					{
						sky_systick_sleep( 100 );
					}

				} while ( rc != 0 && sky_systick_elapsed( tick ) < timeout );
			}
		}
		else
		{
			if ( SKY_INFINITE == timeout )
			{
				rc = pthread_rwlock_wrlock( &p->rwlock );
			}
			else if ( 0 == timeout )
			{
				rc = pthread_rwlock_trywrlock( &p->rwlock );
			}
			else
			{
				sky_abstime_after_msec( timeout, &ts );
				rc = pthread_rwlock_timedwrlock( &p->rwlock, &ts );
			}
		}
	}

	return ( 0 == rc );
}

SKY_API bool sky_rwlock_rdlock ( SKY_HANDLE rwlock, uint32_t timeout )
{
	SKY_HANDLE_RWLOCK * p = (SKY_HANDLE_RWLOCK*) rwlock;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_RWLOCK, rwlock );
	struct timespec ts;
	int rc = -1;

	if ( p != NULL )
	{
		if ( p->fd_rwlock != -1 )
		{
			if ( SKY_INFINITE == timeout )
			{
				rc = flock( p->fd_rwlock, LOCK_SH );
			}
			else if ( 0 == timeout )
			{
				rc = flock( p->fd_rwlock, LOCK_SH | LOCK_NB );
			}
			else
			{
				uint32_t tick = sky_systick_get();

				do
				{
					rc = flock( p->fd_rwlock, LOCK_SH | LOCK_NB );
					if ( rc != 0 )
					{
						sky_systick_sleep( 100 );
					}

				} while ( rc != 0 && sky_systick_elapsed( tick ) < timeout );
			}
		}
		else
		{
			if ( SKY_INFINITE == timeout )
			{
				rc = pthread_rwlock_rdlock( &p->rwlock );
			}
			else if ( 0 == timeout )
			{
				rc = pthread_rwlock_tryrdlock( &p->rwlock );
			}
			else
			{
				sky_abstime_after_msec( timeout, &ts );
				rc = pthread_rwlock_timedrdlock( &p->rwlock, &ts );
			}
		}
	}

	return ( 0 == rc );
}

SKY_API void sky_rwlock_unlock ( SKY_HANDLE rwlock )
{
	SKY_HANDLE_RWLOCK * p = (SKY_HANDLE_RWLOCK*) rwlock;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_RWLOCK, rwlock );

	if ( p != NULL )
	{
		if ( p->fd_rwlock != -1 )
		{
			flock( p->fd_rwlock, LOCK_UN );
		}
		else
		{
			pthread_rwlock_unlock( &p->rwlock );
		}
	}
}


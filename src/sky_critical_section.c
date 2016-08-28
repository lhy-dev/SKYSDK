#include <pthread.h>
#include "sky.h"

struct sky_handle_critical_section
{
	uint16_t cb;
	uint16_t type;
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;
};
typedef struct sky_handle_critical_section  SKY_HANDLE_CRITICAL_SECTION;

bool sky_critical_section_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_critical_section_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_critical_section_create ( void )
{
	SKY_HANDLE_CRITICAL_SECTION * p = sky_mem_alloc( sizeof( SKY_HANDLE_CRITICAL_SECTION ) );
	if ( NULL == p )
	{
		return SKY_INVALID_HANDLE;
	}

	p->cb = sizeof( SKY_HANDLE_CRITICAL_SECTION );
	p->type = SKY_HANDLE_TYPE_CRITICAL_SECTION;

	pthread_mutexattr_init( &p->attr );
	pthread_mutexattr_settype( &p->attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &p->mutex, &p->attr );

	return p;
}

void sky_critical_section_destroy ( SKY_HANDLE crit_sect )
{
	SKY_HANDLE_CRITICAL_SECTION * p = (SKY_HANDLE_CRITICAL_SECTION*) crit_sect;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_CRITICAL_SECTION, crit_sect );

	if ( p != NULL )
	{
		pthread_mutex_destroy( &p->mutex );
		pthread_mutexattr_destroy( &p->attr );
		sky_mem_free( p );
	}
}

SKY_API void sky_critical_section_enter ( SKY_HANDLE crit_sect )
{
	SKY_HANDLE_CRITICAL_SECTION * p = (SKY_HANDLE_CRITICAL_SECTION*) crit_sect;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_CRITICAL_SECTION, crit_sect );

	if ( p != NULL )
	{
		pthread_mutex_lock( &p->mutex );
	}
}

SKY_API void sky_critical_section_leave ( SKY_HANDLE crit_sect )
{
	SKY_HANDLE_CRITICAL_SECTION * p = (SKY_HANDLE_CRITICAL_SECTION*) crit_sect;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_CRITICAL_SECTION, crit_sect );

	if ( p != NULL )
	{
		pthread_mutex_unlock( &p->mutex );
	}
}

SKY_API bool sky_critical_section_try ( SKY_HANDLE crit_sect )
{
	SKY_HANDLE_CRITICAL_SECTION * p = (SKY_HANDLE_CRITICAL_SECTION*) crit_sect;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_CRITICAL_SECTION, crit_sect );

	if ( p != NULL )
	{
		if ( pthread_mutex_trylock( &p->mutex ) == 0 )
		{
			return true;
		}
	}

	return false;
}

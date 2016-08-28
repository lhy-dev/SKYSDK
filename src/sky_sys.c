#include <endian.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "sky.h"

bool sky_sys_init ( void )
{
	struct rlimit rlim;
	int ret;

	SKY_LOG_TRACE( "%s\n", __func__ );

	rlim.rlim_cur = 1024 * 1024 * 100;	//RLIM_INFINITY;
	rlim.rlim_max = 1024 * 1024 * 100;	//RLIM_INFINITY;
	ret = setrlimit( RLIMIT_CORE, &rlim );
	SKY_LOG_TRACE( "%s: RLIMIT_CORE returned %d\n", __func__, ret );

	return true;
}

void sky_sys_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API uint32_t sky_systick_get ( void )
{
	uint32_t tick;
	struct timespec tp = { 0, 0 };

	do
	{
		clock_gettime( CLOCK_MONOTONIC, &tp );
		tick = tp.tv_sec * 1000 + tp.tv_nsec / ( 1000 * 1000 );
	} while ( 0 == tick );

	return tick;
}

SKY_API uint32_t sky_systick_diff ( uint32_t early_tick, uint32_t later_tick )
{
	uint32_t diff_tick = 0;

	if ( early_tick != 0 )
	{
		if ( later_tick < early_tick )
		{
			diff_tick = UINT32_MAX - early_tick;
			diff_tick += later_tick;
			diff_tick += 1;
		}
		else
		{
			diff_tick = later_tick - early_tick;
		}
	}
	else
	{
		diff_tick = later_tick;
	}

	return diff_tick;
}

SKY_API uint32_t sky_systick_elapsed ( uint32_t last_tick )
{
	return sky_systick_diff( last_tick, sky_systick_get() );
}

SKY_API void sky_systick_sleep ( uint32_t milliseconds )
{
	milliseconds = max( 1, milliseconds );

	if ( milliseconds < 1000 )
	{
		usleep( milliseconds * 1000 );
	}
	else
	{
		struct timespec req;
		struct timespec rem;
		int rc;

		req.tv_sec = milliseconds / 1000;
		req.tv_nsec = milliseconds % 1000;
		req.tv_nsec *= ( 1000 * 1000 );

		while ( 1 )
		{
			rc = nanosleep( &req, &rem );
			if ( 0 == rc )
			{
				break;
			}

			if ( -1 == rc && EINTR == errno )
			{
				// The pause has been interrupted by a signal that was
				// delivered to the thread
				sky_mem_copy( &req, &rem, sizeof( req ) );
			}
			else
			{
				break;
			}
		}
	}
}

SKY_API uint64_t sky_time_get_ctime ()
{
	return time( NULL );
}

SKY_API bool sky_time_get ( SKY_TIME * time )
{
	struct timespec ts;
	int rc;

	RFON( time );

	rc = clock_gettime( CLOCK_REALTIME_COARSE, &ts );

	if ( 0 == rc )
	{
		time->time = ts.tv_sec;
		time->useconds = ts.tv_nsec / 1000;
	}

	return ( 0 == rc );
}

SKY_API bool sky_time_set ( SKY_TIME * time )
{
	struct timespec ts;
	int rc;

	RFON( time );

	ts.tv_sec = time->time;
	ts.tv_nsec = time->useconds * 1000;

	rc = clock_settime( CLOCK_REALTIME_COARSE, &ts );

	return ( 0 == rc );
}

static void tm_time_to_sys_time (
	struct tm * tm_time,
	SKY_SYSTEM_TIME * sys_time )
{
	sys_time->year			= tm_time->tm_year + 1900;
	sys_time->month			= tm_time->tm_mon + 1;
	sys_time->day_of_week	= tm_time->tm_wday + 1;
	sys_time->day_of_year	= tm_time->tm_yday + 1;
	sys_time->day			= tm_time->tm_mday;
	sys_time->hour			= tm_time->tm_hour;
	sys_time->minute		= tm_time->tm_min;
	sys_time->second		= tm_time->tm_sec;
}

static void sys_time_to_tm_time (
	SKY_SYSTEM_TIME * sys_time,
	struct tm * tm_time )
{
	sky_mem_clear( tm_time, sizeof( struct tm ) );

	tm_time->tm_year		= sys_time->year - 1900;
	tm_time->tm_mon			= sys_time->month - 1;
	tm_time->tm_wday		= sys_time->day_of_week - 1;
	tm_time->tm_yday		= sys_time->day_of_year - 1;
	tm_time->tm_mday		= sys_time->day;
	tm_time->tm_hour		= sys_time->hour;
	tm_time->tm_min			= sys_time->minute;
	tm_time->tm_sec			= sys_time->second;
}

SKY_API bool sky_time_get_localtime ( SKY_SYSTEM_TIME * sys_time )
{
	struct timeval tv;
	struct tm tm_time;

	SKY_ASSERT( sys_time != NULL );

	tzset();

	if ( gettimeofday( &tv, NULL ) != 0 )
	{
		return false;
	}

	if ( localtime_r( &tv.tv_sec, &tm_time ) == NULL )
	{
		return false;
	}

	sys_time->timezone = timezone / 60;
	sys_time->dstflag = 0;
	sys_time->milliseconds = tv.tv_usec / 1000;
	tm_time_to_sys_time( &tm_time, sys_time );
	return true;
}

SKY_API bool sky_time_get_gmtime ( SKY_SYSTEM_TIME * sys_time )
{
	struct timeval tv;
	struct tm tm_time;

	SKY_ASSERT( sys_time != NULL );

	tzset();

	if ( gettimeofday( &tv, NULL ) != 0 )
	{
		return false;
	}

	if ( gmtime_r( &tv.tv_sec, &tm_time ) == NULL )
	{
		return false;
	}

	sys_time->timezone = timezone / 60;
	sys_time->dstflag = 0;
	sys_time->milliseconds = tv.tv_usec / 1000;
	tm_time_to_sys_time( &tm_time, sys_time );
	return true;
}   

SKY_API bool sky_time_to_sys_time (
	SKY_TIME * time,
	SKY_SYSTEM_TIME * sys_time )
{
	struct tm tm_time;
	time_t t = time->time;

	if ( gmtime_r( &t, &tm_time ) == NULL )
	{
		return false;
	}

	sys_time->timezone = 0;
	sys_time->dstflag = 0;
	sys_time->milliseconds = time->useconds / 1000;
	tm_time_to_sys_time( &tm_time, sys_time );
	return true;
}

SKY_API bool sky_time_from_sys_time (
	SKY_TIME * time,
	SKY_SYSTEM_TIME * sys_time )
{
	struct tm tm_time;
	time_t t;

	sys_time_to_tm_time( sys_time, &tm_time );
	tm_time.tm_isdst = sys_time->dstflag;

	t = mktime( &tm_time );
	if ( -1 == t )
	{
		return false;
	}
	time->time = t;
	time->useconds = sys_time->milliseconds * 1000;

	return true;
}

SKY_API int32_t sky_time_utc_offset ( void )
{
	tzset();
	return timezone / 60;
}

void sky_abstime_after_msec (
	uint32_t milliseconds,
	struct timespec *abs_time )
{
	struct timeval tv;

	if ( gettimeofday( &tv, NULL ) != 0 )
	{
#if VERBOSE_DEBUG_INFO
		SKY_LOG_FATAL(
			"%s: gettimeofday failed! errno = %d\n",
			__func__,
			errno );
#endif
	}

#if VERBOSE_DEBUG_INFO
	SKY_LOG_DEBUG(
		"%s: gettimeofday sec = %u, usec = %u\n",
		__func__,
		tv.tv_sec,
		tv.tv_usec );
#endif

	tv.tv_sec += milliseconds / 1000;
	tv.tv_usec += ( milliseconds % 1000 ) * 1000;
	tv.tv_sec += tv.tv_usec / ( 1000 * 1000 );
	tv.tv_usec = tv.tv_usec % ( 1000 * 1000 );

#if VERBOSE_DEBUG_INFO
	SKY_LOG_DEBUG(
		"%s: after add %u ms, sec = %u, usec = %u\n",
		__func__,
		milliseconds,
		tv.tv_sec,
		tv.tv_usec );
#endif

	abs_time->tv_sec = tv.tv_sec;
	abs_time->tv_nsec = tv.tv_usec * 1000;
}

SKY_API int sky_sys_get_process_name ( char * buf, uint32_t len )
{
	int rc;
	char link_target[ SKY_PATH_MAX + 1 ];
	char* last_slash;
	size_t path_len;

	RVON( buf, -1 );
	RVOF( len > 1, -2 );

	/* Read the target of the symbolic link /proc/self/exe.  */
	rc = readlink(
		"/proc/self/exe",
		link_target,
		sizeof( link_target ) - 1 );
	if ( -1 == rc )
	{
		return -3;
	}

	link_target[ rc ] = '\0';

	last_slash = strrchr( link_target, '/' );
	if ( NULL == last_slash || last_slash == link_target )
	{
		return -4;
	}

	last_slash += 1;
	path_len = rc - ( last_slash - link_target );
	if ( path_len + 1 > len )
	{
		return -5;
	}

	sky_mem_copy( buf, last_slash, path_len );
	buf[ path_len ] = '\0';

	return path_len;
}

SKY_API int sky_sys_get_process_directory ( char * buf, uint32_t len )
{
	int rc;
	char link_target[ SKY_PATH_MAX + 1 ];
	char* last_slash;
	size_t path_len;

	RVON( buf, -1 );
	RVOF( len > 1, -2 );

	/* Read the target of the symbolic link /proc/self/exe.  */
	rc = readlink(
		"/proc/self/exe",
		link_target,
		sizeof( link_target ) - 1 );
	if ( -1 == rc )
	{
		return -3;
	}

	link_target[ rc ] = '\0';

	last_slash = strrchr( link_target, '/' );
	if ( NULL == last_slash || last_slash == link_target )
	{
		return -4;
	}

	path_len = last_slash - link_target;
	if ( path_len + 1 > len )
	{
		return -5;
	}

	sky_mem_copy( buf, link_target, path_len );
	buf[ path_len ] = '\0';

	return path_len;
}

SKY_API char const * sky_sys_get_model_name ( void )
{
	return MODULE_NAME;
}

SKY_API int sky_sys_get_node_id ()
{
	return -1;
}

SKY_API bool sky_sys_get_device_uid ( uint8_t * buf, uint32_t * size )
{
	return false;
}

SKY_API bool sky_sys_is_little_endian ( void )
{
	return ( BYTE_ORDER == LITTLE_ENDIAN );
}

SKY_API bool sky_sys_is_big_endian ( void )
{
	return ( BYTE_ORDER == BIG_ENDIAN );
}

SKY_API bool sky_wdt_set_timeout ( uint32_t timeout )
{
	return true;
}

SKY_API uint32_t sky_wdt_get_timeout ( void )
{
	return 0;
}

SKY_API void sky_wdt_reset ( void )
{
}

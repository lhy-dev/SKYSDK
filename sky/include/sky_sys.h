#ifndef SKY_SYS_H
#define SKY_SYS_H

#include "sky_define.h"

#ifndef MODULE_NAME
#define MODULE_NAME						"NONAME_MODULE"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	enum sky_dst_flag
	{
		SKY_DST_NONE,		/* not on DST */
		SKY_DST_USA,		/* USA style DST */
		SKY_DST_AUST,		/* Australian style DST */
		SKY_DST_WET,		/* Western European DST */
		SKY_DST_MET,		/* Middle European DST */
		SKY_DST_EET,		/* Eastern European DST */
		SKY_DST_CAN,		/* Canada */
		SKY_DST_GB,		/* Great Britain and Eire */
		SKY_DST_RUM,		/* Romania */
		SKY_DST_TUR,		/* Turkey */
		SKY_DST_AUSTALT,	/* Australian style with shift in 1986 */
	};
	typedef enum sky_dst_flag			SKY_DST_FLAG;

	struct sky_time
	{
	    uint64_t time;		/* seconds since 1970-01-01 00:00:00 +0000 (UTC) */
	    uint32_t useconds;	/* microseconds */
	};
	typedef struct sky_time			SKY_TIME;

	struct sky_system_time
	{
		uint16_t year;					/* 1900~2038 */
		uint16_t month;					/* 1~12 */
		uint16_t day_of_week;			/* 1~7 */
		uint16_t day_of_year;			/* 1~366 */
		uint16_t day;					/* 1~31 */
		uint16_t hour;					/* 0~23 */
		uint16_t minute;				/* 0~59 */
		uint16_t second;				/* 0~59 */
		uint16_t milliseconds;			/* 0~999 */
		int16_t	timezone;				/* minutes west of Greenwich */
		int16_t dstflag;				/* positive if daylight saving time is
										* in effect, zero if it is not, and
										* negative if the information is not
										*  available. */
	};
	typedef struct sky_system_time		SKY_SYSTEM_TIME;


	SKY_API uint32_t sky_systick_get ( void );

	SKY_API uint32_t sky_systick_diff (
		uint32_t early_tick,
		uint32_t later_tick );

	SKY_API uint32_t sky_systick_elapsed ( uint32_t last_tick );

	SKY_API void sky_systick_sleep ( uint32_t milliseconds );

	SKY_API uint64_t sky_time_get_ctime ( void );	// return time_t

	SKY_API bool sky_time_get ( SKY_TIME * time );

	SKY_API bool sky_time_set ( SKY_TIME * time );

	SKY_API bool sky_time_get_localtime ( SKY_SYSTEM_TIME * sys_time );

	SKY_API bool sky_time_get_gmtime ( SKY_SYSTEM_TIME * sys_time );

	SKY_API bool sky_time_to_sys_time (
		SKY_TIME * time,
		SKY_SYSTEM_TIME * sys_time );

	SKY_API bool sky_time_from_sys_time (
		SKY_TIME * time,
		SKY_SYSTEM_TIME * sys_time );

	SKY_API int32_t sky_time_utc_offset ( void );

	SKY_API bool sky_time_daylight_saving_status ( void );

	SKY_API int sky_sys_get_process_name ( char * buf, uint32_t len );

	SKY_API int sky_sys_get_process_directory ( char * buf, uint32_t len );

	SKY_API char const * sky_sys_get_model_name ( void );

	SKY_API int sky_sys_get_node_id ();

	SKY_API bool sky_sys_get_device_uid ( uint8_t * buf, uint32_t * size );

	SKY_API bool sky_sys_is_little_endian ( void );

	SKY_API bool sky_sys_is_big_endian ( void );

	/* in milliseconds, 0 to disable */
	SKY_API bool sky_wdt_set_timeout ( uint32_t timeout );

	SKY_API uint32_t sky_wdt_get_timeout ( void );

	SKY_API void sky_wdt_reset ( void ); /* feed watchdog */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

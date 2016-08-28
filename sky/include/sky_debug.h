#ifndef SKY_DEBUG_H
#define SKY_DEBUG_H

#include "sky_define.h"
#include <assert.h>

#define SKY_LOG_LEVEL_ENVNAME			"SKY_LOG_LEVEL"

#ifndef SKY_LOG_LEVEL_DEBUG
#define SKY_LOG_LEVEL_DEBUG			6
#endif

#ifndef SKY_LOG_LEVEL_TRACE
#define SKY_LOG_LEVEL_TRACE			5
#endif

#ifndef SKY_LOG_LEVEL_WARN
#define SKY_LOG_LEVEL_WARN			4
#endif

#ifndef SKY_LOG_LEVEL_ERROR
#define SKY_LOG_LEVEL_ERROR			3
#endif

#ifndef SKY_LOG_LEVEL_FATAL
#define SKY_LOG_LEVEL_FATAL			2
#endif

#define SKY_LOG_DEBUG	sky_log_level < SKY_LOG_LEVEL_DEBUG ? 0 : sky_log_debug
#define SKY_LOG_TRACE	sky_log_level < SKY_LOG_LEVEL_TRACE ? 0 : sky_log_trace
#define SKY_LOG_WARN	sky_log_level < SKY_LOG_LEVEL_WARN  ? 0 : sky_log_warn
#define SKY_LOG_ERROR	sky_log_level < SKY_LOG_LEVEL_ERROR ? 0 : sky_log_error
#define SKY_LOG_FATAL	sky_log_level < SKY_LOG_LEVEL_FATAL ? 0 : sky_log_fatal

#define SKY_LOG_PRINT_BUF	sky_log_level < SKY_LOG_LEVEL_TRACE ? 0 : SKY_log_print_buf


#ifdef DEBUG
#define SKY_ASSERT						running_unit_test?(void)0:assert
#else
#define SKY_ASSERT						1?(void)0:assert
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API int sky_log_debug ( char const * fmt, ... );

	SKY_API int sky_log_trace ( char const * fmt, ... );

	SKY_API int sky ( char const * fmt, ... );

	SKY_API int sky_log_error ( char const * fmt, ... );

	SKY_API int sky_log_fatal ( char const * fmt, ... );

	SKY_API int sky_log_print_buf ( void * buf, uint32_t len );

	extern int sky_log_level;
	extern bool running_unit_test;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

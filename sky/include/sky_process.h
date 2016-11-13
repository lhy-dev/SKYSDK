#ifndef SKY_PROCESS_H
#define SKY_PROCESS_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API bool sky_process_is_first_instance ( char const * proc_name );

	SKY_API char const * sky_process_get_name ( void );

	SKY_API char const * sky_process_get_short_name ( void );

	SKY_API bool sky_process_set_run_in_background ( void );

	SKY_API bool sky_process_is_run_in_background ( void );

	SKY_API char * sky_process_get_env ( char const * name );

	SKY_API int sky_process_set_env (
		char const * name,
		char const * value,
		int overwrite );

	SKY_API int sky_process_clear_env ( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

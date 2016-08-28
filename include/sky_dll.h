#ifndef SKY_DLL_H
#define SKY_DLL_H

#include "sky_define.h"



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API SKY_HANDLE sky_dll_load ( char const * dll_name );

	SKY_API void * sky_dll_get_function (
		SKY_HANDLE dll_handle,
		char const * func_name );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

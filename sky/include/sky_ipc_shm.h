#ifndef SKY_SHARE_MEMORY_H
#define SKY_SHARE_MEMORY_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	SKY_API SKY_HANDLE sky_share_memory_create (
		char const * name,
		uint32_t size );

	SKY_API SKY_HANDLE sky_share_memory_open ( char const * name );

	SKY_API void * sky_share_memory_get_buffer ( SKY_HANDLE share_mem );

	SKY_API uint32_t sky_share_memory_size ( SKY_HANDLE share_mem );

#ifdef __cplusplus
}
#endif

#endif

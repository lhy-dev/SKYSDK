#ifndef SKY_MEM_H
#define SKY_MEM_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API void * sky_mem_alloc ( uint32_t size );

	SKY_API void sky_mem_free ( void * buf );

	SKY_API void sky_mem_set ( void * p, uint8_t value, uint32_t size );

	SKY_API void sky_mem_clear ( void * p, uint32_t size );

	SKY_API void sky_mem_copy ( void * dst, void const * src, uint32_t size );

	SKY_API void sky_mem_move ( void * dst, void const * src, uint32_t size );

	SKY_API int sky_mem_cmp (
		void const * buf1,
		void const * buf2,
		uint32_t size );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

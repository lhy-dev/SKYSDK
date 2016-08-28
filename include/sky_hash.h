#ifndef SKY_HASH_H
#define SKY_HASH_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API SKY_HANDLE sky_hash_create ( uint32_t size );

	SKY_API void * sky_hash_add (
		SKY_HANDLE hash,
		char const * key,
		void const * value );

	SKY_API bool sky_hash_find (
		SKY_HANDLE hash,
		char const * key,
		void ** value );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

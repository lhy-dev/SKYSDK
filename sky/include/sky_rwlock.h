#ifndef SKY_RWLOCK_H
#define SKY_RWLOCK_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API SKY_HANDLE sky_rwlock_create ( char const * name );

	SKY_API SKY_HANDLE sky_rwlock_open ( char const * name );

	SKY_API bool sky_rwlock_wrlock ( SKY_HANDLE rwlock, uint32_t timeout );

	SKY_API bool sky_rwlock_rdlock ( SKY_HANDLE rwlock, uint32_t timeout );

	SKY_API void sky_rwlock_unlock ( SKY_HANDLE rwlock );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

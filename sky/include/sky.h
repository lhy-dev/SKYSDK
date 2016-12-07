#ifndef SKY_H
#define SKY_H

#include "sky_define.h"
#include "sky_debug.h"
#include "sky_list.h"
#include "sky_hash.h"
#include "sky_mem.h"
#include "sky_str.h"
#include "sky_sys.h"
#include "sky_regex.h"
#include "sky_process.h"
#include "sky_ipc_shm.h"
#include "sky_base64.h"
#include "sky_rwlock.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void* HANDLE;

SKY_API bool sky_init(void);

SKY_API void sky_uninit(void);

SKY_API void close_handle(HANDLE handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

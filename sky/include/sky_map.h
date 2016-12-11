
#ifndef SKY_MAP_H
#define SKY_MAP_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stdbool.h>
#endif
#include "sky_define.h"

//typedef struct SKY_HANDLE_DATA_TAG*  SKY_HANDLE;

typedef int (*MAP_FILTER_CALLBACK)(const char* mapProperty,const char* mapValue);

SKY_API SKY_HANDLE sky_map_create( MAP_FILTER_CALLBACK mapFilterFunc);

//SKY_API void sky_map_destroy(SKY_HANDLE handle);

SKY_API SKY_HANDLE sky_map_clone(SKY_HANDLE handle);

SKY_API int sky_map_add(SKY_HANDLE handle, const char* key, const char* value);

SKY_API int sky_map_addorupdate(SKY_HANDLE handle, const char* key, const char* value);

SKY_API int sky_map_delete(SKY_HANDLE handle, const char* key);

SKY_API int sky_map_containskey(SKY_HANDLE handle, const char* key, bool * keyExists);

SKY_API int sky_map_containsvalue(SKY_HANDLE handle, const char* value, bool* valueExists);

SKY_API const char* sky_map_getvaluefromkey(SKY_HANDLE handle, const char* key);

#ifdef __cplusplus
}
#endif

#endif /* MAP_H */

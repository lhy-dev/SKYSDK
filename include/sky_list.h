#ifndef SKY_LIST_H
#define SKY_LIST_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef bool (*SKY_LIST_CMP_FUNC) (
		void const * item_data,
		void const * param1,
		void const * param2 );

	SKY_API SKY_HANDLE sky_list_create ( uint32_t max_size );

	SKY_API uint32_t sky_list_size ( SKY_HANDLE list );

	SKY_API uint32_t sky_list_max_size ( SKY_HANDLE list );

	SKY_API bool sky_list_full ( SKY_HANDLE list );

	SKY_API bool sky_list_empty ( SKY_HANDLE list );

	SKY_API void * sky_list_front ( SKY_HANDLE list );

	SKY_API void * sky_list_back ( SKY_HANDLE list );

	SKY_API bool sky_list_push_front ( SKY_HANDLE list, void * data );

	SKY_API void * sky_list_pop_front ( SKY_HANDLE list );

	SKY_API bool sky_list_push_back ( SKY_HANDLE list, void * data );

	SKY_API void * sky_list_pop_back ( SKY_HANDLE list );

	SKY_API void * sky_list_get_at ( SKY_HANDLE list, uint32_t index );

	SKY_API bool sky_list_insert (
		SKY_HANDLE list,
		uint32_t index,
		void * data );

	SKY_API void * sky_list_remove ( SKY_HANDLE list, void const * data );

	SKY_API void * sky_list_remove_if (
		SKY_HANDLE list,
		SKY_LIST_CMP_FUNC cmp_func,
		void const * param1,
		void const * param2 );

	SKY_API bool sky_list_contain ( SKY_HANDLE list, void const * data );

	SKY_API void * sky_list_find_if (
		SKY_HANDLE list,
		SKY_LIST_CMP_FUNC cmp_func,
		void const * param1,
		void const * param2 );

	SKY_API void sky_list_clear ( SKY_HANDLE list );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

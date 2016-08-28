#include "sky.h"

#ifndef SKY_LIST_PREALLOC_NUM
#define SKY_LIST_PREALLOC_NUM            0
#endif

struct sky_list_item
{
	void * data;
	struct sky_list_item * next;
	struct sky_list_item * prev;
};
typedef struct sky_list_item          SKY_LIST_ITEM;

struct sky_handle_list
{
	uint16_t cb;
	uint16_t handle_type;
	SKY_LIST_ITEM head;
	SKY_LIST_ITEM tail;
	SKY_LIST_ITEM * prealloc;
	uint32_t count;
	uint32_t max_size;
};
typedef struct sky_handle_list        SKY_HANDLE_LIST;

bool sky_list_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_list_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_list_create ( uint32_t max_size )
{
	int i;
	SKY_HANDLE_LIST * list;
	SKY_LIST_ITEM * item;

	RVOF( max_size > 0, SKY_INVALID_HANDLE );

	list = sky_mem_alloc( sizeof( SKY_HANDLE_LIST ) );
	RVON( list, SKY_INVALID_HANDLE );

	list->cb = sizeof( SKY_HANDLE_LIST );
	list->handle_type = SKY_HANDLE_TYPE_LIST;
	list->head.data = NULL;
	list->head.prev = NULL;
	list->head.next = &list->tail;
	list->tail.data = NULL;
	list->tail.prev = &list->head;
	list->tail.next = NULL;
	list->prealloc = NULL;
	// Add pre-alloc item
	for ( i = 0; i < SKY_LIST_PREALLOC_NUM; i++ )
	{
		item = sky_mem_alloc( sizeof( SKY_LIST_ITEM ) );
		if ( NULL == item )
		{
			SKY_ASSERT( 0 );
			sky_mem_free( list );
			while ( list->prealloc != NULL )
			{
				item = list->prealloc->next;
				sky_mem_free( list->prealloc );
				list->prealloc = item;
			}
			return SKY_INVALID_HANDLE;
		}
		item->data = NULL;
		item->next = list->prealloc;
		list->prealloc = item;
	}
	list->count = 0;
	list->max_size = max_size;

	return list;
}

void sky_list_destroy ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * temp;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	if ( l->count != 0 )
	{
		SKY_LOG_WARN(
			"%s: list is not empty! count = %d\n",
			__func__,
			l->count );
	}

	while ( l->head.next != &l->tail )
	{
		temp = l->head.next;
		l->head.next = temp->next;
		sky_mem_free( temp );
		l->count -= 1;
	}

	while ( l->prealloc != NULL )
	{
		temp = l->prealloc->next;
		sky_mem_free( l->prealloc );
		l->prealloc = temp;
	}

	sky_mem_free( l );
}

SKY_API uint32_t sky_list_size ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	return l->count;
}

SKY_API uint32_t sky_list_max_size ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	return l->max_size;
}

SKY_API bool sky_list_full ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	return ( l->count == l->max_size );
}

SKY_API bool sky_list_empty ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	return ( l->count == 0 );
}

SKY_API void * sky_list_front ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	if ( l->head.next != &l->tail )
	{
		return l->head.next->data;
	}
	return NULL;
}

SKY_API void * sky_list_back ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );
	if ( l->tail.prev != &l->head )
	{
		return l->tail.prev->data;
	}
	return NULL;
}

SKY_API bool sky_list_push_front ( SKY_HANDLE list, void * data )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RFOF( l->count < l->max_size );

	if ( NULL == l->prealloc )
	{
		item = sky_mem_alloc( sizeof( SKY_LIST_ITEM ) );
	}
	else
	{
		item = l->prealloc;
		l->prealloc = item->next;
	}

	RFON( item );

	item->data = data;
	item->prev = &l->head;
	item->next = l->head.next;
	item->prev->next = item;
	item->next->prev = item;
	l->count += 1;

	return true;
}

SKY_API void * sky_list_pop_front ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RNOF( l->count > 0 );
	RNOF( l->head.next != &l->tail );

	item = l->head.next;

	l->head.next = item->next;
	item->next->prev = item->prev;

	l->count -= 1;

	// put back to prealloc list
	item->next = l->prealloc;
	l->prealloc = item;

	return item->data;
}

SKY_API bool sky_list_push_back ( SKY_HANDLE list, void * data )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RFOF( l->count < l->max_size );

	if ( NULL == l->prealloc )
	{
		item = sky_mem_alloc( sizeof( SKY_LIST_ITEM ) );
	}
	else
	{
		item = l->prealloc;
		l->prealloc = item->next;
	}

	RFON( item );

	item->data = data;
	item->prev = l->tail.prev;
	item->next = &l->tail;
	item->prev->next = item;
	item->next->prev = item;
	l->count += 1;

	return true;
}

SKY_API void * sky_list_pop_back ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RNOF( l->count > 0 );
	RNOF( l->head.next != &l->tail );

	item = l->tail.prev;

	l->tail.prev = item->prev;
	item->prev->next = item->next;

	l->count -= 1;

	// put back to prealloc list
	item->next = l->prealloc;
	l->prealloc = item;

	return item->data;
}

SKY_API void * sky_list_get_at ( SKY_HANDLE list, uint32_t index )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RNOF( l->count > 0 );
	RNOF( index < l->count );

	item = l->head.next;
	while ( item != NULL && index-- > 0 )
	{
		item = item->next;
	}

	return ( item != NULL ) ? item->data : NULL;
}

SKY_API bool sky_list_insert (
	SKY_HANDLE list,
	uint32_t index,
	void * data )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * item;
	SKY_LIST_ITEM * itor;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	RFOF( l->count < l->max_size );

	if ( NULL == l->prealloc )
	{
		SKY_ASSERT( 0 );   // list item count exceed the max pre-alloc number
		item = sky_mem_alloc( sizeof( SKY_LIST_ITEM ) );
	}
	else
	{
		item = l->prealloc;
		l->prealloc = item->next;
	}

	RFON( item );

	itor = l->head.next;
	while ( index-- && itor != NULL )
	{
		itor = itor->next;
	}

	if ( NULL == itor )
	{
		itor = &l->tail;
	}

	item->data = data;
	item->next = itor;
	item->prev = itor->prev;
	item->prev->next = item;
	item->next->prev = item;
	l->count += 1;

	return true;
}

static bool compare_data_ptr (
	void const * item_data,
	void const * param1,
	void const * param2 )
{
	return ( item_data == param1 );
}

SKY_API void * sky_list_remove ( SKY_HANDLE list, void const * data )
{
	return sky_list_remove_if( list, compare_data_ptr, data, NULL );
}

SKY_API void * sky_list_remove_if (
	SKY_HANDLE list,
	SKY_LIST_CMP_FUNC cmp_func,
	void const * param1,
	void const * param2 )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * itor;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	for ( itor = l->head.next; itor != &l->tail; itor = itor->next )
	{
		if ( cmp_func( itor->data, param1, param2 ) )
		{
			itor->prev->next = itor->next;
			itor->next->prev = itor->prev;
			l->count -= 1;

			// put back to the prealloc list
			itor->next = l->prealloc;
			l->prealloc = itor;
			break;
		}
	}

	return ( itor != &l->tail ) ? itor->data : NULL;
}

SKY_API bool sky_list_contain ( SKY_HANDLE list, void const * data )
{
	void * ret = sky_list_find_if( list, compare_data_ptr, data, NULL );

	return ( ret != NULL && ret == data );
}

SKY_API void * sky_list_find_if (
	SKY_HANDLE list,
	SKY_LIST_CMP_FUNC cmp_func,
	void const * param1,
	void const * param2 )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * itor;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	SKY_ASSERT( cmp_func != NULL );

	for ( itor = l->head.next; itor != &l->tail; itor = itor->next )
	{
		if ( cmp_func( itor->data, param1, param2 ) )
		{
			break;
		}
	}

	return ( itor != &l->tail ) ? itor->data : NULL;
}

SKY_API void sky_list_clear ( SKY_HANDLE list )
{
	SKY_HANDLE_LIST * l = (SKY_HANDLE_LIST*) list;
	SKY_LIST_ITEM * temp;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_LIST, list );

	while ( l->head.next != &l->tail )
	{
		temp = l->head.next->next;
		l->head.next->next = l->prealloc;
		l->prealloc = l->head.next;
		l->head.next = temp;
		l->count -= 1;
	}

	l->tail.prev = &l->head;
}

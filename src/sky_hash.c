#include <search.h>
#include <errno.h>
#include "sky.h"

struct key_string
{
	char * str;
	struct key_string * next;
};
typedef struct key_string			KEY_STRING;

struct sky_handle_hash
{
	uint16_t cb;
	uint16_t type;
	struct hsearch_data htab;
	KEY_STRING * key_array;
};
typedef struct sky_handle_hash  	SKY_HANDLE_HASH;


bool sky_hash_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_hash_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_hash_create ( uint32_t size )
{
	SKY_HANDLE_HASH * h;
	int rc;

	h = sky_mem_alloc( sizeof( SKY_HANDLE_HASH ) );
	if ( NULL == h )
	{
		SKY_LOG_ERROR(
			"%s: fail to alloc memory for hash object.\n",
			__func__ );
		return SKY_INVALID_HANDLE;
	}

	h->cb = sizeof( SKY_HANDLE_HASH );
	h->type = SKY_HANDLE_TYPE_HASH;
	h->key_array = NULL;
	rc = hcreate_r( size + 10, &h->htab );
	if ( 0 == rc )
	{
		SKY_LOG_ERROR(
			"%s: create hash object failed, error = %d\n",
			__func__,
			errno );
		sky_mem_free( h );
		return SKY_INVALID_HANDLE;
	}

	SKY_LOG_TRACE(
		"%s: size = %d, actual_size = %d\n",
		__func__,
		size,
		h->htab.size );

	return h;
}

void sky_hash_destroy ( SKY_HANDLE hash )
{
	KEY_STRING * next;
	SKY_HANDLE_HASH * h = (SKY_HANDLE_HASH*) hash;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_HASH, hash );

	if ( h != NULL )
	{
		while ( h->key_array != NULL )
		{
			next = h->key_array->next;
			SKY_MEM_SAFE_FREE( h->key_array->str );
			sky_mem_free( h->key_array );
			h->key_array = next;
		}

		hdestroy_r( &h->htab );
		sky_mem_free( h );
	}
}

SKY_API void * sky_hash_add (
	SKY_HANDLE hash,
	char const * key,
	void const * value )
{
	SKY_HANDLE_HASH * h = (SKY_HANDLE_HASH*) hash;
	ENTRY e;
	ENTRY *retval;
	int key_len;
	int rc;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_HASH, hash );

	RNON( key );

	key_len = sky_str_len( key );
	RNOF( key_len > 0 );

	e.key = (char*) key;
	e.data = (void*) value;
	retval = NULL;

	rc = hsearch_r( e, FIND, &retval, &h->htab );
	if ( rc != 0 )
	{
		// key already exist
		if ( retval != NULL )
		{
			// update the value
			retval->data = (void*) value;
		}
		else
		{
			SKY_ASSERT( 0 );
			return NULL;
		}
	}
	else
	{
		KEY_STRING * ks = sky_mem_alloc( sizeof( KEY_STRING ) );
		RNON( ks );
		ks->str = sky_mem_alloc( key_len + 1 );
		if ( NULL == ks->str )
		{
			sky_mem_free( ks );
			return NULL;
		}
		sky_str_cpy( ks->str, key, key_len + 1 );

		e.key = ks->str;
		e.data = (void*) value;
		retval = NULL;
		rc = hsearch_r( e, ENTER, &retval, &h->htab );
		if ( 0 == rc || NULL == retval )
		{
			SKY_LOG_ERROR(
				"%s: fail to add, key = %s, value = %p, error = %d\n",
				__func__,
				key,
				value,
				errno );
			sky_mem_free( ks->str );
			sky_mem_free( ks );
			return NULL;
		}

		ks->next = h->key_array;
		h->key_array = ks;
	}

	return retval;
}

SKY_API bool sky_hash_find (
	SKY_HANDLE hash,
	char const * key,
	void ** value )
{
	SKY_HANDLE_HASH * h = (SKY_HANDLE_HASH*) hash;
	ENTRY e;
	ENTRY *retval;
	int rc;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_HASH, hash );

	RFON( key );

	e.key = (char*) key;
	e.data = NULL;
	retval = NULL;
	rc = hsearch_r( e, FIND, &retval, &h->htab );
	if ( 0 == rc || NULL == retval )
	{
		return false;
	}

	if ( value != NULL )
	{
		*value = retval->data;
	}

	return true;
}

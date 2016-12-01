#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "sky.h"

#define MAX_SHARE_MEM_COUNT				1000

#define SHARE_MEM_NAME_PATTERN			"/%s"

struct sky_handle_share_mem
{
	uint16_t cb;
	uint16_t handle_type;
	int file;
	void * ptr;
	uint32_t size;
	bool is_creator;
	char name[ 128 ];
};
typedef struct sky_handle_share_mem	SKY_HANDLE_SHARE_MEM;

SKY_HANDLE share_mem_list = SKY_INVALID_HANDLE;
SKY_HANDLE share_mem_list_cs = SKY_INVALID_HANDLE;

bool sky_share_memory_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );

	if ( SKY_INVALID_HANDLE == share_mem_list )
	{
		share_mem_list = sky_list_create( MAX_SHARE_MEM_COUNT );
		RFOF( share_mem_list != SKY_INVALID_HANDLE );
	}

	if ( SKY_INVALID_HANDLE == share_mem_list_cs )
	{
		share_mem_list_cs = sky_critical_section_create();
		RFOF( share_mem_list_cs != SKY_INVALID_HANDLE );
	}

	return true;
}

void sky_share_memory_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );

	SKY_SAFE_CLOSE_HANDLE( share_mem_list );
	SKY_SAFE_CLOSE_HANDLE( share_mem_list_cs );
}

SKY_API void * sky_share_memory_create (
	char const * name,
	uint32_t size )
{
	SKY_HANDLE_SHARE_MEM * sm = NULL;

	SKY_LOG_DEBUG( "%s: name = %s, size = %u\n", __func__, name, size );

	if ( NULL == name )
	{
		SKY_ASSERT( 0 );
		return SKY_INVALID_HANDLE;
	}

	if ( sky_str_len( name ) > SKY_NAME_MAX - 1 )
	{
		SKY_LOG_ERROR( "%s: name = %s is too long \n", __func__, name );
		SKY_ASSERT( 0 );
		return SKY_INVALID_HANDLE;
	}

	do
	{
		sm = sky_mem_alloc( sizeof( SKY_HANDLE_SHARE_MEM ) );
		if ( NULL == sm )
		{
			SKY_LOG_TRACE(
				"%s: cannot allocate memory for share memory item!\n",
				__func__ );
			break;
		}

		sm->cb = sizeof( SKY_HANDLE_SHARE_MEM );
		sm->handle_type = SKY_HANDLE_TYPE_SHARE_MEMORY;

		if ( sky_str_printf(
			sm->name,
			sizeof( sm->name ),
			SHARE_MEM_NAME_PATTERN,
			name ) <= 0 )
		{
			SKY_LOG_ERROR(
				"%s: fail to copy name for share memory!\n",
				__func__ );
			break;
		}

		sm->file = shm_open(
			sm->name,
			O_CREAT | O_EXCL | O_RDWR,
			S_IRUSR | S_IWUSR );
		if ( -1 == sm->file )
		{
			SKY_LOG_INFO(
				"%s: Could not create share memory object %s, errno = %d.\n",
				__func__,
				sm->name,
				errno );
			break;
		}

		if ( ftruncate( sm->file, size ) == -1 )
		{
			SKY_LOG_ERROR(
				"%s: Could not set the size of the share memory object %s!\n",
				__func__,
				name );
			break;
		}

		sm->ptr = mmap(
			NULL,
			size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			sm->file,
			0 );
		if ( MAP_FAILED == sm->ptr )
		{
			SKY_LOG_ERROR(
				"%s : Could not map view of file (%d).\n",
				__func__,
				errno );
			break;
		}

		sm->size = size;
		sm->is_creator = true;

		// add to share memory list
		sky_critical_section_enter( share_mem_list_cs );
		if ( !sky_list_push_back( share_mem_list, sm ) )
		{
			sky_critical_section_leave( share_mem_list_cs );
			break;
		}
		sky_critical_section_leave( share_mem_list_cs );

		return sm;
	} while ( 0 );

	if ( sm != NULL )
	{
		if ( sm->ptr != MAP_FAILED )
		{
			munmap( sm->ptr, sm->size );
		}

		if ( sm->file != -1 )
		{
			close( sm->file );
			shm_unlink( sm->name );
		}
	}

	SKY_MEM_SAFE_FREE( sm );

	return SKY_INVALID_HANDLE;
}

SKY_API void * sky_share_memory_open ( char const * name )
{
	SKY_HANDLE_SHARE_MEM * sm = NULL;
	struct stat sm_stat;

	RVON( name, SKY_INVALID_HANDLE );

	if ( sky_str_len( name ) > SKY_NAME_MAX - 1 )
	{
		SKY_LOG_ERROR(
			"%s: The name of the share memory object is too long.\n",
			__func__ );
		SKY_ASSERT( 0 );
		return SKY_INVALID_HANDLE;
	}

	do
	{
		sm = sky_mem_alloc( sizeof( SKY_HANDLE_SHARE_MEM ) );
		if ( NULL == sm )
		{
			SKY_LOG_ERROR(
				"%s: cannot allocate memory for share memory item!\n",
				__func__ );
			break;
		}

		sm->cb = sizeof( SKY_HANDLE_SHARE_MEM );
		sm->handle_type = SKY_HANDLE_TYPE_SHARE_MEMORY;

		if ( sky_str_printf(
			sm->name,
			sizeof( sm->name ),
			SHARE_MEM_NAME_PATTERN,
			name ) <= 0 )
		{
			SKY_LOG_ERROR(
				"%s: fail to copy name for share memory!\n",
				__func__ );
			break;
		}

		sm->file = shm_open(
			sm->name,
			O_RDWR,
			S_IRUSR | S_IWUSR );
		if ( -1 == sm->file )
		{
			SKY_LOG_INFO(
				"%s: Could not open the share memory object %s.\n",
				__func__,
				sm->name );
			break;
		}

		if ( fstat( sm->file, &sm_stat ) == -1 )
		{
			SKY_LOG_ERROR(
				"%s: Could not get file status for share memory object %s.\n",
				__func__,
				sm->name );
			break;
		}

		sm->ptr = mmap(
			NULL,
			sm_stat.st_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			sm->file,
			0 );
		if ( MAP_FAILED == sm->ptr )
		{
			SKY_LOG_ERROR(
				"%s: Could not map view of file (%d).\n",
				__func__,
				errno );
			break;
		}

		sm->size = sm_stat.st_size;
		sm->is_creator = false;

		// add to share memory list
		sky_critical_section_enter( share_mem_list_cs );
		if ( !sky_list_push_back( share_mem_list, sm ) )
		{
			sky_critical_section_leave( share_mem_list_cs );
			break;
		}
		sky_critical_section_leave( share_mem_list_cs );

		return sm;
	} while ( 0 );

	if ( sm != NULL )
	{
		if ( sm->ptr != MAP_FAILED )
		{
			munmap( sm->ptr, sm->size );
		}

		if ( sm->file != -1 )
		{
			close( sm->file );
			shm_unlink( sm->name );
		}
	}

	SKY_MEM_SAFE_FREE( sm );

	return SKY_INVALID_HANDLE;
}

SKY_API void * sky_share_memory_get_buffer ( SKY_HANDLE share_mem )
{
	SKY_HANDLE_SHARE_MEM * p = (SKY_HANDLE_SHARE_MEM*) share_mem;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_SHARE_MEMORY, share_mem );

	return p->ptr;
}

SKY_API uint32_t sky_share_memory_size ( SKY_HANDLE share_mem )
{
	SKY_HANDLE_SHARE_MEM * p = (SKY_HANDLE_SHARE_MEM*) share_mem;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_SHARE_MEMORY, share_mem );

	return p->size;
}

bool sky_share_memory_destroy ( SKY_HANDLE share_mem )
{
	SKY_HANDLE_SHARE_MEM * p = (SKY_HANDLE_SHARE_MEM*) share_mem;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_SHARE_MEMORY, share_mem );

	sky_critical_section_enter( share_mem_list_cs );
	p = sky_list_remove( share_mem_list, p );
	sky_critical_section_leave( share_mem_list_cs );

	RFON( p );
	munmap( p->ptr, p->size );
	close( p->file );
	if ( p->is_creator )
	{
		shm_unlink ( p->name );
	}
	sky_mem_free( p );

	return true;
}


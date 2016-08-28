#include <dlfcn.h>
#include "sky.h"

struct sky_handle_dll
{
	uint16_t cb;
	uint16_t handle_type;
	void * dll_handle;
};
typedef struct sky_handle_dll			SKY_HANDLE_DLL;

bool sky_dll_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_dll_uninit ( void )
{
}

SKY_API SKY_HANDLE sky_dll_load ( char const * dll_name )
{
	SKY_HANDLE_DLL * dll = sky_mem_alloc( sizeof( SKY_HANDLE_DLL ) );

	RVON( dll, SKY_INVALID_HANDLE );

	dll->cb = sizeof( SKY_HANDLE_DLL );
	dll->handle_type = SKY_HANDLE_TYPE_DLL;
	dll->dll_handle = dlopen( dll_name, RTLD_NOW );
	if ( NULL == dll->dll_handle )
	{
		SKY_LOG_TRACE(
			"%s: fail to load %s\n",
			__func__,
			dlerror() );
		sky_mem_free( dll );
		return SKY_INVALID_HANDLE;
	}

	return dll;
}

void sky_dll_close ( SKY_HANDLE dll )
{
	SKY_HANDLE_DLL * d = (SKY_HANDLE_DLL*) dll;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_DLL, dll );

	if ( d != NULL )
	{
		if ( d->dll_handle != NULL )
		{
			dlclose( d->dll_handle );
		}

		sky_mem_free( d );
	}
}

SKY_API void * sky_dll_get_function (
	SKY_HANDLE dll_handle,
	char const * func_name )
{
	SKY_HANDLE_DLL * d = (SKY_HANDLE_DLL*) dll_handle;
	void * func;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_DLL, dll_handle );

	RNON( d );

	func = dlsym( d->dll_handle, func_name );
	if ( NULL == func )
	{
		SKY_LOG_TRACE(
			"%s: fail to get function %s, error = %s\n",
			__func__,
			func_name,
			dlerror() );
	}

	return func;
}

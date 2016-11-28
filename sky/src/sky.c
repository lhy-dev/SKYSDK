#include "sky.h"

bool sky_critical_section_init ( void );
void sky_critical_section_uninit ( void );
bool sky_debug_init ( void );
void sky_debug_uninit ( void );
bool sky_mem_init ( void );
void sky_mem_uninit ( void );
bool sky_list_init ( void );
void sky_list_uninit ( void );
bool sky_sys_init ( void );
void sky_sys_uninit ( void );
bool sky_str_init ( void );
void sky_str_uninit ( void );
bool sky_hash_init ( void );
void sky_hash_uninit ( void );
bool sky_regex_init( void );
void sky_regex_uninit( void );
bool sky_dll_init ( void );
void sky_dll_uninit ( void );
bool sky_process_init( void );
void sky_process_unit( void );
bool sky_rwlock_init( void );
void sky_rwlock_unit( void );

typedef bool (*init_func_type) ();
typedef void (*uninit_func_type) ();

const init_func_type INIT_FUNCS [] =
{
	sky_debug_init,
	sky_critical_section_init,
	sky_mem_init,
	sky_list_init,
	sky_sys_init,
	sky_str_init,
	sky_hash_init,
	sky_regex_init
	sky_dll_init,
	sky_process_init,
	sky_rwlock_init
	
};

const uninit_func_type UNINIT_FUNCS [] =
{
	sky_rwlock_unit,
	sky_process_unit,
	sky_dll_uninit,
	sky_regex_uninit
	sky_hash_uninit,
	sky_str_uninit,
	sky_sys_uninit,
	sky_list_uninit,
	sky_mem_init,
	sky_critical_section_uninit,
	sky_debug_uninit

};

const int INIT_FUNC_CNT = sizeof( INIT_FUNCS ) / sizeof( INIT_FUNCS[ 0 ] );
static int init_count = 0;

SKY_API bool sky_init ( void )
{
	int i,j;

	if ( 0 == init_count )
	{
		for ( i = 0; i < INIT_FUNC_CNT; i++ )
		{
			if ( !INIT_FUNCS[ i ]() )
			{
				for ( j = i - 1; j >= 0; j-- )
				{
					UNINIT_FUNCS[ j ]();
				}
				return false;
			}
		}
	}

	init_count++;

	return true;
}

SKY_API void sky_uninit ( void )
{
	int i;

	SKY_LOG_TRACE( "%s: init_count = %d\n", __func__, init_count );
	if ( 1 == init_count )
	{
		for ( i = 0; i < INIT_FUNC_CNT; i++ )
		{
			UNINIT_FUNCS[ i ]();
		}
	}
	init_count--;
}

void sky_critical_section_destroy ( SKY_HANDLE crit_sect );
void sky_list_destroy ( SKY_HANDLE list );
void sky_hash_destroy ( SKY_HANDLE hash );
void sky_regex_destroy(SKY_HANDLE regex );
void sky_dll_close ( SKY_HANDLE dll );
void sky_rwlock_destroy ( SKY_HANDLE rwlock );

SKY_API void sky_close_handle ( SKY_HANDLE object_handle )
{
	struct sky_handle * obj = (struct sky_handle*) object_handle;
	switch ( obj->type )
	{
	case SKY_HANDLE_TYPE_CRITICAL_SECTION:
			sky_critical_section_destroy( object_handle );
			break;
	case SKY_HANDLE_TYPE_LIST:
		sky_list_destroy( object_handle );
		break;

	case SKY_HANDLE_TYPE_HASH:
		sky_hash_destroy ( object_handle );
		break;
	case SKY_HANDLE_TYPE_REGEX:
		sky_regex_destroy(object_handle );
		break;
	case SKY_HANDLE_TYPE_DLL:
		sky_dll_close( object_handle );
		break;
	case SKY_HANDLE_TYPE_RWLOCK:
		sky_rwlock_destroy( object_handle );

	default:
		break;
	}
}

#include <stdlib.h>
#include <memory.h>
#include "sky.h"

bool sky_mem_init ( void )
{
	return true;
}

void sky_mem_uninit ( void )
{

}

SKY_API void * sky_mem_alloc ( uint32_t size )
{
	return calloc( 1, size );
}

SKY_API void sky_mem_free ( void * p )
{
	free( p );
}


SKY_API void sky_mem_set ( void * p, uint8_t value, uint32_t size )
{
	memset( p, value, size );
}

SKY_API void sky_mem_clear ( void * p, uint32_t size )
{
	memset( p, 0, size );
}

SKY_API void sky_mem_copy ( void * dst, void const * src, uint32_t size )
{
	memcpy( dst, src, size );
}

SKY_API void sky_mem_move ( void * dst, void const * src, uint32_t size )
{
	memmove( dst, src, size );
}

SKY_API int sky_mem_cmp (
	void const * buf1,
	void const * buf2,
	uint32_t size )
{
	return memcmp( buf1, buf2, size );
}

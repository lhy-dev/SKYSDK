#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "sky.h"

bool sky_str_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_str_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API char * sky_str_cpy (
	char * dst,
	char const * src,
	uint32_t count )
{
	return strncpy( dst, src, count );
}

SKY_API int sky_str_cmp (
	char const * str1,
	char const * str2 )
{
	return strcmp( str1, str2 );
}

SKY_API int sky_str_casecmp (
	char const * str1,
	char const * str2 )
{
#ifdef WIN32
	return _stricmp( str1, str2 );
#else
	return strcasecmp( str1, str2 );
#endif
}

SKY_API char * sky_str_cat (
	char * dst,
	char const * src,
	uint32_t count )
{
	return strncat( dst, src, count - 1 );
}

SKY_API char * sky_str_str (
	const char * haystack,
	const char * needle )
{
	return strstr( haystack, needle );
}

SKY_API uint32_t sky_str_len ( char const * str )
{
	return strlen( str );
}

SKY_API int sky_str_printf (
	char * str,
	uint32_t size,
	char const * format,
	... )
{
	va_list args;
	int ret;

	va_start( args, format );
	ret = vsnprintf( str, size, format, args );
	va_end(args);

	return ret;
}

SKY_API int sky_str_scanf (
	const char * str,
	const char * format,
	... )
{
#ifdef WIN32
	return -1;
#else
	va_list args;
	int ret;

	va_start( args, format );
	ret = vsscanf( str, format, args );
	va_end(args);

	return ret;
#endif
}

SKY_API int sky_str_to_hex (
	const char * str,
	uint32_t str_len,
	char * hex,
	uint32_t hex_len )
{
	uint32_t i;
	char buf[ 4 ];

	RVON( str, -1 );
	RVON( hex, -2 );
	RVOF( str_len * 2 <= hex_len, -2 );

	for ( i = 0; i < str_len; i++ )
	{
		sprintf( buf, "%02x", str[ i ] );
		sky_mem_copy( hex + i * 2, buf, 2 );
	}

	return 0;
}

SKY_API int sky_str_from_hex (
	const char * hex,
	uint32_t hex_len,
	char * str,
	uint32_t str_len )
{
	uint32_t value;
	int i;

	RVON( str, -1 );
	RVON( hex, -2 );
	RVOF( str_len * 2 <= hex_len, -2 );

	for ( i = 0; i < hex_len; i += 2 )
	{
		sscanf( &hex[ i ], "%02x", &value );
		str[ i / 2 ] = (char) ( value & 0xff );
	}

	return 0;
}

SKY_API char * sky_str_index ( char const * s, int c )
{
	return index( s, c );
}

SKY_API char * sky_str_rindex ( char const * s, int c )
{
	return rindex( s, c );
}

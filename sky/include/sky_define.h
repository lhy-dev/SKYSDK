#ifndef SKY_DEFINE_H
#define SKY_DEFINE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef SKY_EXPORTS
	#define SKY_API __declspec(dllexport)
#elif defined( SKY_IMPORTS )
	#define SKY_API __declspec(dllimport)
#else
	#define SKY_API
#endif

#if __STDC_VERSION__ < 199901L
	#if __GNUC__ >= 2
		#define __func__ __FUNCTION__
	#else
		#define __func__ "<unknown>"
	#endif
#endif

#ifndef VERBOSE_DEBUG_INFO
#define VERBOSE_DEBUG_INFO				0
#endif

typedef void*							SKY_HANDLE;

#define SKY_INVALID_HANDLE				((void*)0)

#define SKY_INFINITE					((uint32_t)-1)

#define SKY_MAXIMUM_WAIT_OBJECTS		64     // Maximum number of wait objects
#define SKY_WAIT_OBJECT_0				((uint32_t)0x00000000L)
#define SKY_WAIT_ABANDONED_0			((uint32_t)0x00000080L)
#define SKY_WAIT_TIMEOUT				((uint32_t)0x00000102L)
#define SKY_WAIT_FAILED				   ((uint32_t)0xFFFFFFFFL)

#ifndef bool
#define bool							unsigned char
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL							0
#else
#define NULL							((void *)0)
#endif
#endif

#ifndef max
#define max(a,b)						(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)						(((a) < (b)) ? (a) : (b))
#endif

#ifndef TEXT
#define TEXT(quote)						(quote)
#endif

#define LOWORD_OF(l)					((uint16_t)((l) & 0xffff))
#define HIWORD_OF(l)					((uint16_t)(((l) >> 16) & 0xffff))
#define LOBYTE_OF(w)					((uint8_t)((w) & 0xff))
#define HIBYTE_OF(w)					((uint8_t)(((w) >> 8) & 0xff))

struct sky_handle
{
	uint16_t cb;
	uint16_t type;
};

enum sky_handle_type
{
	SKY_HANDLE_TYPE_CRITICAL_SECTION	= 1,
	SKY_HANDLE_TYPE_LIST				= 2,
	SKY_HANDLE_TYPE_RWLOCK				= 3,
	SKY_HANDLE_TYPE_SHARE_MEMORY		= 4,
	SKY_HANDLE_TYPE_TIMER				= 5,
	SKY_HANDLE_TYPE_REGEX			= 6,
	SKY_HANDLE_TYPE_HASH				= 7,
	SKY_HANDLE_TYPE_DLL				= 8,
	SKY_HANDLE_TYPE_MAP 			=9
};
typedef enum sky_handle_type			SKY_HANDLE_TYPE;

#define SKY_ERR_CODE(class,err) \
	( 0x80000000 | ( ((uint32_t)class)<<16) | ((uint32_t)err) )

enum sky_error_class
{
	SKY_ERROR_CLASS_CRITICAL_SECTION	= 1,
	SKY_ERROR_CLASS_LIST				= 2,
	SKY_ERROR_CLASS_MUTEX				= 3,
	SKY_ERROR_CLASS_RWLOCK				= 4,
	SKY_ERROR_CLASS_SHARE_MEMORY		= 5,
	SKY_ERROR_CLASS_TIMER				= 6,
	SKY_ERROR_CLASS_REGEX				= 7,
	SKY_ERROR_CLASS_HASH				= 8,
	SKY_ERROR_CLASS_DLL				= 9,
};
typedef enum sky_error_class			SKY_ERROR_CLASS;

#define SKY_VALIDATE_HANDLE(type,handle) \
	SKY_ASSERT((handle!=SKY_INVALID_HANDLE)&&((uint16_t*)handle)[1]==type)

#ifndef __func__
#define __func__						__FUNCTION__
#endif

#define SKY_MEM_ALLOC_ARRAY(p,count) \
	p = sky_mem_alloc( sizeof( (p)[ 0 ] ) * (count) )

#define SKY_MEM_SAFE_FREE(mem) \
	do \
	{ \
		if ( (mem) != NULL ) \
		{ \
			sky_mem_free( mem ); \
			(mem) = NULL; \
		} \
	} while ( 0 )

#define SKY_SAFE_CLOSE_HANDLE(h) \
	do \
	{ \
		if( (h) != SKY_INVALID_HANDLE ) \
		{ \
			sky_close_handle( h ); \
			h = SKY_INVALID_HANDLE; \
		} \
	} while ( 0 )

#if 0
	// with ASSERT
	#define BON(cond)		if ( NULL == (cond) ) { SKY_ASSERT(0); break; }
	#define BOF(cond)		if ( !(cond) ) { SKY_ASSERT(0); break; }
	#define BOT(cond)		if ( cond ) { SKY_ASSERT(0); break; }
	#define BOZ(cond)		if ( 0 == (cond() ) { SKY_ASSERT(0); break; }

	#define RON(cond)		if ( NULL == (cond) ) { SKY_ASSERT(0); return; }
	#define ROF(cond)		if ( !(cond) ) { SKY_ASSERT(0); return; }
	#define ROT(cond)		if ( cond ) { SKY_ASSERT(0); return; }
	#define ROZ(cond)		if ( 0 == (cond) ) { SKY_ASSERT(0); return; }

	#define RVON(cond,v)	if ( NULL == (cond) ) { SKY_ASSERT(0); return v; }
	#define RVOF(cond,v)	if ( !(cond) ) { SKY_ASSERT(0); return v; }
	#define RVOT(cond,v)	if ( cond ) { SKY_ASSERT(0); return v; }
	#define RVOZ(cond,v)	if ( 0 == (cond) ) { SKY_ASSERT(0); return v; }
#else
	#define BON(cond)		if ( NULL == (cond) ) { break; }
	#define BOF(cond)		if ( !(cond) ) { break; }
	#define BOT(cond)		if ( cond ) { break; }
	#define BOZ(cond)		if ( 0 == (cond() ) { break; }

	#define RON(cond)		if ( NULL == (cond) ) { return; }
	#define ROF(cond)		if ( !(cond) ) { return; }
	#define ROT(cond)		if ( cond ) { return; }
	#define ROZ(cond)		if ( 0 == (cond) ) { return; }

	#define RVON(cond,v)	if ( NULL == (cond) ) { return v; }
	#define RVOF(cond,v)	if ( !(cond) ) { return v; }
	#define RVOT(cond,v)	if ( cond ) { return v; }
	#define RVOZ(cond,v)	if ( 0 == (cond) ) { return v; }
#endif

#define RFON(cond)		RVON( cond, false )
#define RFOF(cond)		RVOF( cond, false )
#define RFOT(cond)		RVOT( cond, false )
#define RFOZ(cond)		RVOZ( cond, false )

#define RTON(cond)		RVON( cond, true )
#define RTOF(cond)		RVOF( cond, true )
#define RTOT(cond)		RVOT( cond, true )
#define RTOZ(cond)		RVOZ( cond, true )

#define RNON(cond)		RVON( cond, NULL )
#define RNOF(cond)		RVOF( cond, NULL )
#define RNOT(cond)		RVOT( cond, NULL )
#define RNOZ(cond)		RVOZ( cond, NULL )

#endif

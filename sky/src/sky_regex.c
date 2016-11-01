#include <sys/types.h>
#include <regex.h>
#include "sky.h"

#define REGEX_MAX_MATCH					100

struct sky_handle_regex
{
	uint16_t cb;
	uint16_t type;
	regex_t regex;
	SKY_REGEX_SUBSTR * sub_str;
};
typedef struct sky_handle_regex  SKY_HANDLE_REGEX;

static void sky_regex_substr_free ( SKY_REGEX_SUBSTR * sub_str )
{
	SKY_REGEX_SUBSTR * next;

	while ( sub_str != NULL )
	{
		next = sub_str->next;
		SKY_MEM_SAFE_FREE( sub_str->str );
		SKY_MEM_SAFE_FREE( sub_str );
		sub_str = next;
	}
}

bool sky_regex_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_regex_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_regex_create ( char const * regex )
{
	SKY_HANDLE_REGEX * r = sky_mem_alloc( sizeof( SKY_HANDLE_REGEX ) );
	int rc;

	if ( NULL == r )
	{
		SKY_LOG_ERROR(
			"%s: fail to alloc memory for regex object %s.\n",
			__func__,
			regex );
		return SKY_INVALID_HANDLE;
	}

	r->cb = sizeof( SKY_HANDLE_REGEX );
	r->type = SKY_HANDLE_TYPE_REGEX;
	r->sub_str = NULL;
	rc = regcomp( &r->regex, regex, REG_EXTENDED | REG_NEWLINE );
	if ( rc != 0 )
	{
		char errmsg [ 1024 ];
		sky_mem_clear( errmsg, sizeof( errmsg ) );
		regerror ( rc, &r->regex, errmsg, sizeof( errmsg ) );
		SKY_LOG_ERROR(
			"%s: create regex object %s failed, error = %d, msg = %s\n",
			__func__,
			regex,
			rc,
			errmsg );
		sky_mem_free( r );
		return SKY_INVALID_HANDLE;
	}

	return r;
}

void sky_regex_destroy ( SKY_HANDLE regex )
{
	SKY_HANDLE_REGEX * r = (SKY_HANDLE_REGEX*) regex;
	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_REGEX, regex );

	if ( r != NULL )
	{
		if ( r->sub_str != NULL )
		{
			sky_regex_substr_free( r->sub_str );
		}
		regfree( &r->regex );
		sky_mem_free( r );
	}
}

SKY_API uint32_t sky_regex_match ( SKY_HANDLE regex, char const * str )
{
	SKY_HANDLE_REGEX * r = (SKY_HANDLE_REGEX*) regex;
    char const * p = str;
    regmatch_t m [ REGEX_MAX_MATCH ];
    uint32_t match_count = 0;
    int i = 0;

	SKY_VALIDATE_HANDLE( SKY_HANDLE_TYPE_REGEX, regex );

	while ( regexec( &r->regex, p, REGEX_MAX_MATCH, m, 0 ) == 0 )
	{
		for ( i = 0; i < REGEX_MAX_MATCH; i++ )
		{
			if ( -1 == m[ i ].rm_so )
			{
				break;
			}

			match_count++;
		}
		p += m[ 0 ].rm_eo;
	}

	return match_count;
}

SKY_API SKY_REGEX_SUBSTR * sky_regex_parse (
	SKY_HANDLE regex,
	char const * str )
{
	SKY_HANDLE_REGEX * r = (SKY_HANDLE_REGEX*) regex;
	char const * p = str;
	regmatch_t m[ REGEX_MAX_MATCH ];
	SKY_REGEX_SUBSTR * sub_str = NULL;

	if ( r->sub_str != NULL )
	{
		sky_regex_substr_free( r->sub_str );
		r->sub_str = NULL;
	}

	while ( 1 )
	{
		int i = 0;
		int nomatch = regexec( &r->regex, p, REGEX_MAX_MATCH, m, 0 );
		if ( nomatch )
		{
			break;
		}
		for ( i = 0; i < REGEX_MAX_MATCH; i++ )
		{
			int len;

			if ( m[ i ].rm_so == -1 )
			{
				break;
			}

			sub_str = sky_mem_alloc( sizeof( SKY_REGEX_SUBSTR ) );
			if ( NULL == sub_str )
			{
				break;
			}

			len = m[ i ].rm_eo - m[ i ].rm_so;
			sub_str->str = sky_mem_alloc( len + 1 );
			if ( NULL == sub_str->str )
			{
				break;
			}
			sub_str->next = NULL;

			sky_mem_copy( sub_str->str, p + m[ i ].rm_so, len );
			sub_str->str[ len ] = '\0';

			if ( NULL == r->sub_str )
			{
				r->sub_str = sub_str;
			}
			else
			{
				SKY_REGEX_SUBSTR * s;
				for ( s = r->sub_str; s != NULL; s = s->next )
				{
					if ( NULL == s->next )
					{
						s->next = sub_str;
						break;
					}
				}
			}
		}
		p += m[ 0 ].rm_eo;
	}

	return r->sub_str;
}

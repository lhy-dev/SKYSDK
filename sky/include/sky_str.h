#ifndef SKY_STR_H
#define SKY_STR_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API char * sky_str_cpy (
		char * dst,
		char const * src,
		uint32_t count );

	SKY_API int sky_str_cmp (
		char const * str1,
		char const * str2 );

	SKY_API int sky_str_casecmp (
		char const * str1,
		char const * str2 );

	SKY_API char * sky_str_cat (
		char * dst,
		char const * src,
		uint32_t count );

	/**
	 * The strstr() function finds the first occurrence of the substring needle
	 * in the string haystack. The terminating null bytes (\0)are not compared.
	 */
	SKY_API char * sky_str_str (
		const char * haystack,
		const char * needle );

	SKY_API uint32_t sky_str_len ( char const * str );

	SKY_API int sky_str_printf (
		char * str,
		uint32_t size,
		char const * format,
		... );

	SKY_API int sky_str_scanf (
		const char * str,
		const char * format,
		... );

	SKY_API int sky_str_to_hex (
		const char * str,
		uint32_t str_len,
		char * hex,
		uint32_t hex_len );

	SKY_API int sky_str_from_hex (
		const char * hex,
		uint32_t hex_len,
		char * str,
		uint32_t str_len );

	SKY_API char * sky_str_index ( char const * s, int c );

	SKY_API char * sky_str_rindex ( char const * s, int c );

	SKY_API int sky_str_to_xml_str (
		char const * str,
		char * xml_str,
		int size );

	SKY_API int sky_str_from_xml_str (
		char const * xml_str,
		char * str,
		int size );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

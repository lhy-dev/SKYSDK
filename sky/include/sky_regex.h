#ifndef SKY_REGEX_H
#define SKY_REGEX_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	struct sky_regex_substr
	{
		char * str;
		struct sky_regex_substr * next;
	};
	typedef struct sky_regex_substr	SKY_REGEX_SUBSTR;

	SKY_API SKY_HANDLE sky_regex_create ( char const * regex );

	SKY_API uint32_t sky_regex_match ( SKY_HANDLE regex, char const * str );

	SKY_API SKY_REGEX_SUBSTR * sky_regex_parse (
		SKY_HANDLE regex,
		char const * str );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

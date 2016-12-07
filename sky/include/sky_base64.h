#ifndef SKY_BASE64_H
#define SKY_BASE64_H

#include "sky_define.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	SKY_API char * sky_base64_encode (
		uint8_t const * data,
		uint32_t input_length,
		uint32_t * output_length );

	SKY_API uint8_t * sky_base64_decode (
		char const * data,
		uint32_t input_length,
		uint32_t * output_length );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#include <rpc/des_crypt.h>
#include "sky.h"

static char base64_encoding_table[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static uint8_t base64_decoding_table[ 256 ];

static const int base64_mod_table[] = {0, 2, 1};


bool sky_base64_init ( void )
{
	int i;

	SKY_LOG_TRACE( "%s\n", __func__ );

	sky_mem_clear( base64_decoding_table, sizeof( base64_decoding_table ) );
	for ( i = 0; i < 64; i++ )
	{
		base64_decoding_table[ (unsigned char) base64_encoding_table[ i ] ] = i;
	}

	return true;
}

void sky_base64_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API char * sky_base64_encode (
	uint8_t const * data,
	uint32_t input_length,
	uint32_t * output_length )
{
	char * encoded_data;
	int i;
	int j;

	RNON( output_length );

	*output_length = 4 * ( ( input_length + 2 ) / 3 );

	encoded_data = sky_mem_alloc( *output_length );
	RNON( encoded_data );

	for ( i = 0, j = 0; i < input_length; )
	{
		uint32_t octet_a = i < input_length ? (unsigned char) data[ i++ ] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char) data[ i++ ] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char) data[ i++ ] : 0;

		uint32_t triple = ( octet_a << 0x10 ) + ( octet_b << 0x08 ) + octet_c;

		encoded_data[ j++ ] = base64_encoding_table[ ( triple >> 3 * 6 ) & 0x3F ];
		encoded_data[ j++ ] = base64_encoding_table[ ( triple >> 2 * 6 ) & 0x3F ];
		encoded_data[ j++ ] = base64_encoding_table[ ( triple >> 1 * 6 ) & 0x3F ];
		encoded_data[ j++ ] = base64_encoding_table[ ( triple >> 0 * 6 ) & 0x3F ];
	}

	for ( i = 0; i < base64_mod_table[ input_length % 3 ]; i++ )
	{
		encoded_data[ *output_length - 1 - i ] = '=';
	}

	return encoded_data;
}

SKY_API uint8_t * sky_base64_decode (
	char const * data,
	uint32_t input_length,
	uint32_t * output_length )
{
	uint8_t * decoded_data;
	int i;
	int j;

	RNON( output_length );
	RNOF( 0 == input_length % 4 );

	*output_length = input_length / 4 * 3;

	if ( data[ input_length - 1 ] == '=' )
	{
		( *output_length )--;
	}

	if ( data[ input_length - 2 ] == '=' )
	{
		( *output_length )--;
	}

	decoded_data = sky_mem_alloc( *output_length );
	RNON( decoded_data );

	for ( i = 0, j = 0; i < input_length; )
	{
		uint32_t sextet_a = data[ i ] == '=' ? 0 & i++ : base64_decoding_table[ (uint8_t) data[ i++ ] ];
		uint32_t sextet_b = data[ i ] == '=' ? 0 & i++ : base64_decoding_table[ (uint8_t) data[ i++ ] ];
		uint32_t sextet_c = data[ i ] == '=' ? 0 & i++ : base64_decoding_table[ (uint8_t) data[ i++ ] ];
		uint32_t sextet_d = data[ i ] == '=' ? 0 & i++ : base64_decoding_table[ (uint8_t) data[ i++ ] ];

		uint32_t triple = ( sextet_a << 3 * 6 ) + ( sextet_b << 2 * 6 ) + ( sextet_c << 1 * 6 ) + ( sextet_d << 0 * 6 );

		if ( j < *output_length )
		{
			decoded_data[ j++ ] = ( triple >> 2 * 8 ) & 0xFF;
		}

		if ( j < *output_length )
		{
			decoded_data[ j++ ] = ( triple >> 1 * 8 ) & 0xFF;
		}

		if ( j < *output_length )
		{
			decoded_data[ j++ ] = ( triple >> 0 * 8 ) & 0xFF;
		}
	}

	return decoded_data;
}


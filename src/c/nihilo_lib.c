#include "nihilo.h"

static const GColor8 nihilo_cool_colors[6][4] = {
	{
		{ .b = 0, .g = 2, .r = 3, .a = 3 },
		{ .b = 0, .g = 1, .r = 3, .a = 3 },
		{ .b = 0, .g = 0, .r = 2, .a = 3 },
		{ .b = 0, .g = 0, .r = 1, .a = 3 },
	},
	{
		{ .b = 0, .g = 3, .r = 2, .a = 3 },
		{ .b = 0, .g = 3, .r = 1, .a = 3 },
		{ .b = 0, .g = 2, .r = 0, .a = 3 },
		{ .b = 0, .g = 1, .r = 0, .a = 3 },
	},
	{
		{ .b = 2, .g = 3, .r = 0, .a = 3 },
		{ .b = 1, .g = 3, .r = 0, .a = 3 },
		{ .b = 0, .g = 2, .r = 0, .a = 3 },
		{ .b = 0, .g = 1, .r = 0, .a = 3 },
	},
	{
		{ .b = 3, .g = 2, .r = 0, .a = 3 },
		{ .b = 3, .g = 1, .r = 0, .a = 3 },
		{ .b = 2, .g = 0, .r = 0, .a = 3 },
		{ .b = 1, .g = 0, .r = 0, .a = 3 },
	},
	{
		{ .b = 3, .g = 0, .r = 2, .a = 3 },
		{ .b = 3, .g = 0, .r = 1, .a = 3 },
		{ .b = 2, .g = 0, .r = 0, .a = 3 },
		{ .b = 1, .g = 0, .r = 0, .a = 3 },
	},
	{
		{ .b = 2, .g = 0, .r = 3, .a = 3 },
		{ .b = 1, .g = 0, .r = 3, .a = 3 },
		{ .b = 0, .g = 0, .r = 2, .a = 3 },
		{ .b = 0, .g = 0, .r = 1, .a = 3 },
	},
};

GColor8 const *nihilo_get_colorset( cool_colorsets_e col ) {
	return nihilo_cool_colors[col];
}

static uint32_t rotl32( uint32_t x, unsigned r ) {
	return ( x << r ) | ( x >> ( 32 - r ) );
}

uint32_t nihilo_hash32( uint8_t *p, size_t len ) {
	uint32_t h = 0x9747b28cU ^ (uint32_t) len;

	while ( len >= 4 ) {
		uint32_t k;

		memcpy( &k, p, 4 ); /* safe even if p is unaligned */

		k *= 0xcc9e2d51U;
		k = rotl32( k, 15 );
		k *= 0x1b873593U;

		h ^= k;
		h = rotl32( h, 13 );
		h = h * 5U + 0xe6546b64U;

		p += 4;
		len -= 4;
	}

	uint32_t k = 0;

	switch ( len ) {
		case 3: k ^= (uint32_t) p[2] << 16; /* fall through */
		case 2: k ^= (uint32_t) p[1] << 8; /* fall through */
		case 1:
			k ^= p[0];
			k *= 0xcc9e2d51U;
			k = rotl32( k, 15 );
			k *= 0x1b873593U;
			h ^= k;
	}

	h ^= h >> 16;
	h *= 0x85ebca6bU;
	h ^= h >> 13;
	h *= 0xc2b2ae35U;
	h ^= h >> 16;

	return h;
}

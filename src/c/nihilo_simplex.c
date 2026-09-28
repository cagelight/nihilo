#include "nihilo.h"

#define GRID_ORDER 2

#if GRID_ORDER == 1
#define GRID_X 200
#define GRID_Y 228
#elif GRID_ORDER == 2
#define GRID_X 100
#define GRID_Y 114
#elif GRID_ORDER == 3
#define GRID_X 50
#define GRID_Y 57
#endif

#define REFRESH_INTERVAL 2
#define REFRESH_RATE (REFRESH_INTERVAL * 1000)
#define CLOSE_SECONDS 300
#define CLOSE_COUNT (CLOSE_SECONDS / REFRESH_INTERVAL)

static struct nihilo_simplex_data_s {
	uint8_t grid[GRID_X][GRID_Y];
	uint8_t counter;
	uint16_t close_counter;
	uint8_t colormode;
	Layer *layer;
} *nihilo_simplex_data;

#define NIHILO (*nihilo_simplex_data)

// ================================================================
// INTERNALS
// ================================================================

static void nihilo_simplex_update_proc( Layer *layer, GContext *ctx ) {
	GBitmap *fb = graphics_capture_frame_buffer( ctx );
	if ( !fb )return;

	for ( uint8_t y = 0; y < GRID_Y; y++ ) {
#if GRID_ORDER == 1
		GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, y );
#elif GRID_ORDER == 2
		GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, y * 2 );
		GBitmapDataRowInfo r2 = gbitmap_get_data_row_info( fb, y * 2 + 1 );
#elif GRID_ORDER == 3
		GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, y * 4 );
		GBitmapDataRowInfo r2 = gbitmap_get_data_row_info( fb, y * 4 + 1 );
		GBitmapDataRowInfo r3 = gbitmap_get_data_row_info( fb, y * 4 + 2 );
		GBitmapDataRowInfo r4 = gbitmap_get_data_row_info( fb, y * 4 + 3 );
#endif
		for ( uint8_t x = 0; x < GRID_X; x++ ) {
			uint8_t val = NIHILO.grid[x][y] + NIHILO.counter;

			uint8_t d = val < 128 ? val : 256 - val;
			uint8_t col = ( d * 3 + 64 ) >> 7;
			GColor8 cval = nihilo_get_colorset( NIHILO.colormode )[col];

#if GRID_ORDER == 1
			r1.data[x] = cval.argb;
#elif GRID_ORDER == 2
			r1.data[x * 2] = r1.data[x * 2 + 1] = r2.data[x * 2] = r2.data[x * 2 + 1] = cval.argb;
#elif GRID_ORDER == 3
			r1.data[x * 4] = r1.data[x * 4 + 1] = r1.data[x * 4 + 2] = r1.data[x * 4 + 3] =
				r2.data[x * 4] = r2.data[x * 4 + 1] = r2.data[x * 4 + 2] = r2.data[x * 4 + 3] =
				r3.data[x * 4] = r3.data[x * 4 + 1] = r3.data[x * 4 + 2] = r3.data[x * 4 + 3] =
				r4.data[x * 4] = r4.data[x * 4 + 1] = r4.data[x * 4 + 2] = r4.data[x * 4 + 3] = cval.argb;
#endif
		}
	}

	graphics_release_frame_buffer( ctx, fb );
}

static const uint8_t P[256] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

static int32_t F( float x ) {
	int32_t i = (int32_t) x;
	return i - ( x < i );
}

static float N( uint8_t h, float x, float y ) {
	float t = .5f - x * x - y * y, u, v;
	if ( t <= 0 )return 0;
	h &= 7;
	u = h < 4 ? x : y;
	v = h < 4 ? y : x;
	t *= t;
	return t * t * ( ( h & 1 ? -u : u ) + ( h & 2 ? -2 * v : 2 * v ) );
}

float simplex2( float x, float y ) {
	float s = ( x + y ) * .3660254f, t, x0, y0, x1, y1;
	int32_t i = F( x + s ), j = F( y + s );
	uint8_t a, ii = (uint8_t) i, jj = (uint8_t) j;
	t = ( (float) i + (float) j ) * .21132487f;
	x0 = x - (float) i + t;
	y0 = y - (float) j + t;
	a = x0 > y0;
	x1 = x0 - a + .21132487f;
	y1 = y0 - !a + .21132487f;
	return 45 * (
		N( P[(uint8_t) ( ii + P[jj] )], x0, y0 ) +
		N( P[(uint8_t) ( ii + a + P[(uint8_t) ( jj + !a )] )], x1, y1 ) +
		N( P[(uint8_t) ( ii + 1 + P[(uint8_t) ( jj + 1 )] )], x0 - .57735027f, y0 - .57735027f )
	);
}

// ================================================================
// EXTERNALS
// ================================================================

static void nihilo_simplex_create( Window *win ) {
	nihilo_simplex_data = calloc( 1, sizeof( struct nihilo_simplex_data_s ) );

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	NIHILO.colormode = rand() % 6;

	NIHILO.layer = layer_create( bounds );
	layer_set_update_proc( NIHILO.layer, nihilo_simplex_update_proc );
	layer_add_child( window_get_root_layer( win ), NIHILO.layer );
}

static void nihilo_simplex_destroy() {
	layer_destroy( NIHILO.layer );
	free( nihilo_simplex_data );
}

static void nihilo_simplex_subinfo( nihilo_sim_subinfo_t *info ) {
	info->refresh_rate = REFRESH_RATE;
}

static nihilo_sim_reult_t nihilo_simplex_simulate( nihilo_sim_reason_t reason ) {
	if ( reason == NIHILO_SIM_REASON_INIT ) {
		int16_t xoff = rand() % 16000;
		int16_t yoff = rand() % 16000;
		float scale = ( (float) ( rand() % 100 ) ) / 100.0f;
		scale = scale * 0.03f + 0.01f;

		for ( int16_t y = 0; y < GRID_Y; y++ ) {
			for ( int16_t x = 0; x < GRID_X; x++ ) {
				float n = simplex2( ( x + xoff ) * scale, ( y + yoff ) * scale );
				n = fmaxf( -1.0f, fminf( 1.0f, n ) );
				NIHILO.grid[x][y] = (uint8_t) ( ( n + 1.0f ) * 127.5f + 0.5f );
			}
		}
	} else if ( reason == NIHILO_SIM_REASON_CLOCK ) {
		NIHILO.counter += 8;
		++NIHILO.close_counter;
	}

	layer_mark_dirty( NIHILO.layer );
	return NIHILO.close_counter >= CLOSE_COUNT ? NIHILO_SIM_RESULT_CLOSE : NIHILO_SIM_RESULT_CONTINUE;
}

nihilo_sim_t nihilo_simplex = {
	.create = nihilo_simplex_create,
	.destroy = nihilo_simplex_destroy,
	.subinfo = nihilo_simplex_subinfo,
	.simulate = nihilo_simplex_simulate
};

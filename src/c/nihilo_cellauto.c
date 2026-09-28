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

#define REFRESH_INTERVAL 1
#define REFRESH_RATE (REFRESH_INTERVAL * 1000)
#define CLOSE_SECONDS 600
#define CLOSE_COUNT (CLOSE_SECONDS / REFRESH_INTERVAL)

typedef struct ca_set_s {
	uint32_t born : 9;
	uint32_t surv : 9;
	uint32_t states : 6;
	uint32_t seed_thresh : 8;
} ca_set_t;

typedef uint8_t ( *grid_ref_t )[GRID_Y];

#define HASH_COUNT 8

static struct nihilo_cellauto_data_s {
	uint8_t grid_a[GRID_X][GRID_Y];
	uint8_t grid_b[GRID_X][GRID_Y];
	uint32_t hashes[HASH_COUNT];
	uint16_t frame_counter;
	uint8_t color;
	ca_set_t const *ca_set;
	bool ab;

	GFont font;
	Layer *layer;
} *nihilo_cellauto_data;

#define NIHILO (*nihilo_cellauto_data)

typedef enum {
	CA_NEIGH_0 = 1 << 0,
	CA_NEIGH_1 = 1 << 1,
	CA_NEIGH_2 = 1 << 2,
	CA_NEIGH_3 = 1 << 3,
	CA_NEIGH_4 = 1 << 4,
	CA_NEIGH_5 = 1 << 5,
	CA_NEIGH_6 = 1 << 6,
	CA_NEIGH_7 = 1 << 7,
	CA_NEIGH_8 = 1 << 8,
} ca_neighbor_flag_e;

#define SEED_BITS 8
#define SEED_MAX (1 << 8)

#define CA_SET_COUNT 18

static char const *cellauto_names[CA_SET_COUNT] = {
	"Game of Life",
	"Star Wars",
	"Soft Freeze",
	"Glissergy",
	"Living on the Edge",
	"Bombers",
	"Brain 6",
	"Brian's Brain",
	"Transers",
	"Wanderers",
	"Faders",
	"Incredible Machines",
	"SediMental",
	"Banners",
	"Fireworks",
	"Sticks",
	"Xtasy",
	"Prairie on Fire",
};

static const ca_set_t cellauto_sets[CA_SET_COUNT] = {
	/* Game of Life: B3/S23/C2 */
	{
		.born = CA_NEIGH_3,
		.surv = CA_NEIGH_2 | CA_NEIGH_3,
		.states = 2,
		.seed_thresh = SEED_MAX / 3
	},
	/* Star Wars: B2/S345/C4 */
	{
		.born = CA_NEIGH_2,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 4,
		.seed_thresh = SEED_MAX / 10
	},
	/* Soft Freeze: B38/S134/C6 */
	{
		.born = CA_NEIGH_3 | CA_NEIGH_8,
		.surv = CA_NEIGH_1 | CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5 | CA_NEIGH_8,
		.states = 6,
		.seed_thresh = SEED_MAX / 5
	},
	/* Glissergy: B245678/S035678/C5 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_4 | CA_NEIGH_5 | CA_NEIGH_6 | CA_NEIGH_7 | CA_NEIGH_8,
		.surv = CA_NEIGH_0 | CA_NEIGH_3 | CA_NEIGH_5 | CA_NEIGH_6 | CA_NEIGH_7 | CA_NEIGH_8,
		.states = 5,
		.seed_thresh = SEED_MAX / 20
	},
	/* Living on the Edge: B3/S345/C6 */
	{
		.born = CA_NEIGH_3,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 6,
		.seed_thresh = SEED_MAX / 5
	},
	/* Bombers: B24/S345/C25 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_4,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 25,
		.seed_thresh = SEED_MAX / 16
	},
	/* Brain 6: B246/S6/C3 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_4 | CA_NEIGH_6,
		.surv = CA_NEIGH_6,
		.states = 3,
		.seed_thresh = SEED_MAX / 16
	},
	/* Brian's Brain: B2/S/C3 */
	{
		.born = CA_NEIGH_2,
		.surv = 0,
		.states = 3,
		.seed_thresh = SEED_MAX / 8
	},
	/* Transers: B26/S345/C5 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_6,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 5,
		.seed_thresh = SEED_MAX / 13
	},
	/* Wanderers: B34678/S345/C5 */
	{
		.born = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_6 | CA_NEIGH_7 | CA_NEIGH_8,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 5,
		.seed_thresh = SEED_MAX / 6
	},
	/* Faders: B2/S2/C25 */
	{
		.born = CA_NEIGH_2,
		.surv = CA_NEIGH_2,
		.states = 25,
		.seed_thresh = SEED_MAX / 13
	},
	/* Incredible Machines: B2/S345/C6 */
	{
		.born = CA_NEIGH_2,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 6,
		.seed_thresh = SEED_MAX / 3
	},
	/* SediMental: B25678/S45678/C4 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_5 | CA_NEIGH_6 | CA_NEIGH_7 | CA_NEIGH_8,
		.surv = CA_NEIGH_4 | CA_NEIGH_5 | CA_NEIGH_6 | CA_NEIGH_7 | CA_NEIGH_8,
		.states = 4,
		.seed_thresh = SEED_MAX / 3
	},
	/* Banners: B3457/S2367/C5 */
	{
		.born = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5 | CA_NEIGH_7,
		.surv = CA_NEIGH_2 | CA_NEIGH_3 | CA_NEIGH_6 | CA_NEIGH_7,
		.states = 5,
		.seed_thresh = SEED_MAX / 6
	},
	/* Fireworks: B13/S2/C21 */
	{
		.born = CA_NEIGH_1 | CA_NEIGH_3,
		.surv = CA_NEIGH_2,
		.states = 21,
		.seed_thresh = SEED_MAX / 200 + 1
	},
	/* Sticks: B2/S3456/C6 */
	{
		.born = CA_NEIGH_2,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5 |
		CA_NEIGH_6,
		.states = 6,
		.seed_thresh = SEED_MAX / 13
	},
	/* Xtasy: B2356/S1456/C16 */
	{
		.born = CA_NEIGH_2 | CA_NEIGH_3 | CA_NEIGH_5 | CA_NEIGH_6,
		.surv = CA_NEIGH_1 | CA_NEIGH_4 | CA_NEIGH_5 | CA_NEIGH_6,
		.states = 16,
		.seed_thresh = SEED_MAX / 20
	},
	/* Prairie on Fire: B34/S345/C6 */
	{
		.born = CA_NEIGH_3 | CA_NEIGH_4,
		.surv = CA_NEIGH_3 | CA_NEIGH_4 | CA_NEIGH_5,
		.states = 6,
		.seed_thresh = SEED_MAX / 7
	},
};

// ================================================================
// INTERNALS
// ================================================================

static bool survives( uint8_t neigh ) {
	return neigh <= 8 && ( NIHILO.ca_set->surv & ( 1u << neigh ) );
}

static bool reproduces( uint8_t neigh ) {
	return neigh <= 8 && ( NIHILO.ca_set->born & ( 1u << neigh ) );
}

static grid_ref_t current_grid() {
	return NIHILO.ab ? NIHILO.grid_a : NIHILO.grid_b;
}

static grid_ref_t flip_grid() {
	NIHILO.ab = !NIHILO.ab;
	return current_grid();
}


static void nihilo_cellauto_update_proc( Layer *layer, GContext *ctx ) {
	GColor8 const *cset = nihilo_get_colorset( NIHILO.color );

	graphics_context_set_fill_color( ctx, GColorBlack );
	graphics_fill_rect( ctx, layer_get_bounds( layer ), 0, GCornerNone );

	graphics_context_set_text_color( ctx, GColorDarkGray );
	graphics_draw_text( ctx, cellauto_names[NIHILO.ca_set - cellauto_sets], NIHILO.font, layer_get_bounds( layer ), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );

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
			uint8_t val = current_grid()[x][y];
			if ( val <= 3 ) {
				GColor8 cval = cset[val];
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
	}

	graphics_release_frame_buffer( ctx, fb );
}

// ================================================================
// EXTERNALS
// ================================================================

static void nihilo_cellauto_create( Window *win ) {
	nihilo_cellauto_data = calloc( 1, sizeof( struct nihilo_cellauto_data_s ) );

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	NIHILO.font = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_ORBITRON_28 ) );
	NIHILO.layer = layer_create( bounds );
	NIHILO.color = rand() % NIHILO_COOL_COLORSET_MAX;
	layer_set_update_proc( NIHILO.layer, nihilo_cellauto_update_proc );
	layer_add_child( window_get_root_layer( win ), NIHILO.layer );
}

static void nihilo_cellauto_destroy() {
	layer_destroy( NIHILO.layer );
	free( nihilo_cellauto_data );
}

static void nihilo_cellauto_subinfo( nihilo_sim_subinfo_t *info ) {
	info->refresh_rate = REFRESH_RATE;
}

static nihilo_sim_reult_t nihilo_cellauto_simulate( nihilo_sim_reason_t reason ) {
	if ( reason == NIHILO_SIM_REASON_INIT ) {
		NIHILO.ca_set = &cellauto_sets[rand() % CA_SET_COUNT];
		for ( int16_t y = 0; y < GRID_Y; y++ ) {
			for ( int16_t x = 0; x < GRID_X; x++ ) {
				uint8_t val = rand() % SEED_MAX;
				if ( val < NIHILO.ca_set->seed_thresh ) {
					current_grid()[x][y] = 0;
				} else {
					current_grid()[x][y] = -1;
				}
			}
		}
	} else if ( reason == NIHILO_SIM_REASON_CLOCK ) {
		grid_ref_t grid_past = current_grid();
		grid_ref_t grid_present = flip_grid();
		for ( int16_t y = 0; y < GRID_Y; y++ ) {
			for ( int16_t x = 0; x < GRID_X; x++ ) {
				uint8_t state = grid_past[x][y];
				uint8_t xm = ( x == 0 ) ? GRID_X - 1 : x - 1;
				uint8_t xp = ( x == GRID_X - 1 ) ? 0 : x + 1;
				uint8_t ym = ( y == 0 ) ? GRID_Y - 1 : y - 1;
				uint8_t yp = ( y == GRID_Y - 1 ) ? 0 : y + 1;

				uint8_t neigh = 0;
				if ( grid_past[xm][ym] <= 1 )
					++neigh;
				if ( grid_past[x][ym] <= 1 )
					++neigh;
				if ( grid_past[xp][ym] <= 1 )
					++neigh;
				if ( grid_past[xm][y] <= 1 )
					++neigh;
				if ( grid_past[xp][y] <= 1 )
					++neigh;
				if ( grid_past[xm][yp] <= 1 )
					++neigh;
				if ( grid_past[x][yp] <= 1 )
					++neigh;
				if ( grid_past[xp][yp] <= 1 )
					++neigh;

				if ( state <= 1 ) {
					grid_present[x][y] = survives( neigh ) ? 1 : 2;
				} else if ( state >= NIHILO.ca_set->states && reproduces( neigh ) ) {
					grid_present[x][y] = 0;
				} else {
					grid_present[x][y] = UINT8_MAX;
				}
			}
		}

		NIHILO.hashes[NIHILO.frame_counter % HASH_COUNT] = nihilo_hash32( (uint8_t *) grid_present, GRID_X * GRID_Y );
		++NIHILO.frame_counter;

		for ( unsigned i = 1; i < HASH_COUNT; ++i ) {
			for ( unsigned j = 0; j < i; ++j ) {
				if ( NIHILO.hashes[i] == NIHILO.hashes[j] && NIHILO.hashes[i] )
					return NIHILO_SIM_RESULT_CLOSE;
			}
		}
	}

	layer_mark_dirty( NIHILO.layer );
	return NIHILO.frame_counter >= CLOSE_COUNT ? NIHILO_SIM_RESULT_CLOSE : NIHILO_SIM_RESULT_CONTINUE;
}

nihilo_sim_t nihilo_cellauto = {
	.create = nihilo_cellauto_create,
	.destroy = nihilo_cellauto_destroy,
	.subinfo = nihilo_cellauto_subinfo,
	.simulate = nihilo_cellauto_simulate
};

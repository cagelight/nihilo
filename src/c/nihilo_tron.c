#include "nihilo.h"

static const GColor8 hsv_s100[6][3] = {
	{ { .argb = 0xF0 }, { .argb = 0xE0 }, { .argb = 0xD0 } }, //   0°  Red
	{ { .argb = 0xFC }, { .argb = 0xE8 }, { .argb = 0xD4 } }, //  60°  Yellow
	{ { .argb = 0xCC }, { .argb = 0xC8 }, { .argb = 0xC4 } }, // 120°  Green
	{ { .argb = 0xCF }, { .argb = 0xCA }, { .argb = 0xC5 } }, // 180°  Cyan
	{ { .argb = 0xC3 }, { .argb = 0xC2 }, { .argb = 0xC1 } }, // 240°  Blue
	{ { .argb = 0xF3 }, { .argb = 0xE2 }, { .argb = 0xD1 } }, // 300°  Magenta
};

static const char *dead_text[6] = {
	"RED IS DEAD!",
	"YELLOW IS JELL-O",
	"GREEN? HAS-BEEN.",
	"CYAN IS LESS THAN",
	"BLUE TURNS TO STEW",
	"MAGENTA IRREDENTA"
};

static const char *win_text[6] = {
	"WINNER: RED",
	"WINNER: YELLOW",
	"WINNER: GREEN",
	"WINNER: CYAN",
	"WINNER: BLUE",
	"WINNER: MAGENTA"
};

#define GRID_ORDER 3

#if GRID_ORDER == 2
#define GRID_X 100
#define GRID_Y 114
#elif GRID_ORDER == 3
#define GRID_X 50
#define GRID_Y 57
#endif

#define REFRESH_INTERVAL 1
#define REFRESH_RATE (REFRESH_INTERVAL * 1000)
#define TEXT_LIFE (120 / REFRESH_INTERVAL)

#define BIKES 6
#define DANGER_INTERVAL 10
#define DANGER_START 10
#define BIKE_TAIL 6

#define BIKE_AGGR_BITS 4
#define BIKE_AGGR_MAX ((uint32_t)(1 << BIKE_AGGR_BITS))
#define BIKE_FEAR_BITS 4
#define BIKE_FEAR_MAX ((uint32_t)(1 << BIKE_FEAR_BITS))
#define BIKE_RAND_BITS 5
#define BIKE_RAND_MAX ((uint32_t)(1 << BIKE_RAND_BITS))

typedef struct tron_bike_s {
	uint32_t x : 7, y : 7;
	// 14
	uint32_t team : 3; // 0 = dead
	// 17
	uint32_t dir : 2;
	// 19
	uint32_t aggr : BIKE_AGGR_BITS; // aggressiveness -- chance to change directions spontaneously towards the nearest bike, as well as chance to boost 2 turns in 1 step
	// 23
	uint32_t rand : BIKE_RAND_BITS; // randomness profile -- chance to change directions randomly + pattern
	// 28
	uint32_t fear : BIKE_FEAR_BITS; // fear -- chance to change directions spontaneously away from the nearest bike
	// 32
} tron_bike_t;

typedef struct tron_space_s {
	uint16_t team : 3;
	uint16_t life : 13;
} tron_space_t;

static struct nihilo_tron_data_s {
	tron_space_t grid[GRID_X][GRID_Y];
	tron_bike_t bikes[BIKES];
	uint16_t danger;
	uint8_t danger_int;
	char const *draw_text;
	uint16_t text_cd;

	GFont font;
	Layer *layer;
} *nihilo_tron_data;

#define NIHILO (*nihilo_tron_data)

// ================================================================
// INTERNALS
// ================================================================

static tron_space_t *nihilo_tron_get_space( int8_t x, int8_t y ) {
	if ( x < 0 || x >= GRID_X )
		return NULL;

	if ( y < 0 || y >= GRID_Y )
		return NULL;

	return &NIHILO.grid[x][y];
}


static void nihilo_tron_assert_space( tron_bike_t *bike ) {
	tron_space_t *space = &NIHILO.grid[bike->x][bike->y];
	space->team = bike->team;
	space->life = NIHILO.danger;
}

static void nihilo_tron_update_proc( Layer *layer, GContext *ctx ) {
	graphics_context_set_fill_color( ctx, GColorBlack );
	graphics_fill_rect( ctx, layer_get_bounds( layer ), 0, GCornerNone );

	if ( NIHILO.draw_text ) {
		graphics_context_set_text_color( ctx, GColorDarkGray );
		graphics_draw_text( ctx, NIHILO.draw_text, NIHILO.font, layer_get_bounds( layer ), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );
	}

	GBitmap *fb = graphics_capture_frame_buffer( ctx );
	if ( !fb )return;

	for ( int8_t y = 0; y < GRID_Y; y++ ) {
#if GRID_ORDER == 2
		GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, y * 2 );
		GBitmapDataRowInfo r2 = gbitmap_get_data_row_info( fb, y * 2 + 1 );
#elif GRID_ORDER == 3
		GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, y * 4 );
		GBitmapDataRowInfo r2 = gbitmap_get_data_row_info( fb, y * 4 + 1 );
		GBitmapDataRowInfo r3 = gbitmap_get_data_row_info( fb, y * 4 + 2 );
		GBitmapDataRowInfo r4 = gbitmap_get_data_row_info( fb, y * 4 + 3 );
#endif
		for ( int8_t x = 0; x < GRID_X; x++ ) {
			tron_space_t *g = &NIHILO.grid[x][y];

			uint8_t cval = 0;
			if ( g->team ) {
				GColor8 const *tcol = hsv_s100[g->team - 1];
				if ( g->life >= NIHILO.danger - 1 )
					cval = tcol[0].argb;
				else if ( g->life >= NIHILO.danger - 1 - BIKE_TAIL )
					cval = tcol[1].argb;
				else if ( g->life )
					cval = tcol[2].argb;
			}

			if ( cval ) {
#if GRID_ORDER == 2
				r1.data[x * 2] = r1.data[x * 2 + 1] = r2.data[x * 2] = r2.data[x * 2 + 1] = cval;
#elif GRID_ORDER == 3
				r1.data[x * 4] = r1.data[x * 4 + 1] = r1.data[x * 4 + 2] = r1.data[x * 4 + 3] =
					r2.data[x * 4] = r2.data[x * 4 + 1] = r2.data[x * 4 + 2] = r2.data[x * 4 + 3] =
					r3.data[x * 4] = r3.data[x * 4 + 1] = r3.data[x * 4 + 2] = r3.data[x * 4 + 3] =
					r4.data[x * 4] = r4.data[x * 4 + 1] = r4.data[x * 4 + 2] = r4.data[x * 4 + 3] = cval;
#endif
			}
		}
	}

	graphics_release_frame_buffer( ctx, fb );
}

// ================================================================
// EXTERNALS
// ================================================================

static void nihilo_tron_create( Window *win ) {
	nihilo_tron_data = calloc( 1, sizeof( struct nihilo_tron_data_s ) );

	// RED -- calm and fearless
	NIHILO.bikes[0].team = 1;
	NIHILO.bikes[0].dir = rand() % 4;
	NIHILO.bikes[0].aggr = BIKE_AGGR_MAX - 1;
	NIHILO.bikes[0].fear = 0;
	NIHILO.bikes[0].rand = 0;

	// YELLOW -- crazy
	NIHILO.bikes[1].team = 2;
	NIHILO.bikes[1].dir = rand() % 4;
	NIHILO.bikes[1].aggr = BIKE_AGGR_MAX - 1;
	NIHILO.bikes[1].fear = BIKE_FEAR_MAX - 1;
	NIHILO.bikes[1].rand = BIKE_RAND_MAX - 1;

	// GREEN
	NIHILO.bikes[2].team = 3;
	NIHILO.bikes[2].dir = rand() % 4;
	NIHILO.bikes[2].aggr = rand() % BIKE_AGGR_MAX;
	NIHILO.bikes[2].fear = rand() % BIKE_FEAR_MAX;
	NIHILO.bikes[2].rand = rand() % BIKE_RAND_MAX;

	// CYAN
	NIHILO.bikes[3].team = 4;
	NIHILO.bikes[3].dir = rand() % 4;
	NIHILO.bikes[3].aggr = rand() % BIKE_AGGR_MAX;
	NIHILO.bikes[3].fear = rand() % BIKE_FEAR_MAX;
	NIHILO.bikes[3].rand = rand() % BIKE_RAND_MAX;

	// BLUE -- calm and risk averse
	NIHILO.bikes[4].team = 5;
	NIHILO.bikes[4].dir = rand() % 4;
	NIHILO.bikes[4].aggr = 0;
	NIHILO.bikes[4].fear = BIKE_FEAR_MAX - 1;
	NIHILO.bikes[4].rand = 0;

	// MAGENTA
	NIHILO.bikes[5].team = 6;
	NIHILO.bikes[5].dir = rand() % 4;
	NIHILO.bikes[5].aggr = rand() % BIKE_AGGR_MAX;
	NIHILO.bikes[5].fear = rand() % BIKE_FEAR_MAX;
	NIHILO.bikes[5].rand = rand() % BIKE_RAND_MAX;

#if GRID_ORDER == 2
	NIHILO.bikes[0].x = 50;
	NIHILO.bikes[0].y = 17;
	NIHILO.bikes[1].x = 85;
	NIHILO.bikes[1].y = 37;
	NIHILO.bikes[2].x = 85;
	NIHILO.bikes[2].y = 77;
	NIHILO.bikes[3].x = 50;
	NIHILO.bikes[3].y = 97;
	NIHILO.bikes[4].x = 15;
	NIHILO.bikes[4].y = 77;
	NIHILO.bikes[5].x = 15;
	NIHILO.bikes[5].y = 37;
#elif GRID_ORDER == 3
	NIHILO.bikes[0].x = 25;
	NIHILO.bikes[0].y = 8;
	NIHILO.bikes[1].x = 43;
	NIHILO.bikes[1].y = 18;
	NIHILO.bikes[2].x = 43;
	NIHILO.bikes[2].y = 38;
	NIHILO.bikes[3].x = 25;
	NIHILO.bikes[3].y = 48;
	NIHILO.bikes[4].x = 7;
	NIHILO.bikes[4].y = 38;
	NIHILO.bikes[5].x = 7;
	NIHILO.bikes[5].y = 18;
#endif

	NIHILO.danger = DANGER_START;

	for ( uint8_t b = 0; b < BIKES; b++ ) {
		nihilo_tron_assert_space( &NIHILO.bikes[b] );
	}

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	NIHILO.font = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_NOTCAKE_MONO_22 ) );
	NIHILO.layer = layer_create( bounds );
	layer_set_update_proc( NIHILO.layer, nihilo_tron_update_proc );
	layer_add_child( window_get_root_layer( win ), NIHILO.layer );
}

static void nihilo_tron_destroy() {
	fonts_unload_custom_font( NIHILO.font );
	layer_destroy( NIHILO.layer );
	free( nihilo_tron_data );
}

static void nihilo_tron_subinfo( nihilo_sim_subinfo_t *info ) {
	info->refresh_rate = REFRESH_RATE;
}

static nihilo_sim_reult_t nihilo_tron_simulate( nihilo_sim_reason_t reason ) {
	if ( reason != NIHILO_SIM_REASON_INIT && reason != NIHILO_SIM_REASON_CLOCK )
		return NIHILO_SIM_RESULT_CONTINUE;

	++NIHILO.danger_int;
	if ( NIHILO.danger_int >= DANGER_INTERVAL ) {
		NIHILO.danger_int = 0;
		++NIHILO.danger;
	}

	if ( NIHILO.danger_int != 0 )
		for ( uint8_t x = 0; x < GRID_X; x++ ) {
			for ( uint8_t y = 0; y < GRID_Y; y++ ) {
				tron_space_t *g = &NIHILO.grid[x][y];
				if ( g->life ) {
					g->life--;
					if ( !g->life )
						g->team = 0;
				}
			}
		}

	uint8_t winner = 0;
	uint8_t live_bikes = 0;
	for ( uint8_t b = 0; b < BIKES; b++ ) {
		tron_bike_t *bike = &NIHILO.bikes[b];

		if ( bike->team == 0 ) // already dead
			continue;

		winner = b;
		++live_bikes;

		int8_t fx = 0, fy = 0;
		switch ( bike->dir ) {
			case 0: // north
				fy = 1;
				break;
			case 1: // east
				fx = 1;
				break;
			case 2: // south
				fy = -1;
				break;
			case 3: // west
				fx = -1;
				break;
		}

		tron_space_t *forward = nihilo_tron_get_space( (int8_t) ( bike->x + fx ), (int8_t) ( bike->y + fy ) );
		bool forwardok = forward && forward->life == 0;

		bool rnd_turn = false;
		if ( bike->rand > 0 ) {
			uint32_t modval = BIKE_RAND_MAX * BIKE_RAND_MAX * 50;
			if ( rand() % modval < bike->rand * bike->rand )
				rnd_turn = true;
		}

		if ( forwardok && !rnd_turn ) {
			bike->x += fx;
			bike->y += fy;
			nihilo_tron_assert_space( bike );
			continue;
		}

		// hit a wall, swerve or die
		// or random turn

		int8_t nx = 0;
		int8_t ny = 0;
		switch ( bike->dir ) {
			case 0: // north
			case 2: // south
				nx = 1;
				break;
			case 1: // east
			case 3: // west
				ny = 1;
				break;
		}

		tron_space_t *op1 = nihilo_tron_get_space( (int8_t) ( bike->x + nx ), (int8_t) ( bike->y + ny ) );
		tron_space_t *op2 = nihilo_tron_get_space( (int8_t) ( bike->x - nx ), (int8_t) ( bike->y - ny ) );
		bool op1ok = op1 && op1->life == 0;
		bool op2ok = op2 && op2->life == 0;

		if ( !op1ok && !op2ok ) {
			// DIE -- unless this was a random turn
			if ( forwardok ) {
				bike->x += fx;
				bike->y += fy;
				nihilo_tron_assert_space( bike );
			} else {
				bike->team = 0;
				NIHILO.draw_text = dead_text[b];
				NIHILO.text_cd = 0;
			}
			continue;
		}

		tron_space_t *choice = NULL;
		if ( op1ok && op2ok ) // pick one at random
			choice = rand() % 2 ? op1 : op2;
		else // or pick the only one available
			choice = op1ok ? op1 : op2;

		bike->x += ( choice == op1 ) ? nx : -nx;
		bike->y += ( choice == op1 ) ? ny : -ny;

		switch ( bike->dir ) {
			case 0:
			case 2:
				bike->dir = ( choice == op1 ) ? 1 : 3;
				break;
			case 1:
			case 3:
				bike->dir = ( choice == op1 ) ? 0 : 2;
				break;
		}

		nihilo_tron_assert_space( bike );
	}

	if ( live_bikes == 1 ) {
		if ( NIHILO.draw_text != win_text[winner] ) {
			NIHILO.draw_text = win_text[winner];
			NIHILO.text_cd = 0;
		}
	}

	if ( NIHILO.draw_text ) {
		if ( NIHILO.text_cd >= TEXT_LIFE )
			NIHILO.draw_text = NULL;
		else
			NIHILO.text_cd += REFRESH_INTERVAL;
	}

	if ( !NIHILO.draw_text && live_bikes <= 1 ) {
		return NIHILO_SIM_RESULT_CLOSE;
	} else {
		layer_mark_dirty( NIHILO.layer );
		return NIHILO_SIM_RESULT_CONTINUE;
	}
}

nihilo_sim_t nihilo_tron = {
	.create = nihilo_tron_create,
	.destroy = nihilo_tron_destroy,
	.subinfo = nihilo_tron_subinfo,
	.simulate = nihilo_tron_simulate
};

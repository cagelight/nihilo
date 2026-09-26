#include "nihilo.h"

static const GColor8 hsv_s100_v100[18] = {
	{ .argb = 0xF0 }, //   0°  RGB = (3,0,0)
	{ .argb = 0xF4 }, //  20°  RGB = (3,1,0)
	{ .argb = 0xF8 }, //  40°  RGB = (3,2,0)
	{ .argb = 0xFC }, //  60°  RGB = (3,3,0)
	{ .argb = 0xEC }, //  80°  RGB = (2,3,0)
	{ .argb = 0xDC }, // 100°  RGB = (1,3,0)
	{ .argb = 0xCC }, // 120°  RGB = (0,3,0)
	{ .argb = 0xCD }, // 140°  RGB = (0,3,1)
	{ .argb = 0xCE }, // 160°  RGB = (0,3,2)
	{ .argb = 0xCF }, // 180°  RGB = (0,3,3)
	{ .argb = 0xCB }, // 200°  RGB = (0,2,3)
	{ .argb = 0xC7 }, // 220°  RGB = (0,1,3)
	{ .argb = 0xC3 }, // 240°  RGB = (0,0,3)
	{ .argb = 0xD3 }, // 260°  RGB = (1,0,3)
	{ .argb = 0xE3 }, // 280°  RGB = (2,0,3)
	{ .argb = 0xF3 }, // 300°  RGB = (3,0,3)
	{ .argb = 0xF2 }, // 320°  RGB = (3,0,2)
	{ .argb = 0xF1 }, // 340°  RGB = (3,0,1)
};

#define INTEGERS_TYPE uint16_t
#define INTEGERS_PRINT PRIu16
#define INTEGERS 3
#define INTEGERS_STR "3"
#define INTEGERS_MOD 999
#define ROWS 1
#define FONT RESOURCE_ID_FONT_NOTCAKE_MONO_44
#define FONT_SIZE 44
#define REFRESH_RATE 5000

static struct nihilo_numbers_data_s {
	INTEGERS_TYPE counter;
	char counter_buffer[ROWS][INTEGERS + 1];
	TextLayer *number_layers[ROWS];
	GFont number_font;
} *nihilo_numbers_data;
#define NIHILO (*nihilo_numbers_data)

static void nihilo_numbers_create( Window *win ) {
	nihilo_numbers_data = malloc( sizeof( struct nihilo_numbers_data_s ) );

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	NIHILO.counter = 0;
	NIHILO.number_font = fonts_load_custom_font( resource_get_handle( FONT ) );


	for ( int i = 0; i < ROWS; i++ ) {
		int step = 0;
		if ( i > 0 )
			step = ( i + 1 ) / 2 * FONT_SIZE;
		if ( i % 2 )
			step = -step;

		NIHILO.number_layers[i] = text_layer_create( GRect( 0, (bounds.size.h - FONT_SIZE) / 2 + step, bounds.size.w, FONT_SIZE ) );
		text_layer_set_background_color( NIHILO.number_layers[i], GColorClear );
		text_layer_set_text_color( NIHILO.number_layers[i], GColorBlack );
		text_layer_set_font( NIHILO.number_layers[i], NIHILO.number_font );
		text_layer_set_text_alignment( NIHILO.number_layers[i], GTextAlignmentCenter );
		layer_add_child( root, text_layer_get_layer( NIHILO.number_layers[i] ) );
	}
}

static void nihilo_numbers_destroy() {
	for ( int i = 0; i < ROWS; i++ ) {
		text_layer_destroy( NIHILO.number_layers[i] );
	}
	fonts_unload_custom_font( NIHILO.number_font );
	free( nihilo_numbers_data );
}

static void nihilo_numbers_subinfo( nihilo_sim_subinfo_t *info ) {
	info->refresh_rate = REFRESH_RATE;
}

static void nihilo_numbers_simulate( nihilo_sim_reason_t reason ) {
	if ( reason != NIHILO_SIM_REASON_CLOCK )
		return;
	uint8_t color = rand() % 18;
	for ( int i = 0; i < ROWS; i++ ) {
		NIHILO.counter = rand() % INTEGERS_MOD;
		snprintf( NIHILO.counter_buffer[i], sizeof NIHILO.counter_buffer[i], "%0" INTEGERS_STR INTEGERS_PRINT, NIHILO.counter );
		text_layer_set_text_color( NIHILO.number_layers[i], hsv_s100_v100[color] );
		text_layer_set_text( NIHILO.number_layers[i], NIHILO.counter_buffer[i] );
	}
}

nihilo_sim_t nihilo_numbers = {
	.create = nihilo_numbers_create,
	.destroy = nihilo_numbers_destroy,
	.subinfo = nihilo_numbers_subinfo,
	.simulate = nihilo_numbers_simulate
};

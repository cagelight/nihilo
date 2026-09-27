#include "nihilo.h"

static struct nihilo_image_data_s {
	Layer *canvas;
	GBitmap *bitmap;
} *nihilo_image_data;

#define NIHILO (*nihilo_image_data)

static void nihilo_image_update_proc( Layer *layer, GContext *ctx ) {
	graphics_context_set_compositing_mode( ctx, GCompOpSet );
	graphics_draw_bitmap_in_rect(
		ctx,
		NIHILO.bitmap,
		GRect( 0, 0, 200, 228 )
	);
}

// ================================================================
// EXTERNALS
// ================================================================

static void nihilo_image_create( Window *win ) {
	nihilo_image_data = malloc( sizeof( struct nihilo_image_data_s ) );

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	NIHILO.bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMG_ENNF );
	NIHILO.canvas = layer_create( bounds );
	layer_set_update_proc( NIHILO.canvas, nihilo_image_update_proc );
	layer_add_child( root, NIHILO.canvas );
}

static void nihilo_image_destroy() {
	layer_destroy( NIHILO.canvas );
	gbitmap_destroy( NIHILO.bitmap );
	free( nihilo_image_data );
}

static void nihilo_image_subinfo( nihilo_sim_subinfo_t *info ) {
	info->refresh_rate = 5000;
}

static void nihilo_image_simulate( nihilo_sim_reason_t reason ) {
	if ( reason != NIHILO_SIM_REASON_CLOCK )
		return;
}

nihilo_sim_t nihilo_image = {
	.create = nihilo_image_create,
	.destroy = nihilo_image_destroy,
	.subinfo = nihilo_image_subinfo,
	.simulate = nihilo_image_simulate
};

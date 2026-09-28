#include "nihilo.h"

#define LIFE 10000

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

typedef struct image_set_s {
	uint16_t handle;
	uint16_t weight;
} image_set_t;

static const image_set_t images[] = {
	{ .handle = RESOURCE_ID_IMG_ATEOT, .weight = 200 },
	{ .handle = RESOURCE_ID_IMG_BOR, .weight = 50 },
	{ .handle = RESOURCE_ID_IMG_DEMON, .weight = 10 },
	{ .handle = RESOURCE_ID_IMG_ENNF, .weight = 200 },
	{ .handle = RESOURCE_ID_IMG_EP, .weight = 200 },
	{ .handle = RESOURCE_ID_IMG_FALKIE, .weight = 200 },
	{ .handle = RESOURCE_ID_IMG_LOONA, .weight = 1 },
	{ .handle = RESOURCE_ID_IMG_RB, .weight = 50 },
	{ .handle = RESOURCE_ID_IMG_SLR, .weight = 50 },
};
#define NIHILO_IMAGE_COUNT (sizeof(images) / sizeof(image_set_t))

// ================================================================
// EXTERNALS
// ================================================================

static void nihilo_image_create( Window *win ) {
	nihilo_image_data = malloc( sizeof( struct nihilo_image_data_s ) );

	Layer *root = window_get_root_layer( win );
	GRect bounds = layer_get_bounds( root );

	int32_t total_weight = 0;

	for ( uint32_t i = 0; i < NIHILO_IMAGE_COUNT; i++ ) {
		total_weight += images[i].weight;
	}

	int32_t r = rand() % total_weight;

	for ( uint32_t i = 0; i < NIHILO_IMAGE_COUNT; i++ ) {
		if ( r < images[i].weight ) {
			NIHILO.bitmap = gbitmap_create_with_resource( images[i].handle );
			break;
		}

		r -= images[i].weight;
	}


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
	info->refresh_rate = LIFE;
}

static nihilo_sim_reult_t nihilo_image_simulate( nihilo_sim_reason_t reason ) {
	return reason == NIHILO_SIM_REASON_CLOCK ? NIHILO_SIM_RESULT_CLOSE : NIHILO_SIM_RESULT_CONTINUE;
}

nihilo_sim_t nihilo_image = {
	.create = nihilo_image_create,
	.destroy = nihilo_image_destroy,
	.subinfo = nihilo_image_subinfo,
	.simulate = nihilo_image_simulate
};

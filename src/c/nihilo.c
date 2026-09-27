#include "nihilo.h"

static nihilo_sim_t *s_current_sim;
static nihilo_sim_subinfo_t s_subinfo;
static Window *s_main_window;
static AppTimer *s_apptimer = NULL;

#define DEV_OVERRIDE nihilo_image

static void simstep() {
	s_current_sim->simulate( NIHILO_SIM_REASON_CLOCK );
	if ( s_subinfo.refresh_rate > 0 )
		s_apptimer = app_timer_register( s_subinfo.refresh_rate, simstep, NULL );
}

static void main_window_load( Window *window ) {
	s_current_sim->create( window );

	memset( &s_subinfo, 0, sizeof( s_subinfo ) );
	s_current_sim->subinfo( &s_subinfo );

	simstep();
}

static void main_window_unload( Window *window ) {
	s_current_sim->destroy();
}

#ifndef DEV_OVERRIDE
typedef struct sim_set_s {
	nihilo_sim_t *sim;
	uint16_t weight;
} sim_set_t;

static const sim_set_t nihilo_sims[] = {
	{ .sim = &nihilo_numbers, .weight = 200 },
	{ .sim = &nihilo_tron, .weight = 200 },
};

#define NIHILO_SIM_COUNT (sizeof(nihilo_sims) / sizeof(sim_set_t))
#endif

static void init( void ) {
#ifdef DEV_OVERRIDE
	s_current_sim = &DEV_OVERRIDE;
#else
	int32_t total_weight = 0;

	for ( uint32_t i = 0; i < NIHILO_SIM_COUNT; i++ ) {
		total_weight += nihilo_sims[i].weight;
	}

	int32_t r = rand() % total_weight;

	for ( uint32_t i = 0; i < NIHILO_SIM_COUNT; i++ ) {
		if ( r < nihilo_sims[i].weight ) {
			s_current_sim = nihilo_sims[i].sim;
			break;
		}

		r -= nihilo_sims[i].weight;
	}
#endif

	s_main_window = window_create();
	window_set_background_color( s_main_window, GColorBlack );

	window_set_window_handlers( s_main_window,
	                            (WindowHandlers) {
		                            .load = main_window_load,
		                            .unload = main_window_unload
	                            } );

	window_stack_push( s_main_window, true );
}

static void deinit( void ) {
	if ( s_apptimer ) {
		app_timer_cancel( s_apptimer );
	}
	window_destroy( s_main_window );
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}

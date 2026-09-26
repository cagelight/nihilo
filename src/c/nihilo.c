#include "nihilo.h"

static nihilo_sim_t *s_current_sim;
static nihilo_sim_subinfo_t s_subinfo;
static Window *s_main_window;
static AppTimer *s_apptimer = NULL;

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

static void init( void ) {
	s_current_sim = &nihilo_tron;

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

#pragma once

#include <stdint.h>
#include <pebble.h>
#include <inttypes.h>

typedef enum {
	NIHILO_SIM_REASON_NONE,
	NIHILO_SIM_REASON_CLOCK,
	NIHILO_SIM_REASON_FRAMEBUFFER,
} nihilo_sim_reason_t;

typedef struct {
	uint32_t refresh_rate;
} nihilo_sim_subinfo_t;

typedef struct {
	void ( *create )( Window * );
	void ( *destroy )( void );
	void ( *subinfo )( nihilo_sim_subinfo_t * );
	void ( *simulate )( nihilo_sim_reason_t );
} nihilo_sim_t;

extern nihilo_sim_t nihilo_image;
extern nihilo_sim_t nihilo_numbers;
extern nihilo_sim_t nihilo_simplex;
extern nihilo_sim_t nihilo_tron;

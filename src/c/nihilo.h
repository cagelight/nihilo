#pragma once

#include <stdint.h>
#include <pebble.h>
#include <inttypes.h>
#include <math.h>

typedef enum {
	NIHILO_SIM_REASON_INIT,
	NIHILO_SIM_REASON_CLOCK
} nihilo_sim_reason_t;

typedef enum {
	NIHILO_SIM_RESULT_CONTINUE,
	NIHILO_SIM_RESULT_CLOSE,
} nihilo_sim_reult_t;

typedef enum {
	NIHILO_COOL_COLORSET_BLAZE,
	NIHILO_COOL_COLORSET_LIME,
	NIHILO_COOL_COLORSET_SPRING,
	NIHILO_COOL_COLORSET_DODGER,
	NIHILO_COOL_COLORSET_VIOLET,
	NIHILO_COOL_COLORSET_HOT,

	NIHILO_COOL_COLORSET_MAX,
} cool_colorsets_e;

GColor8 const *nihilo_get_colorset(cool_colorsets_e);

uint32_t nihilo_hash32(uint8_t *data, size_t len);

typedef struct {
	uint32_t refresh_rate;
} nihilo_sim_subinfo_t;

typedef struct {
	void ( *create )( Window * );
	void ( *destroy )( void );
	void ( *subinfo )( nihilo_sim_subinfo_t * );
	nihilo_sim_reult_t ( *simulate )( nihilo_sim_reason_t );
} nihilo_sim_t;

extern nihilo_sim_t nihilo_cellauto;
extern nihilo_sim_t nihilo_image;
extern nihilo_sim_t nihilo_numbers;
extern nihilo_sim_t nihilo_simplex;
extern nihilo_sim_t nihilo_tron;

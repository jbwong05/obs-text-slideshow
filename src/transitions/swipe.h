#pragma once
#include "transition.h"

#define SWIPE_TRANSITION_ID "swipe_transition"

obs_source_t *swipe_transition_source_create(obs_data_t *);
void swipe_transition_setup_properties(transition *, obs_properties_t *, bool);
transition *create_swipe_transition();
void destroy_swipe_transition(transition *);

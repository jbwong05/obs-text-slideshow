#pragma once
#include "transition.h"

#define CUT_TRANSITION_ID "cut_transition"

obs_source_t *cut_transition_source_create(obs_data_t *);
void cut_transition_setup_properties(transition *, obs_properties_t *, bool);
transition *create_cut_transition();
void destroy_cut_transition(transition *);

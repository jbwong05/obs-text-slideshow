#pragma once
#include "cut.h"
#include <vector>

using std::vector;

static vector<transition_vtable> transition_vtables = {
	{
        cut_transition_source_create,
	    cut_transition_setup_properties,
        create_cut_transition,
	    destroy_cut_transition
    }
};

vector<transition *> *load_transitions();
int match_transition(vector<transition *> *, const char *);
void destroy_transitions(vector<transition *> *);
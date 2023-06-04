#include "transition-utils.h"
#include <util/dstr.h>

vector<struct transition *> *load_transitions() {
    vector<struct transition *> *transitions = new vector<struct transition *>;

    for (unsigned int i = 0; i < transition_vtables.size(); i++) {
	    create_transition constructor = transition_vtables[i].constructor;
	    transition *new_transition = (*constructor)();
	    if (new_transition) {
		    transitions->push_back(new_transition);
	    }
    }

    return transitions;
}

int match_transition(vector<transition *> *transitions, const char *name) {
    for(unsigned int i = 0; i < transitions->size(); i++) {
        transition *curr_transition = transitions->at(i);
        if (astrcmpi(name, curr_transition->prop_val) == 0) {
            return i;
        }
    }

    return -1;
}

void destroy_transitions(vector<transition *> *transitions) {
    for (unsigned int i = 0; i < transitions->size(); i++) {
        transition *curr_transition = transitions->at(i);
        transition_vtables[i].destructor(curr_transition);
    }

    delete transitions;
}
#include "transition-utils.h"
#include "utils.h"
#include <util/dstr.h>

vector<struct transition *> *load_transitions()
{
	vector<struct transition *> *transitions =
		new vector<struct transition *>;

	for (unsigned int i = 0; i < transition_vtables.size(); i++) {
		create_transition constructor =
			transition_vtables[i].constructor;
		transition *new_transition = (*constructor)();
		if (new_transition) {
			transitions->push_back(new_transition);
		}
	}

	return transitions;
}

int match_transition(vector<transition *> *transitions, const char *name)
{
	for (unsigned int i = 0; i < transitions->size(); i++) {
		transition *curr_transition = transitions->at(i);
		if (astrcmpi(name, curr_transition->prop_val) == 0) {
			return i;
		}
	}

	return -1;
}

void destroy_transitions(vector<transition *> *transitions)
{
	for (unsigned int i = 0; i < transitions->size(); i++) {
		transition *curr_transition = transitions->at(i);
		transition_vtables[i].destructor(curr_transition);
	}

	delete transitions;
}

bool transition_selected_callback(void *data, obs_properties_t *props,
				  obs_property_t *p, obs_data_t *settings)
{
	vector<transition *> *transitions = (vector<transition *> *)data;
	const char *tr_name = obs_data_get_string(settings, S_TRANSITION);
	int new_transition_index = match_transition(transitions, tr_name);

	if (new_transition_index != -1) {
		for (unsigned int i = 0; i < transitions->size(); i++) {
			transition *curr_transition = transitions->at(i);
			unordered_set<const char *> *property_names =
				curr_transition->property_names;
			auto iter = property_names->begin();
			while (iter != property_names->end()) {
				const char *curr_name = *iter;
				set_vis(curr_name,
					(int)i == new_transition_index);
				iter++;
			}
		}

		return true;
	} else {
		return false;
	}

	return true;
}
#include "swipe.h"

#define S_SWIPE_DIRECTION "swipe_direction"
#define S_SWIPE_IN "swipe_in"
#define S_SWIPE_LEFT "left"
#define S_SWIPE_RIGHT "right"
#define S_SWIPE_UP "up"
#define S_SWIPE_DOWN "down"

#define T_TR_SWIPE T_TR_("Swipe")
#define TR_SWIPE "swipe"
#define SWIPE_LOCALE(text) T_TR_("Swipe." text)

#define DIRECTION "direction"
#define SWIPE_IN "swipe_in"

obs_source_t *swipe_transition_source_create(obs_data_t *settings) {
	const char *direction = obs_data_get_string(settings, S_SWIPE_DIRECTION);
    bool swipe_in = obs_data_get_bool(settings, S_SWIPE_IN);

    obs_data_t *data = obs_data_create();
    obs_data_set_string(data, DIRECTION, direction);
    obs_data_set_bool(data, SWIPE_IN, swipe_in);

    obs_source_t *source = obs_source_create_private(SWIPE_TRANSITION_ID, NULL, data);
    obs_data_release(data);

    return source;
}

void swipe_transition_setup_properties(transition *transition, obs_properties_t *props, bool visible) {
    obs_property_t *p = obs_properties_add_list(props, S_SWIPE_DIRECTION, SWIPE_LOCALE("Direction"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p, SWIPE_LOCALE("Left"), S_SWIPE_LEFT);
    obs_property_list_add_string(p, SWIPE_LOCALE("Right"), S_SWIPE_RIGHT);
    obs_property_list_add_string(p, SWIPE_LOCALE("Up"), S_SWIPE_UP);
    obs_property_list_add_string(p, SWIPE_LOCALE("Down"), S_SWIPE_DOWN);
    obs_property_set_visible(p, visible);
    transition->property_names->insert(S_SWIPE_DIRECTION);

    p = obs_properties_add_bool(props, S_SWIPE_IN, SWIPE_LOCALE("In"));
    obs_property_set_visible(p, visible);
    transition->property_names->insert(S_SWIPE_IN);
}

transition *create_swipe_transition() {
    transition *swipe_transition = (transition *)bzalloc(sizeof(transition));
    swipe_transition->id = SWIPE_TRANSITION_ID;
    swipe_transition->prop_name = T_TR_SWIPE;
    swipe_transition->prop_val = TR_SWIPE;
    swipe_transition->property_names = new unordered_set<const char *>;

    return swipe_transition;
}

void destroy_swipe_transition(transition *transition) {
    bfree(transition);
}

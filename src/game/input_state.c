#include "game/input_state.h"

void iPressKey(InputState_t *i_state, u8 key) {
	i_state->buttons_state |= key;
	return;
}

void iReleaseKey(InputState_t *i_state, u8 key) {
	i_state->buttons_state &= ~key;
	return;
}
#pragma once
// Internal FSM header. Do not call macros directly.
//
// This implementation attempts to simplify the creation of a transition table
// by declaring it as a function. We use macros to hide that function, proving
// an interface to declare simple transition tables. If needed, we could add
// simple guards quite easily.

// Forward-declares the state's transition function (prv_fsm_[state])
// and declares a FsmState object populated with its name and transition
// function
#define _FSM_DECLARE_STATE(state) \
  _FSM_STATE_TRANSITION(state);   \
  static FsmState state = { .name = #state, .table = prv_fsm_##state }

// This is used for both forward-declaration and the actual function declaration
// Since we're implementing the transition table as a function, we can't rely on
// a return value so we pass in a pointer to be modified on transitions.
#define _FSM_STATE_TRANSITION(state) \
  static void prv_fsm_##state(Fsm *fsm, const Event *e, bool *transitioned)

// Represents a transition through a conditional. This should only be used in
// transition functions. This keeps track of states and signals that a
// transition has occurred through the pointer. We then call the new state's
// output function if it exists.
#define _FSM_ADD_GUARDED_TRANSITION(event_id, guard, state)   \
  do {                                                        \
    if (e->id == event_id && (guard)(fsm, e, fsm->context)) { \
      fsm->last_state = fsm->current_state;                   \
      fsm->current_state = &state;                            \
      *transitioned = true;                                   \
                                                              \
      if (fsm->current_state->output != NULL) {               \
        fsm->current_state->output(fsm, e, fsm->context);     \
      }                                                       \
                                                              \
      return;                                                 \
    }                                                         \
  } while (0)

// Unguarded transitions just always return true for the guard.
#define _FSM_ADD_TRANSITION(event_id, state) \
  _FSM_ADD_GUARDED_TRANSITION(event_id, fsm_guard_true, state)

// Initializes an FSM state with an output function.
#define _fsm_state_init(state, output_func) \
  do {                                      \
    state.output = (output_func);           \
    state.name = #state;                    \
    state.table = prv_fsm_##state;          \
  } while (0)

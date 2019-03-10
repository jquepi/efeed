#include "gcode.h"
#include "fsm.h"
#include "g04.h"

void G04parse(char *line){
	G_pipeline *gref = G_parse(line);
	G_task *gt_new_task = add_empty_task();
	gt_new_task->steps_to_end = (gref->P * 1000) >> 10;
	gt_new_task->callback_ref = dwell_callback;
	gt_new_task->init_callback_ref = G04init_callback;
}

void G04init_callback(void){
	state.function = do_fsm_dwell;
  LL_TIM_SetSlaveMode(TIM3, LL_TIM_SLAVEMODE_DISABLED); // disable pulse generation
	TIM2->ARR = 30;
}

void do_fsm_dwell(state_t *s){
	// callback from TIM2
	state.current_task.steps_to_end--;
	if(state.current_task.steps_to_end == 0){
		//restore connection here?
			LL_TIM_SetSlaveMode(TIM3, LL_TIM_SLAVEMODE_TRIGGER);
	}

}

void dwell_callback(void){
// callback from TIM3
}

#include "timing.h"
#include "scheduling.h"

typedef struct{
  uint64_t time;
  uint32_t handle;
} timer;
int timer_comparator(timer a, timer b) {
  //NOTE: Done this way to avoid overflow
  if (a.time > b.time) {
    return 1;
  }else if (a.time == b.time) {
    return 0;
  }else{
    return -1;
  }
}
#define SORTED_LIST_TYPENAME timer_list
#define SORTED_LIST_TYPE timer
#define SORTED_LIST_CAPACITY MAX_DELAYS
#define SORTED_LIST_COMPARATOR timer_comparator
#include "sortedlist.h"

static uint64_t ticks = 0;

static timer_list timers;

void timing_init() {
  timer_list_init(&timers);
}

void timing_tick() {
  ticks++;
  timer first;
  if (timer_list_get_front(&timers, &first) && first.time < ticks){
    sched_err err = wake_task(first.handle);
    if (err == SCHED_ERR_OK) {
      timer_list_pop_front(&timers, NULL);
    }
  }
}

timer_err timing_delay_ms(uint32_t delay) {
  uint32_t current_task = get_current_task().task_handle;
  if (timer_list_size(&timers) >= MAX_DELAYS) {
    return TIMER_TOO_MANY_TIMERS;
  }
  timer task_timer = {
    .handle = current_task,
    .time = ticks + delay,
  };
  sched_err err = sleep_task(current_task);
  if (err != SCHED_ERR_OK) {
    return TIMER_SCHED_ERR;
  }
  if (!timer_list_insert(&timers, task_timer)){
    // TODO: Either ignore or hard fault
  }
  
  return TIMER_OK;
}

uint64_t timing_get_ticks() {
  return ticks;
}

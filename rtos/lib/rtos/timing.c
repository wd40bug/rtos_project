#include "timing.h"
#include "queue.h"
#include "scheduling.h"

static uint64_t ticks = 0;

typedef struct {
  uint64_t time;
  size_t next;
  uint32_t handle;
} timing_node;
static timing_node timers[MAX_DELAYS];
static size_t head = MAX_DELAYS;
static uint32_t num_timers = 0;

QUEUE_INIT(size_t, MAX_DELAYS);
static size_t_MAX_DELAYS_queue free_queue;

void timing_init() {
  size_t_MAX_DELAYS_queue_init(&free_queue);
  for (int i = 0; i < MAX_DELAYS; i++) {
    size_t_MAX_DELAYS_queue_enqueue(&free_queue, i);
  }
}

void timing_tick() {
  ticks++;
  if (head < MAX_DELAYS && timers[head].time <= ticks) {
    sched_err err = wake_task(timers[head].handle);
    if (err == SCHED_ERR_OK) {
      size_t_MAX_DELAYS_queue_enqueue(&free_queue, head);
      head = timers[head].next;
      num_timers--;
    }
  }
}

timer_err timing_delay_ms(uint32_t delay) {
  uint32_t current_task = get_current_task().task_handle;
  if (num_timers >= MAX_DELAYS) {
    return TIMER_TOO_MANY_TIMERS;
  }
  size_t new_index;
  if (!size_t_MAX_DELAYS_queue_dequeue(&free_queue, &new_index)) {
    return TIMER_TOO_MANY_TIMERS;
  }
  timing_node new_node = {
      .time = ticks + delay,
      .handle = current_task,
      .next = MAX_DELAYS,
  };
  sched_err err = sleep_task(current_task);
  if (err != SCHED_ERR_OK) {
    return TIMER_SCHED_ERR;
  }
  if (head >= MAX_DELAYS || timers[head].time > new_node.time) {
    new_node.next = head;
    head = new_index;
  } else {
    size_t node = head;
    size_t next_node = timers[head].next;
    while (next_node < MAX_DELAYS &&
           timers[next_node].time < new_node.time) {
      node = next_node;
      next_node = timers[next_node].next;
    }
    timers[node].next = new_index;
    new_node.next = next_node;
  }
  num_timers++;
  timers[new_index] = new_node;
  return TIMER_OK;
}

uint64_t timing_get_ticks() {
  return ticks;
}

#include "scheduling.h"
#include "printf.h"
#include "queue.h"
#include "timing.h"
#include <stddef.h>
#include <stm32l476xx.h>
#include <string.h>

// NOTE: No tasks can ever be removed for now XD

const uint32_t MAX_INTERRUPT_PRIORITY = UINT32_MAX;
#define MAX_TASK_STACK_SIZE 512 // 2kb

typedef enum { TASK_READY, TASK_SLEEPING, TASK_DONE } task_status;
typedef enum { NORMAL_TASK, IDLE_TASK, RESTORE_TASK } task_specialness;

typedef struct {
  volatile uint32_t* stack_top;
  task_status status;
  task_data task_data;
  task_specialness specialness;
  uint64_t ticks_started;
} task_data_internal;
typedef task_data_internal* tdi;

QUEUE_INIT(tdi, MAX_TASKS);
tdi_MAX_TASKS_queue priorities[PRIORITIES];
int32_t num_done = 0;

static bool is_running = false;
// To potentially restore the calling function when all tasks end
volatile uint32_t restore_stack[MAX_TASK_STACK_SIZE];
volatile task_data_internal restore_task = {
    .stack_top = restore_stack + MAX_TASK_STACK_SIZE - 1, // Filled in on switch
    .status = TASK_READY,
    .specialness = RESTORE_TASK,
    .task_data = (task_data){
                             .task_handle = MAX_TASKS, // Invalid value
                             .priority = PRIORITIES
    },
    .ticks_started = UINT64_MAX
};

volatile task_data_internal idle_task;
task_err idle(task_data* td) {
  (void)td;
  while (1) {
    __WFE();
  }
}

void scheduling_return() {
  __disable_irq();
  // TODO: this
  __enable_irq();
}

// current task!
// Set to restore_task to save stack properly
volatile task_data_internal* current_task = &restore_task;

// list of tasks
task_data_internal tasks[MAX_TASKS];
// number of tasks
size_t num_tasks = 0;
// Individual task stacks
// NOTE: must be aligned to 8 bytes according to ARM docs
uint32_t stacks[MAX_TASKS][MAX_TASK_STACK_SIZE] __attribute((aligned(8)));

// Start context switch
void generate_pend_sv() {
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

static task_data_internal create_task(
    TASK task, uint32_t handle, uint32_t priority, uint8_t specialness
) {
  // Stacks are top down
  uint32_t* stack_top = stacks[handle] + MAX_TASK_STACK_SIZE - 1;
  xPSR_Type xPSR = {
      .b = {
            .ISR = 0,
            .ICI_IT_1 = 0,
            .GE = 0,
            .T = 1,
            .ICI_IT_2 = 0,
            .Q = 0,
            .V = 0,
            .C = 0,
            .Z = 0,
            .N = 0,
            }
  };
  ShortStackFrame short_frame = {
      .r0 = (uint32_t)&tasks[handle].task_data,
      .r1 = 0,
      .r2 = 0,
      .r3 = 0,
      .r12 = 0,
      // TODO: return address
      .lr = (uint32_t)scheduling_return,
      .return_address = (uint32_t*)task,
      .xpsr = xPSR.w
  };
  FullStackFrame stack = {
      .r4 = 0,
      .r5 = 0,
      .r6 = 0,
      .r7 = 0,
      .r8 = 0,
      .r9 = 0,
      .r10 = 0,
      .r11 = 0,
      .exc_return = EXC_RETURN_THREAD_PSP,
      .short_frame = short_frame
  };
  size_t size_scaled = sizeof(stack) / sizeof(uint32_t);
  stack_top -= size_scaled;
  memcpy(stack_top, &stack, sizeof(stack));

  return (task_data_internal){
      .stack_top = stack_top,
      .task_data =
          (task_data){
                      .task_handle = handle,
                      .priority = priority,
                      },
      .specialness = specialness,
      .status = TASK_READY
  };
}

sched_err scheduling_init() {
  // Set SysTick to highest possible priority
  SCB->SHP[1] |= 0x11 << 24;
  // Set PenSV to lower priority so it won't pre-empt a handler
  SCB->SHP[2] |= 0xFF << 24;

  idle_task = create_task(idle, MAX_TASKS, PRIORITIES, IDLE_TASK);
  return SCHED_ERR_OK;
}

sched_err scheduling_run() {
  // Point PSP into IDLE_TASK stack
  __asm__ volatile("mov r0, %[IDLE_STACK]\n\t"
                   "msr psp, r0\n\t"
                   "isb"
                   :
                   : [IDLE_STACK] "r"(restore_task.stack_top)
                   : "r0");

  // Start switching!
  is_running = true;
  generate_pend_sv();
  // Anything past this point should be impossible
  printf("Uhhhhhhhhh\n");
  return SCHED_ERR_UNEXPECTED_RETURN;
}

sched_err scheduling_add_task(TASK task, uint32_t priority) {
  if (priority >= PRIORITIES) {
    return SCHED_ERR_INVALID_PRIORITY;
  }
  size_t handle = num_tasks++;

  tasks[handle] = create_task(task, handle, priority, NORMAL_TASK);
  if (!tdi_MAX_TASKS_queue_enqueue(&priorities[priority], &tasks[handle])) {
    return SCHED_ERR_TOO_MANY_TASKS;
  }
  return SCHED_ERR_OK;
}

void next_task(void) {
  if (current_task->status == TASK_READY &&
      current_task->specialness == NORMAL_TASK) {
    tdi_MAX_TASKS_queue_enqueue(
        &priorities[current_task->task_data.priority],
        (task_data_internal*)current_task
    );
  }
  task_data_internal* next_task = NULL;
  for (int i = 0; i < PRIORITIES; i++) {
    if (tdi_MAX_TASKS_queue_dequeue(&priorities[i], &next_task)) {
      break; // Valid task
    }
  }
  if (next_task == NULL) {
    if (num_done == num_tasks) {
      next_task = (task_data_internal*)&restore_task;
    } else {
      next_task = (task_data_internal*)&idle_task;
    }
  }
  current_task = next_task;
}

// Side stuff
void PendSV_C() {
  SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
}

void PendSV_Handler() {
  __asm__ volatile(
      // Call to a C function to define some debugging stuff
      "push {lr, r12, %[CurrentTask]}\n\t"
      "bl PendSV_C\n\t"
      "pop {lr, r12, %[CurrentTask]}\n\t"
      // Load Process Stack Pointer into r0
      "mrs r0, psp\n\t"
      // Load value of current_task into r2
      "ldr r2, [%[CurrentTask]]\n\t"
      // Load stack_top value into r1
      "ldr r1, [r2]\n\t"
      // If FPU regs need to be pushed, push them
      "tst r14, #0x10\n\t"
      "it eq\n\t"
      // v(vectorized or floating point) stm (store multiple) db (decrement before) eq (equals branch of it) r0 (location) ! (write-back)
      "vstmdbeq r0!, {s16-s31}\n\t"
      // Save GPRs
      "stmdb r0!, {r4-r11, r14}\n\t"
      // Save stack position to stack_top field
      "str r0, [r2]\n\t"

      // --Actually switch contexts--
      // This calls a function to determine which task to switch into
      // Save register which would be clobbered
      "stmdb sp!, {r0, %[CurrentTask]}\n\t"
      // Disable interrupts for context switch
      "mov r0, MAX_INTERRUPT_PRIORITY\n\t"
      "msr basepri, r0\n\t"
      // Flush instruction pipeline
      "isb\n\t"
      // Switch
      "bl next_task\n\t"
      // Re-enable interrupt
      "mov r0, #0\n\t"
      "msr basepri, r0\n\t"
      "ldmia sp!, {r0, %[CurrentTask]}\n\t"
      // get current task into r2
      "ldr r2, [%[CurrentTask]]\n\t"
      // get new value of stack_top field
      "ldr r0, [r2]\n\t"
      // Restore the core registers
      "ldmia r0!, {r4-r11, r14}\n\t"
      // If the process was using the FPU, restore those
      "tst r14, #0x10\n\t"
      "it eq\n\t"
      "vldmiaeq r0!, {s16-s31}\n\t"
      // Lastly, set the psp to r0 and go to the new task
      "msr psp, r0\n\t"
      "bx r14\n\t"

      // AND THE REST IF THE REGISTERS ARE RESTORED AUTO-MAGICALLY!!!
      :
      : [CurrentTask] "r"(&current_task),
        [MAX_INTERRUPT_PRIORITY] "i"(MAX_INTERRUPT_PRIORITY)
      :
  );
  __builtin_unreachable();
}

#define TICKS_PER_TASK 10

bool should_switch() {
  static uint32_t tick_cnt = 0;
  bool result = is_running && tick_cnt == 0;
  tick_cnt = (tick_cnt + 1) % TICKS_PER_TASK;
  return result;
}

void scheduling_tick() {
  if (should_switch()) {
    generate_pend_sv();
  }
}

sched_err yield() {
  if (!is_running) {
    return SCHED_ERR_NOT_RUNNING;
  }
  generate_pend_sv();
  return SCHED_ERR_OK;
}

task_data get_current_task() {
  __disable_irq();
  task_data ret = current_task->task_data;
  __enable_irq();
  return ret;
}

sched_err wake_task(uint32_t handle) {
  if (handle >= MAX_TASKS) {
    return SCHED_ERR_INVALID_HANDLE;
  }
  task_data_internal* task = &tasks[handle];
  if (task->status != TASK_SLEEPING) {
    return SCHED_ERR_NOT_ASLEEP;
  }
  uint32_t task_prio = task->task_data.priority;
  if (task_prio >= PRIORITIES) {
    return SCHED_ERR_INVALID_PRIORITY;
  }
  tasks[handle].status = TASK_READY;
  tdi_MAX_TASKS_queue_enqueue(&priorities[task_prio], task);
  if (task_prio < current_task->task_data.priority) {
    generate_pend_sv();
  }
  return SCHED_ERR_OK;
}

sched_err sleep_task(uint32_t handle) {
  if (handle >= MAX_TASKS) {
    return SCHED_ERR_INVALID_HANDLE;
  }
  task_data_internal* task = &tasks[handle];
  if (task->status != TASK_READY) {
    return SCHED_ERR_NOT_READY;
  }
  task->status = TASK_SLEEPING;
  if (current_task->task_data.task_handle == handle) {
    generate_pend_sv();
  }
  return SCHED_ERR_OK;
}

#include "scheduling.h"
#include "common.h"
#include "svc.h"
#include <stddef.h>
#include <stm32l476xx.h>
#include <string.h>

// NOTE: No tasks can ever be removed for now XD

const uint32_t MAX_INTERRUPT_PRIORITY = UINT32_MAX;
#define MAX_TASKS 4
#define MAX_TASK_STACK_SIZE 512 // 2kb

enum { TASK_RUNNING, TASK_READY, TASK_STOPPED, TASK_DEAD };

typedef struct {
  volatile uint32_t* stack_top;
  uint8_t task_status;
  task_data task_data;
} task_data_internal;

static bool is_running = false;
// To potentially restore the calling function when all tasks end
volatile uint32_t restore_stack[MAX_TASK_STACK_SIZE];
volatile task_data_internal restore_task = {
    .stack_top = restore_stack + MAX_TASK_STACK_SIZE - 1, // Filled in on switch
    .task_status = TASK_READY,
    .task_data = (task_data){
                             .task_handle = MAX_TASKS, // Invalid value
    }
};

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

sched_err scheduling_init() {
  return SCHED_ERR_OK;
}

sched_err scheduling_run() {
  // Set SysTick to highest possible priority
  SCB->SHP[1] |= 0x11 << 24;
  // Set PenSV to lower priority so it won't pre-empt a handler
  SCB->SHP[2] |= 0xFF << 24;
  // Configure clk to tick
  SysTick->LOAD = SysTick->CALIB; // 10ms according to ARM, but I think it's 1ms
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk;
  // Make sure in priviledged mode
  if ((__get_CONTROL() & CONTROL_nPRIV_Msk) == 0) {
    // TODO:
    // EnablePrivilegedMode();
  }
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
  // Theoretically anything past this point should be impossible
  printf("Uhhhhhhhhh\n");
  return SCHED_ERR_UNEXPECTED_RETURN;
}

sched_err scheduling_add_task(TASK task) {
  if (num_tasks == MAX_TASKS) {
    return SCHED_ERR_TOO_MANY_TASKS;
  }
  size_t handle = num_tasks++;
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
      .lr = 0,
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

  tasks[handle] = (task_data_internal){
      .stack_top = stack_top,
      .task_data =
          (task_data){
                      .task_handle = handle,
                      },
      .task_status = TASK_READY
  };
  return SCHED_ERR_OK;
}

void TaskSwitchContext(void) {
  // TODO: Account for task states and priority
  static size_t task_index = 0;
  if (num_tasks == 0) {
    current_task = &restore_task;
    return;
  }
  current_task = &tasks[task_index];
  task_index = (task_index + 1) % num_tasks;
}

void PendSV_Handler() {
  SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
  __asm__ volatile(
      // Load Process Stack Pointer into r0
      "mrs r0, psp\n\t"
      // Load value of current_task into r2
      "ldr r2, %[CurrentTask]\n\t"
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
      "stmdb sp!, {r0, r3}\n\t"
      // Disable interrupts for context switch
      "mov r0, MAX_INTERRUPT_PRIORITY\n\t"
      "msr basepri, r0\n\t"
      // Flush instruction pipeline
      "isb\n\t"
      // Switch
      "bl TaskSwitchContext\n\t"
      // Re-enable interrupt
      "mov r0, #0\n\t"
      "msr basepri, r0\n\t"
      "ldmia sp!, {r0, r3}\n\t"
      // get current task into r2
      "ldr r2, %[CurrentTask]\n\t"
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
      : [CurrentTask] "m"(current_task),
        [MAX_INTERRUPT_PRIORITY] "i"(MAX_INTERRUPT_PRIORITY)
      : "r0", "r1", "r2", "r12", "r14"
  );
  __builtin_unreachable();
}

bool should_switch() {
  return is_running;
}

void SysTick_Handler(void) {
  HAL_IncTick();
  if (should_switch()) {
    // generate_pend_sv();
  }
}

#define HALT_IF_DEBUGGING()                                                    \
  do {                                                                         \
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {                    \
      __asm__ volatile("bkpt 1");                                              \
    }                                                                          \
  } while (0)

void HardFault_Handler(void) {
  ShortStackFrame* CSF;
  __asm__ volatile("tst lr, #4 \n\t"
                   "ite eq \n\t"
                   "mrseq r0, msp\n\t"
                   "mrsne r0, psp\n\t"
                   "str r0, %[CSF]\n\t"
                   : [CSF] "=m"(CSF)
                   :
                   : "r0");

  HALT_IF_DEBUGGING();

  printf("Hard fault!\n");
  if (SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk) {
    printf("Usage fault\n");
  }
  if (SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk) {
    printf("Bus fault\n");
    if (SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk) {
      printf("Imprecise... fuck\n");
    }
  }
  if (SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk) {
    printf("MemManage fault\n");
  }
  while (1)
    ;
}

sched_err yield() {
  generate_pend_sv();
  return SCHED_ERR_OK;
}

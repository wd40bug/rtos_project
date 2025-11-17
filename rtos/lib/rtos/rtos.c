#include "rtos.h"
#include "messaging.h"
#include "printf.h"
#include "scheduling.h"
#include "serial.h"
#include "timing.h"
#include "frames.h"
#include <stm32l476xx.h>

static void priorities() {
  // Priorities
  NVIC_SetPriorityGrouping(0); // All bits are for group priority
  // 0: TIM1
  NVIC_SetPriority(TIM2_IRQn, 0x0);
  // 1: SVCall and USART2
  NVIC_SetPriority(SVCall_IRQn, 0x1);
  NVIC_SetPriority(USART2_IRQn, 0x1);
  // F: PendSV and SysTick
  NVIC_SetPriority(SysTick_IRQn, 0xF);
  NVIC_SetPriority(PendSV_IRQn, 0xF);
}

void rtos_init() {
  priorities();

  // Configure clk to tick
  if (SysTick_Config(SystemCoreClock / 1000) == 1) {
    while (1)
      ;
  };

  while (timing_get_ticks() != 2000) {
  }
  timing_init();
  init_serial(115200);
  scheduling_init();
  messaging_init();
}

void rtos_run() {
  scheduling_run();
}

void SysTick_Handler(void) {
  timing_tick();
  scheduling_tick();
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
  while (1) {
  };
}

uint64_t ms_since_start(){
  return timing_get_ticks();
}

#include "svc.h"
#include "messaging.h"
#include "stm32l476xx.h"
#include "timing.h"

void gain_priviledge() {
  __asm__ volatile("svc #0\n\t");
}

void relinquish_priviledge() {
  __asm__ volatile("svc #1\n\t");
}

timer_err delay_ms(uint32_t ms) {
  timer_err ret;
  __asm__ volatile("mov r0, %[ms]\n\t"
                   "svc #2\n\t"
                   "mov %[ret], r0\n\t"
                   : [ret] "=r"(ret)
                   : [ms] "r"(ms));
  return ret;
}

message_q_error message_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
) {
  register message_q_error ret asm("r0");
  register TASK_HANDLE handle_r asm("r0") = handle;
  register MESSAGE_QUEUE_HANDLE* q_handle_r asm("r1") = q_handle;
  __asm__ volatile("svc #3\n\t"
                   : [ret] "=r"(ret)
                   : [handle] "r"(handle_r), [q_handle] "r"(q_handle_r));
  return ret;
}

void SVC_Handler(void) {
  ShortStackFrame* svc_args;
  __asm__ volatile(".global SVC_Handler_Main\n\t"
                   "TST lr, #4\n\t"
                   "ITE EQ\n\t"
                   "MRSEQ r0, MSP\n\t"
                   "MRSNE r0, PSP\n\t"
                   "mov %[svc_args], r0\n\t"
                   : [svc_args] "=r"(svc_args)::"r0");
  unsigned int svc_number;

  /*
   * Stack contains:
   * r0, r1, r2, r3, r12, r14, the return address and xPSR
   * First argument (r0) is svc_args[0]
   */
  svc_number = ((char*)svc_args->return_address)[-2];
  switch (svc_number) {
  case 0: /* gain_priviledge */
    __set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
    break;
  case 1: /* relinquish_priviledge */
    __set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
    break;
  case 2:
    svc_args->r0 = timing_delay_ms(svc_args->r0);
    break;
  case 3: /* message_queue_create */
    svc_args->r0 = messaging_queue_create(
        svc_args->r0,
        (MESSAGE_QUEUE_HANDLE*)svc_args->r1
    );
    break;
  default: /* unknown SVC */
    // TODO: Fault
    while (1) {
    }
    break;
  }
}

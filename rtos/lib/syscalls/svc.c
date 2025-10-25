#include "svc.h"
#include "scheduling.h"
#include "stm32l476xx.h"
void SVC_Init(void) {
  // Just to make sure this file is compiled
}

void SVC_Handler(void) {
  __asm__ volatile(".global SVC_Handler_Main\n\t"
                   "TST lr, #4\n\t"
                   "ITE EQ\n\t"
                   "MRSEQ r0, MSP\n\t"
                   "MRSNE r0, PSP\n\t"
                   "B SVC_Handler_Main\n\t");
}

void SVC_Handler_Main( ShortStackFrame *svc_args )
{
  unsigned int svc_number;

  /*
  * Stack contains:
  * r0, r1, r2, r3, r12, r14, the return address and xPSR
  * First argument (r0) is svc_args[0]
  */
  svc_number = ( ( char * )svc_args->return_address )[ -2 ] ;
  switch( svc_number )
  {
    case 0:  /* EnablePrivilegedMode */
      __set_CONTROL( __get_CONTROL( ) & ~CONTROL_nPRIV_Msk ) ;
      break;
    case 1: /* DisablePrivilegedMode */
      __set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk) ;
      break;
    default:    /* unknown SVC */
      break;
  }
}

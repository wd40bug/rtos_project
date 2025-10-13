#include "scheduling.h"

void context_switch() {
  __asm volatile ( 
    "mrs r0, psp"
  );
}

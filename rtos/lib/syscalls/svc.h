#include <stdint.h>
#define EnablePrivilegedMode() __asm("SVC #0")
#define DisablePrivilegedMode() __asm("SVC #1")

void SVC_Init();

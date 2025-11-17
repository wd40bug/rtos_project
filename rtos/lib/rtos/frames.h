#include <stdint.h>
#ifndef FRAMES_H
typedef struct __attribute__((packed)) {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t* return_address;
  uint32_t xpsr;
} ShortStackFrame;

typedef struct __attribute__((packed)) {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t exc_return;
  ShortStackFrame short_frame;
} FullStackFrame;
#endif /* ifndef FRAMES_H */

#include "serial.h"
#include "queue.h"
#include "scheduling.h"
#include "stm32l476xx.h"
#include <stdbool.h>
#include <stddef.h>

#define QUEUE_SIZE 256
QUEUE_INIT(char, QUEUE_SIZE);
#define CBUFFERS_LEN MAX_TASKS + 1
char_QUEUE_SIZE_queue cbuffers[CBUFFERS_LEN];
char_QUEUE_SIZE_queue flush_buffer;

static void enable_txe_interrupt() {
  USART2->CR1 |= USART_CR1_TXEIE;
}

static void disable_txe_interrupt() {
  USART2->CR1 &= ~USART_CR1_TXEIE;
}

static void flush(char_QUEUE_SIZE_queue* Q) {
  __disable_irq();
  char item = '\0';
  while (char_QUEUE_SIZE_queue_dequeue(Q, &item)) {
    if (!char_QUEUE_SIZE_queue_enqueue(&flush_buffer, item)) {
      // TODO: Handle full flush queue
      // yield???
      while (1) {
      }
    }
    if (item == '\n') {
      break;
    }
  }
  __enable_irq();
  enable_txe_interrupt();
}

void init_serial(uint32_t baud) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // GPIOA clk enable

  GPIOA->MODER &= ~(
      GPIO_MODER_MODE2 | GPIO_MODER_MODE3
  ); // Mask out bits for GPIO mode (GPIO MODER doesn't default to 0)
  GPIOA->MODER |=
      2 << GPIO_MODER_MODE2_Pos; // Alternative function for PA.2 (TX)
  GPIOA->MODER |=
      2 << GPIO_MODER_MODE3_Pos; // Alternative function for PA.3 (RX)
  GPIOA->AFR[0] |= 7 << GPIO_AFRL_AFSEL2_Pos; // AF7 for PA.2
  GPIOA->AFR[0] |= 7 << GPIO_AFRL_AFSEL3_Pos; // AF7 for PA.3

  RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; // USART2 clk enable

  uint16_t uart_div = SystemCoreClock / baud;
  USART2->BRR = (uart_div / 16) << USART_BRR_DIV_MANTISSA_Pos |
                (uart_div % 16) << USART_BRR_DIV_FRACTION_Pos; // Baud rate

  USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE; // enable uart
  NVIC_EnableIRQ(USART2_IRQn);

  char_QUEUE_SIZE_queue_init(&flush_buffer);
  for (int i = 0; i < MAX_TASKS; i++) {
    char_QUEUE_SIZE_queue_init(&cbuffers[i]);
  }
}

void _putchar(char character) {
  char_QUEUE_SIZE_queue* Q = &cbuffers[get_current_task().task_handle];
  if (!char_QUEUE_SIZE_queue_enqueue(Q, character)) {
    // TODO: Handle full queue
    while (1) {
    }
  } else if (character == '\n') {
    flush(Q);
  }
}

void USART2_IRQHandler() {
  if (char_QUEUE_SIZE_queue_is_empty(&flush_buffer)) {
    disable_txe_interrupt();
  } else if ((USART2->ISR & USART_ISR_TXE_Msk)) {
    char item;
    char_QUEUE_SIZE_queue_dequeue(&flush_buffer, &item);
    USART2->TDR = item;
  }
}

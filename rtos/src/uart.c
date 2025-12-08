#include "stm32l476xx.h"
#include "printf.h"
#include <ctype.h>
#include <stddef.h>

#define QUEUE_CAPACITY 256
#define QUEUE_TYPENAME recv_queue
#define QUEUE_TYPE char
#include "queue.h"

recv_queue q;

void uart_init() {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

  GPIOA->MODER &= ~(GPIO_MODER_MODE9 | GPIO_MODER_MODE10);

  GPIOA->MODER |= 2 << GPIO_MODER_MODE9_Pos;
  GPIOA->MODER |= 2 << GPIO_MODER_MODE10_Pos;

  GPIOA->AFR[1] |= 7 << GPIO_AFRH_AFSEL9_Pos;
  GPIOA->AFR[1] |= 7 << GPIO_AFRH_AFSEL10_Pos;

  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  uint16_t uart_div = SystemCoreClock / 115200;
  USART1->BRR = (uart_div / 16) << USART_BRR_DIV_MANTISSA_Pos |
                (uart_div % 16) << USART_BRR_DIV_FRACTION_Pos; // Baud rate
  
  USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
  NVIC_EnableIRQ(USART1_IRQn);
  recv_queue_init(&q);
}

void write_uart(char* data, size_t num) {
  size_t sent = 0;
  while (sent < num) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = data[sent++];
  }
}

size_t read_uart(char* buffer, size_t size) {
  return recv_queue_dequeue_all(&q, size, buffer);
}

size_t read_uart_until(char* buffer, size_t max_size, char to_find){
  size_t recv = 0;
  while(recv < max_size - 1) {
    char received;
    while (!recv_queue_dequeue(&q, &received));
    buffer[recv++] = received;
    if (received == to_find){
      break;
    }
  }
  buffer[recv] = '\0';
  return recv;
}

void USART1_IRQHandler() {
  if (USART1->ISR & USART_ISR_RXNE_Msk) {
    recv_queue_enqueue(&q, USART1->RDR);
  }
}

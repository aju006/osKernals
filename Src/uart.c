#include "uart.h"

#define GPIOAEN (1U<<17)
#define UART2EN (1U<<17)

#define SYS_FREQ 8000000
#define APB1_CLK SYS_FREQ
#define UART_BAUDRATE 115200

#define CR1_TE (1U<<3)
#define CR1_UE (1U<<0)

#define SR_TXE (1U<<7)

static void uart_set_bd(uint32_t periph_clk,uint32_t baudrate);
static uint16_t uart_calculate_bd(uint32_t periph_clk,uint32_t baudrate);
static void uart_write(int ch);

int __io_putchar(int ch){
	uart_write(ch);
	return ch;
}

void uart_tx_init(void){
	//Enable clock access to GPIOA
	RCC->AHBENR|=GPIOAEN;
	//Set PA2 to alternate function mode
	GPIOA->MODER&=~(1U<<4);
	GPIOA->MODER|=(1U<<5);
	//alternate function type to AF1; AFR[0] MEANS AFRLow
	GPIOA->AFR[0]|=(1U<<8);
	GPIOA->AFR[0]&=~(1U<<9);
	GPIOA->AFR[0]&=~(1U<<10);
	GPIOA->AFR[0]&=~(1U<<11);
	//Enable clock access to UART
	RCC->APB1ENR|=UART2EN;
	//Configure baudrate
	uart_set_bd(APB1_CLK, UART_BAUDRATE);
	//configure transfer direction
	USART2->CR1=CR1_TE;
	//Enable UART module
	USART2->CR1|=CR1_UE;
}
static void uart_write(int ch){
	//make sure transmit data register is empty
	while(!(USART2->ISR & SR_TXE)){}
	//write to transmit data register
	USART2->TDR=(ch & 0xFF);
}

static void uart_set_bd(uint32_t periph_clk,uint32_t baudrate){
	USART2->BRR=uart_calculate_bd(periph_clk, baudrate);
}
static uint16_t uart_calculate_bd(uint32_t periph_clk,uint32_t baudrate){
	return ((periph_clk+(baudrate/2U))/baudrate);
}

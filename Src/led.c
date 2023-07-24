#include "led.h"

#define GPIOAEN (1U<<17) //0000 0000 0000 0010 0000 0000 0000 0000
#define LED_PIN (1U<<5)

void led_init(void){
	/*Enable clock access to port A(LED in Port A pin 5)*/
	RCC->AHBENR|=GPIOAEN;
	/*set led pin as output pin*/
	GPIOA->MODER|=(1U<<10);
	GPIOA->MODER&=~(1U<<11);
}

void led_on(void){
	/*Set LED Pin High(PA5)*/
	GPIOA->ODR|=LED_PIN;

}

void led_off(void){
	/*Set LED Pin High(PA5)*/
	GPIOA->ODR&=~LED_PIN;
}

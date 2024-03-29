#include "osKernel.h"

#define NUM_OF_THREADS 3
#define STACKSIZE 100

#define BUS_FREQ 8000000

void osSchedulerLaunch(void);

uint32_t MILLIS_PRESCALER;

#define CTRL_ENABLE (1U<<0)
#define CTRL_TICKINT (1U<<1)
#define CTRL_CLCKSRC (1U<<2)
#define CTRL_COUNTFLAG (1U<<16)
#define SYSTICK_RST 0

struct tcb{
	int32_t *stackPtr;
	struct tcb *nextPtr;
};
typedef struct tcb tcbType;

tcbType tcbs[NUM_OF_THREADS];
tcbType *currentPtr;

//EACH THREAD WILL HAVE A STACK OF SIZE //STACKSIZE*4 Bytes//
int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

void osKernelStackInit(int i){
	tcbs[i].stackPtr=&TCB_STACK[i][STACKSIZE-16];//PC
	//set BIT 24 in PSR to activate thumb mode
	TCB_STACK[i][STACKSIZE-1]=(1U<<24);
	/**@Note : Block below is optional, for debugging purpose only*/
	  /**Dummy stack content*/
		TCB_STACK[i][STACKSIZE-3]  = 0xAAAAAAAA;    /*R14 i.e LR*/
		TCB_STACK[i][STACKSIZE-4]  = 0xAAAAAAAA;    /*R12*/
		TCB_STACK[i][STACKSIZE-5]  = 0xAAAAAAAA;    /*R3*/
		TCB_STACK[i][STACKSIZE-6]  = 0xAAAAAAAA;    /*R2*/
		TCB_STACK[i][STACKSIZE-7]  = 0xAAAAAAAA;    /*R1*/
		TCB_STACK[i][STACKSIZE-8]  = 0xAAAAAAAA;    /*R0*/

		TCB_STACK[i][STACKSIZE-9]  = 0xAAAAAAAA;    /*R11*/
		TCB_STACK[i][STACKSIZE-10] = 0xAAAAAAAA;   /*R10*/
		TCB_STACK[i][STACKSIZE-11] = 0xAAAAAAAA;   /*R9*/
		TCB_STACK[i][STACKSIZE-12] = 0xAAAAAAAA;   /*R8*/
		TCB_STACK[i][STACKSIZE-13] = 0xAAAAAAAA;   /*R7*/
		TCB_STACK[i][STACKSIZE-14] = 0xAAAAAAAA;   /*R6*/
		TCB_STACK[i][STACKSIZE-15] = 0xAAAAAAAA;   /*R5*/
		TCB_STACK[i][STACKSIZE-16] = 0xAAAAAAAA;   /*R4*/
}

uint8_t osKernelAddThreads(void(*task0)(void),void(*task1)(void),void(*task2)(void)){
	//Disable Global Interrupts
	__disable_irq();
	tcbs[0].nextPtr=&tcbs[1];
	tcbs[1].nextPtr=&tcbs[2];
	tcbs[2].nextPtr=&tcbs[0];

	//Initialize stack for thread0
	osKernelStackInit(0);
	//Initialize PC
	TCB_STACK[0][STACKSIZE-2]=(int32_t)(task0);

	//Initialize stack for thread0
	osKernelStackInit(1);
	//Initialize PC
	TCB_STACK[1][STACKSIZE-2]=(int32_t)(task1);

	//Initialize stack for thread0
	osKernelStackInit(2);
	//Initialize PC
	TCB_STACK[2][STACKSIZE-2]=(int32_t)(task2);

	//Start from thread0
	currentPtr=&tcbs[0];

	//Enable global interrupts
	__enable_irq();

	return 1;
}
void osKernelInit(void){
	MILLIS_PRESCALER=(BUS_FREQ/1000);
}
void osKernelLaunch(uint32_t quanta){
	//Reset SysTick
	SysTick->CTRL=SYSTICK_RST;

	//Clear SysTick current value register
	SysTick->VAL=0;

	//Load quanta
	SysTick->LOAD=(quanta*MILLIS_PRESCALER)-1;

	//Set SysTick to low priority
	NVIC_SetPriority(SysTick_IRQn,15);

	//Enable SysTick, select internal clock
	SysTick->CTRL=(CTRL_CLCKSRC|CTRL_ENABLE);

	//Enable SysTick interrupt
	SysTick->CTRL|=CTRL_TICKINT;

	//Launch scheduler
	osSchedulerLaunch();
}

__attribute__((naked)) void SysTick_Handler(void){
	/*Suspend current thread*/

	//Disable interrupts
	__asm("CPSID I");

	//Save R4,R5,R6,R7,R8,R9,R10,R11
	__asm("PUSH {R4-R11}");//uncorrected

	//Load address of current into R0
	__asm("LDR R0, =currentPtr");

	//Load R1 from address equals R0, i.e currentPtr = r1
	__asm("LDR R1,[R0]");

	//Store Cortex-M SP at address equals R1, i.e Save SP into tcb
	__asm("STR SP,[R1]");

	/*Choose next thread*/

	//Load R1 from a location 4 bytes above address R1, i.e R1 = currentPtr->next
	__asm("LDR R1,[R1,#4]");

	//Store R1 at address equals R0, i.e currentPtr = R1
	__asm("STR R1,[R0]");

	//Load Cortex-M SP from address equals R1, i.e SP = currentPtr->stackPtr
	__asm("LDR SP,[R1]");

	//Restore R4,R5,R6,R7,R8,R9,R10,R11
	__asm("POP {R4-R11}");//uncorrected

	//Enable global interrupts
	__asm("CPSIE I");

	//Return from exception and restore R0,R1,R2,R3,R12,lr,pc,psr
	__asm("BX LR");
}

void osSchedulerLaunch(void){
	//Load address of currentPtr into R0
	__asm("LDR R0,=currentPtr");

	//Load R2 from address equals R0, i.e R2 =currentPtr
	__asm("LDR R2,[R0]");

	//Load Cortex-M SP from address equals R2, i.e SP = currentPtr->stackPtr
	__asm("LDR SP,[R2]");

	//Restore R4, R5, R6, R7, R8, R9, R10, R11
	__asm("POP {R4-R11}");

	//Restore R0, R1, R2, R3
	__asm("POP {R0-R3}");

	//Restore R12
	__asm("POP {R12}");

	//Skip LR
	__asm("ADD SP,SP,#4");

	//Create a new start location by popping LR
	__asm("POP {LR}");

	//Skip PSR by adding 4 to SP
	__asm("ADD SP,SP,#4");

	//Enable Global Interrupt
	__asm("CPSIE I");

	//Return from exception
	__asm("BX LR");

}

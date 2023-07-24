#include "led.h"
#include "uart.h"
#include "osKernel.h"

#define QUANTA 10

typedef uint32_t TaskProfiler;

TaskProfiler Task0Profiler,Task1Profiler,Task2Profiler;

void motor_run(void);
void motor_stop(void);
void valve_open(void);
void valve_close(void);

void task0(void){
	while(1){
		Task0Profiler++;
	}
}

void task1(void){
	while(1){
		Task1Profiler++;
	}
}

void task2(void){
	while(1){
		Task2Profiler++;
	}
}

int main(void){
	//Initialize Kernel
	osKernelInit();

	//Add threads
	osKernelAddThreads(&task0, &task1, &task2);

	//Set RoundRobin time quanta
	osKernelLaunch(QUANTA);

}
void motor_run(void){
	printf("motor started.........\n\r");
}
void motor_stop(void){
	printf("motor stopped.........\n\r");
}
void valve_open(void){
	printf("valve opened.........\n\r");
}
void valve_close(void){
	printf("valve closed.........\n\r");
}

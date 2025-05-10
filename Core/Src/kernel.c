/*
 * kernel.c
 *
 *  Created on: Jun 24, 2024
 *      Author: Rahul
 */
#include "kernel.h"

uint32_t* previousStack;
uint32_t* MSP_INIT_VAL;
extern void runFirstThread(void);

thread kernel_thread[MAX_SIZE];
uint32_t currentThread;
uint32_t size_ =0;



uint32_t* allocateNewStack(void) {
	uint32_t* newStack = previousStack - STACK_SIZE;
	if(MSP_INIT_VAL - newStack > 0x4000){
		return NULL;
	}

	previousStack = newStack;
	return newStack;
}

void SVC_Handler_Main( unsigned int *svc_args )
{
unsigned int svc_number;
/*
* Stack contains:
* r0, r1, r2, r3, r12, r14, the return address and xPSR
* First argument (r0) is svc_args[0]
*/
svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
switch( svc_number )
{
  case 1:
	  printf("ONE \r\n");
	  break;
  case 2:
	  printf("TWO \r\n");
	  break;
  case 17: //17 is sort of arbitrarily chosen
    printf("Success!\r\n");
    break;
  case 3:
    __set_PSP(kernel_thread[currentThread].sp);

	runFirstThread();
    break;
  case 4:
	  kernel_thread[currentThread].runtime = kernel_thread[currentThread].timeslice;
  //Pend an interrupt to do the context switch
    _ICSR |= 1<<28;
    __asm("isb");
    break;
  default: /* unknown SVC */
  break;
}
}

bool osCreateThread(void (*thread_function)(void*), void* struct_ptr){
	uint32_t*stackptr = allocateNewStack();
		if(stackptr == NULL){
		return false;
	}
	*(--stackptr) = 1<<24; //A magic number, this is xPSR
	*(--stackptr) = (uint32_t)thread_function; //the function name
	for(uint8_t i = 0; i <14; i ++){
		if(i ==5)
		{
			*(--stackptr) =  struct_ptr;

		}
		else{
			*(--stackptr) = 0xA;
		}
	}

	kernel_thread[size_].sp = stackptr;
	kernel_thread[size_].thread_function =thread_function;
	kernel_thread[size_].timeslice = 5;
	kernel_thread[size_].runtime = 5;

	size_++;
    return true;
}

bool osCreateThreadWithDeadline(void (*thread_function)(void*), uint32_t time, void* struct_ptr){
	uint32_t*stackptr = allocateNewStack();
	if(stackptr == NULL){
		return false;
	}
	*(--stackptr) = 1<<24; //A magic number, this is xPSR
	*(--stackptr) = (uint32_t)thread_function; //the function name
	for(uint8_t i = 0; i <14; i ++){
		if(i ==5)
		{
			*(--stackptr) =  struct_ptr;

		}
		else{
			*(--stackptr) = 0xA;
		}
	}

	kernel_thread[size_].sp = stackptr;
	kernel_thread[size_].thread_function =thread_function;
	kernel_thread[size_].timeslice = time;
	kernel_thread[size_].runtime = time;

	size_++;
    return true;
}

void osKernelInitialize(){
	MSP_INIT_VAL = *(uint32_t**)0x0;
    previousStack = MSP_INIT_VAL;
    //set the priority of PendSV to almost the weakest
    SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
    SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
	for(uint32_t i =0; i < MAX_SIZE; i++){
		kernel_thread[size_].sp = NULL;
	    kernel_thread[size_].thread_function = NULL;
	}
}

void osKernelStart(){
	__asm("SVC #3");
}

void osSched(){
	kernel_thread[currentThread].sp = (uint32_t*)(__get_PSP() - 8*4);
	currentThread = (currentThread + 1)%size_;
	__set_PSP(kernel_thread[currentThread].sp);
	return;
}

void osYield(void){
	__asm("SVC #4");
}




/*
 * kernel.h
 *
 *  Created on: Jun 24, 2024
 *      Author: Rahul
 */
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_
#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV
#define STACK_SIZE 0x400
#define MAX_SIZE 1024
typedef struct k_thread{
uint32_t* sp; //stack pointer
void (*thread_function)(void*); //function pointer
uint32_t timeslice;
uint32_t runtime;
}thread;


void SVC_Handler_Main( unsigned int *svc_args );

uint32_t *allocateNewStack();


bool osCreateThread(void (*thread_function)(void*), void* struct_ptr);


bool osCreateThreadWithDeadline(void (*thread_function)(void*), uint32_t time, void* struct_ptr);


void osKernelInitialize();

void osKernelStart();

void osSched();

void osYield(void);

#endif /* SRC_KERNEL_H_ */

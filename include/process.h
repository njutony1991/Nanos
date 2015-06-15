#ifndef __PROCESS_H__
#define __PROCESS_H__

#define KSTACK_SIZE 4096
#define PCB_NUM 50
#define ANY -1
#include "adt/list.h"

typedef struct Semaphore{
	int token;
	ListHead block;     /**blocking queue**/
}Sem;

typedef struct PCB {
	void *tf;
	uint8_t kstack[KSTACK_SIZE];
	pid_t pid;					// pid
	int in_ready;
	int lock_depth;
	int IF_bit;
	/**Sem for add/get messages**/
	//Sem message_guard; 			// mutual exclusion
 	//Sem empty;         			// can't get message from empty messages
 		
 	Sem message_guard[PCB_NUM+1];
 	//Sem hard_ms_guard;
	Sem any_guard;
	
	/**message queue**/
 	ListHead messages[PCB_NUM+1];
 	ListHead hard_messages;
 	/**PCB linkedlist**/
 	ListHead list;
} PCB;

extern PCB *current;

PCB*
create_kthread(void *fun,int ch,PCB **next);

PCB* 
fetch_pcb(pid_t pid);

void lock(void);
void unlock(void);

void sleep(void);
void wakeup(PCB *p);

void P(Sem*);
void V(Sem*);


void copy_from_kernel(PCB* pcb, void* dest, void* src, int len);
void copy_to_kernel(PCB* pcb, void* dest, void* src, int len);

void strcpy_to_kernel(PCB* pcb, char* dest, char* src);
void strcpy_from_kernel(PCB* pcb, char* dest, char* src);


#endif

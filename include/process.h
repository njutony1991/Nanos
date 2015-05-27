#ifndef __PROCESS_H__
#define __PROCESS_H__

#define KSTACK_SIZE 4096
#define ANY -1
#include "adt/list.h"

typedef int pid_t;
typedef int off_t;
typedef int size_t;

typedef struct Semaphore{
	int token;
	ListHead block;     /**blocking queue**/
}Sem;

typedef struct PCB {
	void *tf;
	uint8_t kstack[KSTACK_SIZE];
	pid_t id;					// pid
	/**Sem for add/get messages**/
	Sem message_guard; 			// mutual exclusion
 	Sem empty;         			// can't get message from empty messages
 	/**message queue**/
 	ListHead messages;
 	/**PCB linkedlist**/
 	ListHead list;
} PCB;

extern PCB *current;

typedef struct Message {
	pid_t src, dest;
	union {
		int type;
		int ret;
	};
	union {
		int i[5];
		struct {
			pid_t req_pid;
			int dev_id;
			void *buf;
			off_t offset;
			size_t len;
		};
	};
	ListHead list;
} Msg;

#endif

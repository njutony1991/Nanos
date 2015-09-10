#ifndef _MSG_H_
#define _MSG_H_
#include "adt/list.h"

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


void send(pid_t dest,Msg *m,int in_driver);
void receive(pid_t src,Msg *m,int in_driver);

#endif
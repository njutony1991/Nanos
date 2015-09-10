#include "kernel.h"

extern pid_t FM;
extern pid_t TIMER;

void fm_test(){
	uint8_t buf[NR_FILE_SIZE];
	while(1){
		Msg m1;
		m1.type=FILE_READ;
		m1.src=current->pid;
		m1.dest=FM;
		m1.req_pid=current->pid;
		m1.dev_id=1;
		m1.buf=buf;
		m1.offset=0;
		m1.len=12;
		send(FM,&m1,1);
		receive(FM,&m1,1);
		int i=0;
		for(;i<m1.len;i++)
			printk("%c",buf[i]);

		Msg m2;
		m2.type = NEW_TIMER;
		m2.src = current->pid;
		m2.dest = TIMER;
		m2.i[0] = 1;
		send(TIMER,&m2,0);
		receive(TIMER,&m2,0);
	}
}
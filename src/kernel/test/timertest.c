#include "kernel.h"
extern pid_t TIMER;
void timer_test(){
	while(1){
		Msg m;
		m.type = NEW_TIMER;
		m.src = current->pid;
		m.dest = TIMER;
		m.i[0] = 5;
		printk("before timer_test send\n");
		send(TIMER,&m,0);
		receive(TIMER,&m,0);
		printk("after timer_test receive\n");
	}
}
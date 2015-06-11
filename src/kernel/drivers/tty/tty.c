#include "kernel.h"
#include "hal.h"
#include "tty.h"

pid_t TTY;

void send_keymsg(void);

void init_console(void);
void init_getty(void);
static void ttyd(void);

void init_tty(void) {
	add_irq_handle(1, send_keymsg);
	PCB *p = create_kthread(ttyd,0,NULL);
	TTY = p->pid;
	printk("tty pid : %d\n",p->pid);
	wakeup(p);
	init_console();
	init_getty();
}

static void
ttyd(void) {
	Msg m;
	//printk("in ttyd\n");
	while (1) {
		receive(ANY, &m,1);
		if (m.src == MSG_HARD_INTR) {
			//printk("\nin ttyd MSG_HARD_INTR m.src : %d ,m.type : %d ,m.dev_id : %d\n",m.src,m.type,m.dev_id);
			switch (m.type) {
				case MSG_TTY_GETKEY:
					readkey();
					break;
				case MSG_TTY_UPDATE:
					update_banner();
					break;
				default: assert(0);
			}
		} else {
			//printk("\nin ttyd m.src : %d ,m.type : %d ,m.dev_id : %d\n",m.src,m.type,m.dev_id);
			switch(m.type) {
				case DEV_READ:
					read_request(&m);
					break;
				case DEV_WRITE:
					if (m.dev_id >= 0 && m.dev_id < NR_TTY) {
						char c;
						int i;
						for (i = 0; i < m.len; i ++) {
							copy_to_kernel(fetch_pcb(m.req_pid), &c, (char*)m.buf + i, 1);
							consl_writec(&ttys[m.dev_id], c);
						}
						consl_sync(&ttys[m.dev_id]);
					}
					else {
						assert(0);
					}
					m.ret = m.len;
					pid_t dest = m.src;
					m.src = current->pid;
					send(dest, &m,1);
					break;
				default: assert(0);
			}
		}
	}
}


#include "kernel.h"
#include "tty.h"

static int tty_idx = 1;

static void
getty(void) {
	char name[] = "tty0";
	char buf[256];
	lock();
	name[3] += (tty_idx ++);
	unlock();
	
	while(1) {
		/* Insert code here to do these:
		 * 1. read key input from ttyd to buf (use dev_read())
		 * 2. convert all small letters in buf into capitcal letters
		 * 3. write the result on screen (use dev_write())
		 */
	    //printk("In getty1\n");
	    size_t l=dev_read(name,current->pid,buf,0,200);
	    int i;
	    for(i=0;i<l;i++)
	    	if(buf[i]>='a'&&buf[i]<='z')
	    		buf[i] = 'A'+(buf[i]-'a');
	    buf[l] = '\n';
	    dev_write(name,current->pid,buf,0,l+1);
	}
}

void
init_getty(void) {
	printk("in init_getty\n");
	int i;
	for(i = 0; i < NR_TTY; i ++) {
		wakeup(create_kthread(getty,0,NULL));
	}
}



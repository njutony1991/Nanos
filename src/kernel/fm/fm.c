#include "kernel.h"
#include "x86/x86.h"
#include "hal.h"
#include "string.h"
#include "ramdisk.h"
#include "fm.h"

pid_t FM;

off_t locate_file(int file_name){
	return file_name * NR_FILE_SIZE;
}

void 
do_read(int file_name, uint8_t *buf, off_t offset, size_t len){
	off_t start = locate_file(file_name)+offset;
	dev_read("ram", current->pid, buf, start, len);
}

static void 
fm_thread(void){
	Msg m;
	while(true){
		receive(ANY,&m,1);
		
		switch(m.type){
			case FILE_READ:
					do_read(m.dev_id,m.buf,m.offset,m.len);	
					Msg ret;
					ret.dest = m.src;
					ret.src = FM;
					ret.buf = m.buf;
					send(m.src,&ret,1);
					break;
			default: assert(0);
		}
	}
}


void init_fm(void){
	PCB *p=create_kthread(fm_thread,0,NULL);
	FM = p->pid;
	wakeup(p);
}
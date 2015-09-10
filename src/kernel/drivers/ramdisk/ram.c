#include "kernel.h"
#include "x86/x86.h"
#include "hal.h"
#include "string.h"
#include "ramdisk.h"

pid_t RAMDISK;
static uint8_t file[NR_MAX_FILE][NR_FILE_SIZE]={
	{0x12,0x34,0x56,0x78},
	{"Hello World\n"},
	{0x7f,0x45,0x4c,0x46}
};
static uint8_t *disk = (void*)file;

static void ramdisk_thread(void);
void read_ram(Msg *m);

void init_ramdisk(){
	PCB *p = create_kthread(ramdisk_thread,0,NULL);
	RAMDISK = p->pid;
	wakeup(p);
	hal_register("ram", RAMDISK, 0);
}

static void 
ramdisk_thread(void){
	Msg m;
	while(true){
		receive(ANY,&m,1);

		switch(m.type){
			case DEV_READ:
				read_ram(&m);
				break;
			default: assert(0);
		}
	}
}

void 
read_ram(Msg *m){
	pid_t dest = m->src;
	int count = m->len;
	uint8_t *buf = m->buf;
	uint8_t *disk_p = disk + m->offset; 
	copy_from_kernel(fetch_pcb(m->req_pid),buf,disk_p,count);
	m->src = RAMDISK;
	m->dest = dest;
	send(dest,m,1);
}
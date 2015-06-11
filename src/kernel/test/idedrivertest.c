#include "kernel.h"

extern pid_t IDE;

void drivertest(){
	char data[513];
	while(1){
		dev_read("hda",current->pid,data,0,512);
		int i;
		for(i=0;i<512;i++)
			printk("%x ",data[i]);
	}
}
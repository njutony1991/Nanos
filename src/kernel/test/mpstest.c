#include "kernel.h"

extern PCB *pa;
extern PCB *pb;
extern PCB *pc;
extern PCB *pd;
extern PCB *pe;



void A () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("a"); 
			send(pe->pid, &m1,0);
			receive(pe->pid, &m2,0);
		}
		x ++;
	}
}

void B () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	//printk("sleep B\n");
	receive(pe->pid, &m2,0);
	while(1) {
		if(x % 10000000 == 0) {
			printk("b"); 
			send(pe->pid, &m1,0);
			receive(pe->pid, &m2,0);
		}
		x ++;
	}
}

void C () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	//printk("sleep C\n");
	receive(pe->pid, &m2,0);
	while(1) {
		if(x % 10000000 == 0) {
			printk("c"); 
			send(pe->pid, &m1,0);
			receive(pe->pid, &m2,0);
		}
		x ++;
	}
}

void D () { 
	Msg m1, m2;
	m1.src = current->pid;
	//printk("sleep D\n");
	receive(pe->pid, &m2,0);
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("d"); 
			send(pe->pid, &m1,0);
			receive(pe->pid, &m2,0);
		}
		x ++;
	}
}
 
void E () {
	Msg m1, m2;
	m2.src = current->pid;
	char c;

	while(1) {
		//printk("sleep E\n");
		receive(ANY, &m1,0);
		if(m1.src == pa->pid) {c = '|'; m2.dest = pb->pid; }
		else if(m1.src == pb->pid) {c = '/'; m2.dest = pc->pid;}
		else if(m1.src == pc->pid) {c = '-'; m2.dest = pd->pid;}
		else if(m1.src == pd->pid) {c = '\\';m2.dest = pa->pid;}
		else assert(0);
 
		printk("\033[s\033[1000;1000H%c\033[u", c);
		send(m2.dest, &m2,0);
	}
 
}
#include "kernel.h"

extern PCB *pa;
extern PCB *pb;
extern PCB *pc;
extern PCB *pd;
extern PCB *pe;

void send(pid_t src, Msg *m);
void receive(pid_t src, Msg *m);

void A () { 
	Msg m1, m2;
	m1.src = current->id;
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("a"); 
			send(pe->id, &m1);
			receive(pe->id, &m2);
		}
		x ++;
	}
}

void B () { 
	Msg m1, m2;
	m1.src = current->id;
	int x = 0;
	receive(pe->id, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("b"); 
			send(pe->id, &m1);
			receive(pe->id, &m2);
		}
		x ++;
	}
}

void C () { 
	Msg m1, m2;
	m1.src = current->id;
	int x = 0;
	receive(pe->id, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("c"); 
			send(pe->id, &m1);
			receive(pe->id, &m2);
		}
		x ++;
	}
}

void D () { 
	Msg m1, m2;
	m1.src = current->id;
	receive(pe->id, &m2);
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("d"); 
			send(pe->id, &m1);
			receive(pe->id, &m2);
		}
		x ++;
	}
}
 
void E () {
	Msg m1, m2;
	m2.src = current->id;
	char c;
	while(1) {
		receive(ANY, &m1);
		if(m1.src == pa->id) {c = '|'; m2.dest = pb->id; }
		else if(m1.src == pb->id) {c = '/'; m2.dest = pc->id;}
		else if(m1.src == pc->id) {c = '-'; m2.dest = pd->id;}
		else if(m1.src == pd->id) {c = '\\';m2.dest = pa->id;}
		else assert(0);
 
		printk("\033[s\033[1000;1000H%c\033[u", c);
		send(m2.dest, &m2);
	}
 
}
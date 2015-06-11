#include "kernel.h"

static int lockcount = 0;

extern ListHead ready;
extern ListHead block;
extern ListHead free;
extern PCB *current;

void lock(){
    cli();
    lockcount++;
}

void unlock(){
    lockcount--;
    if(lockcount<=0){
      lockcount=0;
      sti();
    }
}


/**sleep_on_sem and wakeup_from_sem is call after lock().
   So there is no need to add extra atomic guard in it**/
static void sleep_on_sem(Sem *s){
  if(!list_empty(&ready)){
    current->in_ready = 0;
  	list_del(&(current->list));
  	list_init(&(current->list));
  	list_add_before(&(s->block),&(current->list));
  }
  asm volatile("int $0x80");
}

static void wakeup_from_sem(Sem *s){
  if(!list_empty(&s->block)){
  	PCB *p = list_entry((s->block).next,PCB,list);
    p->in_ready = 1;
   	list_del(&(p->list));
    list_init(&(p->list));
    list_add_before(&ready,&(p->list));
  }
}

/**P&V for Sem**/
void P(Sem *s){
	lock();
	(s->token)--;
	if(s->token<0){
     unlock();
	   sleep_on_sem(s);
	}
  else
    unlock();
}

void V(Sem *s){
	lock();
	(s->token)++;
	if(s->token<=0){
		wakeup_from_sem(s);
	}
	unlock();	
}


void create_sem(Sem *s,int token){
	s->token = token;
	list_init(&(s->block));
}




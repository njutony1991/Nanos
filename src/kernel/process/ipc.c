#include "kernel.h"

extern ListHead ready;
extern ListHead block;
extern ListHead free;
extern PCB *current;

extern PCB *fmtest;

void lock(){
    if(current->lock_depth==0){
        current->IF_bit = (read_eflags() & IF_MASK);
    }
    cli();
    current->lock_depth++;
    NOINTR;
}

void unlock(){
    current->lock_depth--;
    assert(current->lock_depth>=0);
    if(current->lock_depth<=0){
      current->lock_depth=0;
      if(current->IF_bit!=0){
        sti();
        INTR;
      }
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
	   sleep_on_sem(s);
	}
  unlock();
}

void V(Sem *s){
	lock();
  // if(s->pid==fmtest->pid)
  //   printk(" in fmtest %d V,after lock()| ",fmtest->pid);
	(s->token)++;
	if(s->token<=0){
    // if(s->pid==fmtest->pid)
    //   printk(" in fmtest %d V,token: %d before wakeup_from_sem | ",fmtest->pid,s->token);
		wakeup_from_sem(s);
	}
  // if(s->pid==fmtest->pid)
  //   printk(" in fmtest %d V,before unlock()| ",fmtest->pid);
	unlock();
}


void create_sem(Sem *s,int token){
	s->token = token;
	list_init(&(s->block));
}




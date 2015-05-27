#include "kernel.h"

/**
  author:tony
  2015-4-20
**/
  
PCB idle, *current = &idle;

/**extern PCB *pa;
extern PCB *pb;
extern PCB *pc;**/

extern ListHead ready;
extern ListHead block;
extern ListHead free;
extern Sem sw_guard;
void P(Sem*);
void V(Sem*);

void sleep(void){
  list_del(&(current->list));
  list_init(&(current->list));
  list_add_before(&block,&(current->list));
  asm volatile("int $0x80");
}

void wakeup(PCB *p){
   list_del(&(p->list));
   list_init(&(p->list));
   list_add_before(&ready,&(p->list));
}

ListHead *ptr = &ready;

void
schedule(void) {
    if(ptr == &ready)
      ptr = ptr->next;
    current = list_entry(ptr,PCB,list);
    ptr = ptr->next;
    /**printk("in schedule,the current is\n");
    printk("id : %d ,tf : %x\n",current->id,
                                current->tf);**/
    //current = pa;    
}

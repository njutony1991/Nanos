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

void sleep(void){
  lock();
  if(!list_empty(&ready)){
    current->in_ready = 0;
    list_del(&current->list);
    list_init(&(current->list));
    list_add_before(&block,&current->list);
  }
  unlock();
  asm volatile("int $0x80");
}

void wakeup(PCB *p){
   lock();
   if(p->in_ready==0){
    p->in_ready = 1;
    list_del(&(p->list));
    list_init(&(p->list));
    list_add_before(&ready,&(p->list));
   }
   unlock();
}

void print_ready();

ListHead *schedule_ptr = &ready;

void
schedule(void) {
    NOINTR;
    if(!list_empty(&ready)){
        if(schedule_ptr == &ready)
            schedule_ptr = schedule_ptr->next;
        current = list_entry(schedule_ptr,PCB,list);
        schedule_ptr = schedule_ptr->next;
        //printk("current is id : %d ,tf : %x\n",current->pid,
        //                          current->tf);
        //print_ready();
    }else
       current = &idle;
}

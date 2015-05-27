#include "kernel.h"

PCB* get_proc(pid_t);
void sleep(void);
void wakeup(PCB*);
void P(Sem*);
void V(Sem*);

extern PCB *current;

void add_message(PCB* funpcb,Msg *message){
    P(&(funpcb->message_guard));
    list_add_before(&(funpcb->messages),&(message->list));
    V(&(funpcb->empty));
    V(&(funpcb->message_guard));
        //wakeup(funpcb);
}

void copy_msg(Msg *from,Msg *to){
    to->src = from->src;
    to->dest = from->dest;
    to->type = from->type;
    int index=0;
    for(;index<5;index++)
      to->i[index] = from->i[index];
    list_init(&(to->list));
}

void send(pid_t dest,Msg *m){
  PCB *de = get_proc(dest);
  add_message(de,m);
}

void receive(pid_t src,Msg *m){
  P(&(current->empty));
  ListHead *p;
  Msg *target;
  int flag = 0;
  while(flag!=1){
    P(&(current->message_guard));
    if(src==ANY){
      ListHead* first_message = (current->messages).next;
      target = list_entry(first_message,Msg,list);
      list_del(first_message);
      list_init(first_message);
      flag = 1;
    } 
    else{
      list_foreach(p,&(current->messages))
          if(((Msg *)(list_entry(p,Msg,list)))->src==src){
            flag = 1;
            target = (Msg *)(list_entry(p,Msg,list));
            list_del(p);
            list_init(p);
            break;
          }
    }
    V(&(current->message_guard));
    if(flag!=1)
      sleep();
  }
  copy_msg(target,m);
}

#include "kernel.h"

Msg message_pool[MSG_NUM];

ListHead msg_free,msg_busy;


void init_msg(){
  list_init(&msg_free);
  list_init(&msg_busy);
  int i;
  for(i=0;i<MSG_NUM;i++)
    list_add_before(&msg_free,&message_pool[i].list);
}

Msg *fetch_msg(){
  lock();
  if(list_empty(&msg_free))
    panic("no more free messages!");
  Msg* result = list_entry(msg_free.next,Msg,list);
  list_del(&result->list);
  list_init(&result->list);
  unlock();
  return result;
}

void free_msg(Msg *tofree){
  lock();
  list_init(&tofree->list);
  list_add_before(&msg_free,&tofree->list);
  unlock();
}
extern PCB *current;

void add_message(PCB* funpcb,Msg *message){
    lock();
    list_add_before(&funpcb->messages,&message->list);
    V(&funpcb->empty);
    wakeup(funpcb);  
    unlock();
}


void copy_msg(Msg *from,Msg *to,int in_driver){
    to->src = from->src;
    to->dest = from->dest;
    to->type = from->type;
    if(in_driver==0){
      int index=0;
      for(;index<5;index++)
        to->i[index] = from->i[index];
    }
    else{
      to->req_pid = from->req_pid;
      to->dev_id = from->dev_id;
      to->buf = from->buf;
      to->offset = from->offset;
      to->len = from->len;
    }
    list_init(&to->list);
}

void send(pid_t dest,Msg *m,int in_driver){
  Msg *tocopy = fetch_msg();
  copy_msg(m,tocopy,in_driver);

  PCB *dest_pcb = fetch_pcb(dest);
  if(dest_pcb==NULL)
    panic("NULL PCB %d\n",dest);
  add_message(dest_pcb,tocopy);
}

void receive(pid_t src,Msg *m,int in_driver){
  ListHead *p;
  Msg *target;
  int flag = 0;

  while(flag!=1){
    P(&current->empty);
    P(&current->message_guard);
    if(src==ANY){
      assert(!list_empty(&current->messages));
      ListHead* first_message = (current->messages).next;
      target = (Msg *)(list_entry(first_message,Msg,list));
      list_del(first_message);
      list_init(first_message);
      flag = 1;
    } 
    else
    {
      assert(!list_empty(&current->messages));
      list_foreach(p,&(current->messages))
          if(((Msg *)(list_entry(p,Msg,list)))->src==src){
            flag = 1;
            target = (Msg *)(list_entry(p,Msg,list));
            list_del(p);
            list_init(p);
            break;
          }
    }
    V(&current->message_guard);
    if(flag!=1){
      V(&current->empty);
      sleep();
    }
  }
  if(target!=NULL){
    copy_msg(target,m,in_driver);
    free_msg(target);
  }
}

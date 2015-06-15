#include "kernel.h"

void create_sem(Sem *,int);

Msg message_pool[MSG_NUM];

ListHead msg_free;

Sem message_guard,message_mutex;

void init_msg(){
  list_init(&msg_free);
  int i;
  for(i=0;i<MSG_NUM;i++)
    list_add_before(&msg_free,&message_pool[i].list);
  create_sem(&message_guard,MSG_NUM);
  create_sem(&message_mutex,1);
}

Msg *fetch_msg(){
    
  P(&message_guard);
  P(&message_mutex);
  //NOINTR;
  //printk("current pid: %d\n",current->pid);
  if(list_empty(&msg_free))
    panic("no more free messages!");
  Msg* result = list_entry(msg_free.next,Msg,list);
  list_del(&result->list);
  list_init(&result->list);
  V(&message_mutex);
  return result;
}

void free_msg(Msg *tofree){
  P(&message_mutex);
  list_init(&tofree->list);
  list_add_before(&msg_free,&tofree->list);
  V(&message_mutex);
  V(&message_guard);
}

extern PCB *current;

void add_message(PCB* funpcb,Msg *message){
    if(message->src>=0&&message->src<=PCB_NUM){
      list_add_before(&funpcb->messages[message->src],&message->list);
      V(&funpcb->message_guard[message->src]);
    }else{
      list_add_before(&funpcb->hard_messages,&message->list);
    }
    V(&funpcb->any_guard);  
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
  lock();
  Msg *tocopy = fetch_msg();
  copy_msg(m,tocopy,in_driver);

  PCB *dest_pcb = fetch_pcb(dest);
  if(dest_pcb==NULL)
    panic("NULL PCB %d\n",dest);
  add_message(dest_pcb,tocopy);
  unlock();
}

void receive(pid_t src,Msg *m,int in_driver){
    Msg *target=NULL;
    P(&current->any_guard);
    if(src==ANY){
      lock();
      if(!list_empty(&current->hard_messages)){
        ListHead* first_message = (current->hard_messages).next;
        target = (Msg *)(list_entry(first_message,Msg,list));
        list_del(first_message);
        list_init(first_message);
      }else{
        int i;
        for(i=0;i<=PCB_NUM;i++)
        {
          if((current->message_guard[i]).token>0){
            P(&current->message_guard[i]);
            assert(!list_empty(&current->messages[i]));
            ListHead* first_message = (current->messages[i]).next;
            target = (Msg *)(list_entry(first_message,Msg,list));
            list_del(first_message);
            list_init(first_message);
            break;
          }
        }
      }
      unlock();
    } 
    else
    {
      P(&current->message_guard[src]);
      lock();
      assert(!list_empty(&current->messages[src]));
      ListHead *first_message = (current->messages[src]).next;
      target = (Msg*)(list_entry(first_message,Msg,list));
      list_del(first_message);
      list_init(first_message);
      unlock();
    }

    if(target!=NULL){
      copy_msg(target,m,in_driver);
      free_msg(target);
    }
}

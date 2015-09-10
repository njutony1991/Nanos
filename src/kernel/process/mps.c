#include "kernel.h"

void create_sem(Sem *,int);

extern PCB *current;
extern pid_t RAMDISK;

void init_msg(PCB *p){
  list_init(&p->msg_free);
  int i;
  for(i=0;i<MSG_NUM;i++)
    list_add_before(&p->msg_free,&p->msg_pool[i].list);
  create_sem(&p->pool_empty,MSG_NUM);
  create_sem(&p->pool_mutex,1);
}


Msg *fetch_msg(PCB *p){
  do{
      P(&p->pool_empty);
  }while(list_empty(&p->msg_free));
  P(&p->pool_mutex);
  NOINTR;

  if(list_empty(&p->msg_free))
    panic("no more free messages!");
  Msg* result = list_entry((p->msg_free).prev,Msg,list);

  list_del(&result->list);

  list_init(&result->list);

  V(&p->pool_mutex);

  return result;
}

void free_msg(Msg *tofree){
  P(&current->pool_mutex);
  list_init(&tofree->list);
  list_add_before(&current->msg_free,&tofree->list);
  V(&current->pool_mutex);
  V(&current->pool_empty);
}



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
  PCB *dest_pcb = fetch_pcb(dest);
  if(dest_pcb==NULL)
    panic("NULL PCB %d\n",dest);
 
  Msg *tocopy = fetch_msg(dest_pcb);

  copy_msg(m,tocopy,in_driver);

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

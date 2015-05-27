#include "kernel.h"

/**
  author:tony
  2015-4-20
**/

ListHead ready,block,free;
/**thread schedule**/
void schedule(void);
void sleep(void);
void wakeup(PCB *p);

/**ipc**/ 
void test_setup();
void create_sem(Sem*,int);

/**mpstest**/
void A();
void B();
void C();
void D();
void E();

/**pid index,free PCB buffer**/
int i=0,id=0;
PCB procs[20];
PCB *pa,*pb,*pc,*pd,*pe;
//Sem sw_guard;   // mutual exclusion for sleep&wakeup

PCB* get_proc(pid_t pid){
    assert(pid <= id);
    return &procs[pid];
}

PCB*
create_kthread(void *fun,int ch,PCB **next) {
	PCB *funpcb = & procs[i++];
	TrapFrame *tf = (TrapFrame *)(funpcb->kstack + KSTACK_SIZE-12)-1;
	tf->eflags = 0x202;
	tf->cs = SELECTOR_KERNEL(SEG_KERNEL_CODE);
	tf->ds = SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->es = SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->fs = SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->gs = SELECTOR_KERNEL(SEG_KERNEL_DATA);
    //tf->ss = SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->error_code = 0;
	tf->eip = (uint32_t)fun;
	//tf->esp = (uint32_t)tf;   the esp is not needed in kernal thread
    //tf->ebp = (uint32_t)(tf+1)+4;
    if(ch != 0){   
        int *para1 = (int *)((uint32_t)(tf+1)-4);   //overwrite the esp and ss
        *para1 = ch;
    }
    if(next != NULL){
        PCB ***para2 = (PCB ***)((uint32_t)(tf+1));
        *para2 = next;
    }

	funpcb->tf = tf;
	funpcb->id = id++;
    create_sem(&(funpcb->message_guard),1);
    create_sem(&(funpcb->empty),0);
    list_init(&(funpcb->messages));
	return funpcb;
}


void
init_proc() {
	list_init(&ready);
	list_init(&block);
	list_init(&free);
    //create_sem(&sw_guard,1);
    pa = create_kthread(A,0,NULL);
    pb = create_kthread(B,0,NULL);
    pc = create_kthread(C,0,NULL);
    pd = create_kthread(D,0,NULL);
    pe = create_kthread(E,0,NULL);
    wakeup(pa);
    wakeup(pb);
    wakeup(pc);
    wakeup(pd);
    wakeup(pe);
	/**pa = create_kthread(print_ch,'a',&pb);
	printk("pa : %d ,pa->tf : %x\n",pa->id,pa->tf);
	list_add_before(&ready,&(pa->list));

    printk("ready:\n");
    ListHead *p1;
    list_foreach(p1,&ready)
        printk("id : %d ,tf : %x\n",((PCB *)(list_entry(p1,PCB,list)))->id,
                                    ((PCB *)(list_entry(p1,PCB,list)))->tf);
    printk("\n-----------\n");

    pb = create_kthread(print_ch,'b',&pc);
	printk("pb : %d ,pb->tf : %x\n",pb->id,pb->tf);
	list_add_before(&block,&(pb->list));
	pc = create_kthread(print_ch,'c',&pd);
	printk("pc : %d ,pc->tf : %x\n",pc->id,pc->tf);
	list_add_before(&block,&(pc->list));
    pd = create_kthread(print_ch,'d',&pa);
    printk("pd : %d ,pd->tf : %x\n",pd->id,pd->tf);
    list_add_before(&block,&(pd->list));
    
    printk("block:\n");
    ListHead *p2;
    list_foreach(p2,&block)
        printk("id : %d ,tf : %x\n",((PCB *)(list_entry(p2,PCB,list)))->id,
                                    ((PCB *)(list_entry(p2,PCB,list)))->tf);
    printk("\n---------\n");**/

    //test_setup();
}



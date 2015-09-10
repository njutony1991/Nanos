#include "kernel.h"
#include "x86/x86.h"
#include "hal.h"
#include "time.h"
#include "string.h"

#define PORT_TIME 0x40
#define PORT_RTC  0x70
#define FREQ_8253 1193182
#define EVENT_NUM 1000

pid_t TIMER;
static long jiffy = 0;
static Time rt;

static void update_jiffy(void);
static void init_i8253(void);
static void init_rt(void);
static void timer_driver_thread(void);

static Timer_Event event_pool[EVENT_NUM];

static ListHead te_free;
static ListHead te_busy;
static Sem te_guard;
static Sem te_free_mutex;
static Sem te_busy_mutex;

void create_sem(Sem *,int);

void init_free_te(){
	list_init(&te_free);
	list_init(&te_busy);
	
	int i=0;
	for(;i<EVENT_NUM;i++)
		list_add_before(&te_free,&event_pool[i].list);

	create_sem(&te_guard,EVENT_NUM);
	create_sem(&te_free_mutex,1);
	create_sem(&te_busy_mutex,1);
}

Timer_Event* fetch_te(){
	do{
		P(&te_guard);
	}while(list_empty(&te_free));

	P(&te_free_mutex);
	Timer_Event *result = list_entry(te_free.next,Timer_Event,list);
	list_del(&result->list);
	list_init(&result->list);
	V(&te_free_mutex);
	return result;
}

void free_te(Timer_Event *te){
	P(&te_busy_mutex);
	list_del(&te->list);
	list_init(&te->list);
	V(&te_busy_mutex);
	P(&te_free_mutex);
	list_add_before(&te_free,&te->list);
	V(&te_free_mutex);
	V(&te_guard);
}

void add_busy(Timer_Event *te){
	P(&te_busy_mutex);
	if(list_empty(&te_busy)){	
		list_del(&te->list);
		list_init(&te->list);
		list_add_after(&te_busy,&te->list);
	}else{
		ListHead *ptr = te_busy.next;
		long te_sec = te->second;
		Timer_Event *tmp = list_entry(ptr,Timer_Event,list);
		while(ptr!=&te_busy && tmp->second<=te_sec){
			te_sec = te_sec-tmp->second;
			ptr = ptr->next;
			tmp = list_entry(ptr,Timer_Event,list);
		}
		te->second = te_sec;
		list_add_before(ptr,&te->list);
	}
	V(&te_busy_mutex);
}

void init_timer(void) {
	init_i8253();
	init_rt();
	add_irq_handle(0, update_jiffy);
	PCB *p = create_kthread(timer_driver_thread,0,NULL);
	printk("\ntimer pid: %d\n",p->pid);
	TIMER = p->pid;
	wakeup(p);
	hal_register("timer", TIMER, 0);

	init_free_te();
}

static void
timer_driver_thread(void) {
	static Msg m;
	Timer_Event	*te = NULL;
	while (true) {
		receive(ANY, &m,1);
		//printk("in timer_driver_thread\n");
		switch (m.type) {
			case NEW_TIMER:
					if(m.i[0]==0){
						Msg reply;
						reply.src = TIMER;
						reply.dest = m.src;
						send(m.src,&reply,1);
					}else{
						te=fetch_te();
						te->pid = m.src;
						te->second = m.i[0];
						add_busy(te);
					}
					break;
			default: assert(0);
		}
	}
}

void process_tes(){
	//printk("in process_tes\n");
	Msg msg;
	if(!list_empty(&te_busy)){
		ListHead *p = te_busy.next;
		Timer_Event *te = list_entry(p,Timer_Event,list);
		te->second--;
		if(te->second==0){
			while(p!=&te_busy && te->second==0){
				p = p->next;
				list_del(&te->list);
				list_init(&te->list);
				msg.src = TIMER;
				msg.dest = te->pid;
				send(te->pid,&msg,1);
				free_te(te);
				if(p!=&te_busy)
					te = list_entry(p,Timer_Event,list);
			}
		}
	}
	//printk("end process_tes\n");
}

long
get_jiffy() {
	return jiffy;
}

static int
md(int year, int month) {
	bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
	static int tab[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return tab[month] + (leap && month == 2);
}

static void
update_jiffy(void) {
	jiffy ++;
	if (jiffy % HZ == 0) {
		rt.second ++;
		if (rt.second >= 60) { rt.second = 0; rt.minute ++; }
		if (rt.minute >= 60) { rt.minute = 0; rt.hour ++; }
		if (rt.hour >= 24)   { rt.hour = 0;   rt.day ++;}
		if (rt.day >= md(rt.year, rt.month)) { rt.day = 1; rt.month ++; } 
		if (rt.month >= 13)  { rt.month = 1;  rt.year ++; }
		process_tes();
	}
}

static void
init_i8253(void) {
	int count = FREQ_8253 / HZ;
	assert(count < 65536);
	out_byte(PORT_TIME + 3, 0x34);
	out_byte(PORT_TIME, count & 0xff);
	out_byte(PORT_TIME, count >> 8);	
}

int get_start_time(unsigned short reg){
	/** to do
	**/
	//unsigned int time;
	//asm volatile("movb %0, %%al" : :"r"(reg));
	//asm volatile("outb %%al, %0" : :);
	//asm	volatile("inb (unsigned short)$0x71, %al");
	//asm volatile("movb %%al, %0" : "=r"(time));
	return 0;
}

static void
init_rt(void) {
	memset(&rt, 0, sizeof(Time));
	/* Optional: Insert code here to initialize current time correctly */

	rt.year = get_start_time(0x09);
	rt.month = get_start_time(0x08);
	rt.day = get_start_time(0x07);
	rt.hour = get_start_time(0x04);
	rt.minute = get_start_time(0x02);
	rt.second = get_start_time(0x00);
}

void 
get_time(Time *tm) {
	memcpy(tm, &rt, sizeof(Time));
}

#ifndef __TIME_H__
#define __TIME_H__

#define HZ        100
#define NEW_TIMER  1
typedef struct Time {
	int year, month, day;
	int hour, minute, second;
} Time;

typedef struct Timer_Event{
	long second;
	pid_t pid;
	ListHead list;
}Timer_Event;

inline long get_jiffy();

void get_time(Time *tm);

#endif

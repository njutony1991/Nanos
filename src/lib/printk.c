#include "common.h"
/**
	author:tony
	2015-4-5
**/

void printdec(void (*printer)(char),int32_t num){
	int32_t a[16];
	int32_t i = 0,j,digit,upper;
	int32_t flag1 = 0,flag2 = 0;
	//- Integer.Min will overflow
	if(num == 0x80000000){
		num = num+1;
		flag1 = 1;
	}

	// reverse to negetive number to positive number
	if(num>=0)
		upper = num;
	else{
		flag2 = 1;
		upper = -num;
	}

	do{
		digit = upper%10;
		upper = upper/10;
		a[i++] = digit;
	}while(upper!=0);

	if(flag2==1)
		printer('-');
	
	for(j = i-1;j>=1;j--)
		printer(a[j]+'0');

	//distinguish the Integer.Min from other numbers
	if(flag1==0)
		printer(a[0]+'0');
	else
		printer((a[0]+1)+'0');
}


const char dic[6] ={'a','b','c','d','e','f'};
void printhex(void (*printer)(char),uint32_t num){
	char tmp[16];
	int32_t i = 0,j;
	uint32_t digit,upper;
	upper = num;
	do{
		digit = upper%16;
		upper = upper/16;
		if(digit>=10)
			tmp[i++] = dic[digit-10];
		else
			tmp[i++] = '0'+digit;
	}while(upper!=0);
	for(j=i-1;j>=0;j--)
		printer(tmp[j]);
}

/* implement this function to support printk */
void vfprintf(void (*printer)(char), const char *ctl, void **args) {
	const char *p = ctl;
	void **argp = args;
	char* charp;

	for(;*p != '\0';p++){
		if(*p != '%'){
			printer(*p);
			continue;
		}
		switch(*(++p)){
			case 'd':	
				printdec(printer,(int32_t)(*argp));
				argp++;
				break;
			case 'x':
				printhex(printer,(uint32_t)(*argp));
				argp++;
				break;
			case 's':
				charp = (char*)(*argp);
				for(;*charp!='\0';charp++)
					printer(*charp);
				argp++;
				break;
			case 'c':
				charp = (char*)argp;
				printer(*charp);
				argp++;
				break;
		}
	}
}

extern void serial_printc(char);

/* __attribute__((__noinline__))  here is to disable inlining for this function to avoid some optimization problems for gcc 4.7 */
void __attribute__((__noinline__)) 
printk(const char *ctl, ...) {
	void **args = (void **)&ctl + 1;
	vfprintf(serial_printc, ctl, args);
}

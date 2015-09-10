#include "kernel.h"
#include "x86/x86.h"
#include "memory.h"
#include "mm.h"

pid_t MM;

void init_mm(void){

}

static void 
mm_thread(){
	Msg m;
	while(true){
		recevie(ANY,&m,1);
		switch(m.type){
			case NEW_PAGE:
				
				break;
			default:
				assert(0);	
		}
	}
}
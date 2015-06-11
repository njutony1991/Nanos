#include "kernel.h"


/**for test**/
void print_ch(int ch,PCB **next){
    int x = 0;
    while(1){
        if(x % 100000 == 0){
            printk("%c",ch);
            if(*next!=0)
                wakeup(*next);
            sleep();
        }
        x++;
    }
}

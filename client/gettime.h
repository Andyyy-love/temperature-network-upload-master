#ifndef _GETTIME_
#define _GETTIME_

#include <time.h>
void get_localtime(char *local_time);  //local_time数组大小要大于20
int interval_timer(time_t *last_time, int interval);
int opt_localtime(char *local_time, size_t size);
#endif 


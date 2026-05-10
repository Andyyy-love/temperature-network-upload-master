#ifndef _TRANSFERDATA_
#define _TRANSFERDATA_
#include "gettime.h"
#include "ds18b20.h"


typedef struct 
{
    char id[32];
    char time[32];
    float temperature;
}package;

int makepackage(package *pack);

#endif
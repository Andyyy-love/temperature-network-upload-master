/********************************************************************************
 *      Copyright:  (C) 2025 YANG Studio
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(03/09/25)
 *         Author:  hjh
 *      ChangeLog:  1, Release initial version on "03/09/25 10:22:14"
 *                 
 ********************************************************************************/


#ifndef _DS18B20_H
#define _DS18B20_H

#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<errno.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>

int get_devid(char *id, int size);
int get_temperature(float *temp);


#endif  
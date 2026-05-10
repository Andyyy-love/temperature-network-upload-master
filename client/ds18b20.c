/*********************************************************************************
 *      Copyright:  (C) 2025 YANG Studio
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file `:
 *                 
 *        Version:  1.0.0(03/09/25)
 *         Author:  hjh 
 *      ChangeLog:  1, Release initial version on "03/09/25 10:21:56"
 *                 
 ********************************************************************************/

#include "ds18b20.h"



/***********************************************************************************
 * @brief 生成设备 ID,然后定义一个缓冲数组，把ID放在数组里
 *
 * @param id 存储生成的设备 ID 的字符数组
 * @param size 数组大小
 * @return 0 成功, -1 失败
 *
 * 此函数生成一个格式为 "RPI@xxxx" 的设备 ID。设备 ID 前缀为 "RPI@"，后跟一个四位数的序号（此处是硬编码为 520）。
 * 如果输入的 ID 或数组大小无效，则返回 -1。
 **********************************************************************************/
int get_devid(char *id, int size)     
{
    int sn=520;
	if (!id || size<=0)
	{
		printf("Invalid parameters\n");
		return -1;
	}
	memset(id, 0, size);
	snprintf(id, size, "RPI@%04d", sn);

	return 0;
}

/***********************************************************************************
 * @brief 获取 DS18B20 温度传感器的温度
 *
 * @param temp 存储温度值的浮动变量
 * @return 0 成功, -1 失败
 *
 * 此函数用于从 DS18B20 温度传感器读取温度。首先它通过扫描 `/sys/bus/w1/devices/` 目录来找到 DS18B20 芯片的设备路径，然后读取该文件以获取温度数据。
 * 如果读取温度失败，返回 -1；成功读取温度并返回。
 **********************************************************************************/
int get_temperature(float *temp)
{
	int               fd = -1;
	char              buf[128];
	char              *ptr = NULL;
	DIR               *dirp = NULL;
	struct dirent     *direntp = NULL;
	char              w1_path[64]="/sys/bus/w1/devices/";
	char              chip_sn[32];
	// char              ds18b20_path[64];
	int               found = 0;

	dirp = opendir(w1_path);
	if(!dirp)
	{
		printf("open folder %s failure:%s\n",w1_path,strerror(errno));
		return -1;
	}

	while(NULL !=(direntp=readdir(dirp)))
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
			found = 1;

		}
		//printf("filename:%s\n",direntp->d_name);
	
	}

	closedir(dirp);

	if(!found)
	{
		printf("Can not find ds18b20 chipset\n");
		return -1;
	}

	strncat(w1_path,chip_sn,sizeof(w1_path)-strlen(w1_path));
	strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));
	//printf("w1_path:%s\n",w1_path);

	fd = open(w1_path,O_RDONLY);
	
	if(fd<0)
	{
		printf("open file failure:%s\n",strerror(errno));
		return -1;
	}
	memset(buf,0,sizeof(buf));

	if(read(fd,buf,sizeof(buf))<0)
	{
		printf("read data from fd=%d failure:%s\n",fd,strerror(errno));
		return -1;
	}
	//printf("buf:%s\n",buf);
	ptr=strstr(buf,"t=");

	if(!ptr)
	{
		printf("Can not find t= string\n");

		close(fd);
		return -1;
	}
	ptr +=2;
	*temp=atof(ptr)/1000;
	printf("temprature:%f\n",*temp);

	close(fd);
	return 0;
}



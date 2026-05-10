#include "gettime.h"

/*  
获取当前时间
*/
void get_localtime(char *local_time) 
{
	time_t 		now;
	struct tm  *Time;

	time(&now);
	Time=localtime(&now);   //将时间戳转换为本地时间的结构体
	strftime(local_time, 32, "%Y-%m-%d %H:%M:%S", Time);
}

/******************************************************************
	获取当前时间，需带数组大小参数
*******************************************************************/
int opt_localtime(char *local_time, size_t size)
{
    if (!local_time || size < 20) // 检查参数有效性并确保缓冲区足够大
    {
        return -1; // 返回错误代码
    }

    time_t now;
    struct tm *Time;

    time(&now);
    Time = localtime(&now);
    if (Time == NULL)
    {
        return -2; // 时间转换失败
    }

    // 格式化时间并返回结果
    if (strftime(local_time, size, "%Y-%m-%d %H:%M:%S", Time) == 0)
    {
        return -3; // 格式化失败
    }

    return 0; // 成功
}

/***********************************************************************************
 * @brief 检查时间间隔是否已到
 *
 * @param last_time 上次执行时间的时间戳
 * @param interval 检查的时间间隔，单位为秒
 * @return 1 表示时间间隔已到，0 表示时间间隔未到
 *
 * 此函数检查从上次执行到现在是否已经超过了指定的时间间隔。如果时间间隔已到，
 * 函数将更新 `last_time` 为当前时间，并返回 1；如果时间间隔未到，则返回 0。
 ***********************************************************************************/
int interval_timer(time_t *last_time, int interval)
{
	time_t 		now;

	time(&now);

	if (now >= *last_time+interval)
	{
		*last_time = now;
		return 1;
	}

	return 0;
}
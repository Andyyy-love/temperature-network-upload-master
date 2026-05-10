#include "transferdata.h"


/*函数对序列号、采样时间、采集温度值封装成一个结构体,存在pack中 */
int makepackage(package *pack)
{
    int result;  //接收opt_localtime()函数返回值
    if (!pack)
    {
        return -1;
    }
    memset(pack, 0, sizeof(*pack));
    if (get_devid(pack->id, sizeof(pack->id))< 0 ) //结构体成员地址,这里是数组，传递数组首地址
    {
        printf("get_device() error\n");
        return -2;
    }
    result = opt_localtime(pack->time, sizeof(pack->time));

    if (result == 0)
    {
        printf("Current local time: %s\n", pack->time);
    }
    else
    {
        printf("sizeof(pack->time)=: %d\n", sizeof(pack->time));
        printf("Error occurred: %d\n", result);
    }
    if (get_temperature(&pack->temperature) < 0)
    {
        printf("get_temperature() error\n");
        return -3;
    }

    return 0;
}

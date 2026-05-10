#include "packet.h"

#include <stdio.h>
#include <string.h>

#include "ds18b20.h"
#include "gettime.h"

int packet_sample(packet_t *pack)
{
    if (!pack)
    {
        return -1;
    }

    memset(pack, 0, sizeof(*pack));

    if (get_devid(pack->id, sizeof(pack->id)) < 0)
    {
        return -2;
    }

    if (opt_localtime(pack->time, sizeof(pack->time)) < 0)
    {
        return -3;
    }

    if (get_temperature(&pack->temperature) < 0)
    {
        return -4;
    }

    return 0;
}

int packet_to_json(const packet_t *pack, char *buf, size_t size)
{
    int len;

    if (!pack || !buf || size == 0)
    {
        return -1;
    }

    len = snprintf(buf, size,
                   "{\"id\":\"%s\",\"time\":\"%s\",\"temperature\":%.2f}\n",
                   pack->id, pack->time, pack->temperature);

    if (len < 0 || (size_t)len >= size)
    {
        return -2;
    }

    return len;
}

#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>

typedef struct
{
    char id[32];
    char time[32];
    float temperature;
} packet_t;

int packet_sample(packet_t *pack);
int packet_to_json(const packet_t *pack, char *buf, size_t size);

#endif

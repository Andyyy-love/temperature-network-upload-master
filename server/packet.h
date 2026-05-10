#ifndef PACKET_H
#define PACKET_H

typedef struct
{
    char id[32];
    char time[32];
    float temperature;
} packet_t;

int packet_from_json(packet_t *pack, const char *json);

#endif

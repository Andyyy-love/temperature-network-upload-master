#ifndef CJSON_H
#define CJSON_H

typedef struct cJSON
{
    char valuestring[64];
    double valuedouble;
} cJSON;

cJSON *cJSON_Parse(const char *value);
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);
void cJSON_Delete(cJSON *item);

#endif

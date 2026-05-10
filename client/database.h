#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "packet.h"

#define TA_NAME "temperature"

int database_open(sqlite3 **db, const char *filename);
void database_close(sqlite3 *db);
int database_create(sqlite3 *db);
int database_insert(sqlite3 *db, const packet_t *pack);
int database_count(sqlite3 *db);
int database_pop(sqlite3 *db, packet_t *pack);

#endif

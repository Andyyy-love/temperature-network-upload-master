#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *CREATE_SQL =
    "create table if not exists " TA_NAME "("
    "COUNT integer primary key autoincrement,"
    "ID text not null,"
    "TIME datetime not null,"
    "TEMP real not null"
    ")";

int database_open(sqlite3 **db, const char *filename)
{
    if (!db || !filename)
    {
        return -1;
    }

    if (sqlite3_open(filename, db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
        return -2;
    }

    sqlite3_exec(*db, "pragma synchronous = OFF;", NULL, NULL, NULL);
    sqlite3_exec(*db, "pragma auto_vacuum = 2;", NULL, NULL, NULL);
    return database_create(*db);
}

void database_close(sqlite3 *db)
{
    if (db)
    {
        sqlite3_close(db);
    }
}

int database_create(sqlite3 *db)
{
    char *errmsg = NULL;

    if (!db)
    {
        return -1;
    }

    if (sqlite3_exec(db, CREATE_SQL, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "create table failed: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -2;
    }

    return 0;
}

int database_insert(sqlite3 *db, const packet_t *pack)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql = "insert into " TA_NAME "(ID,TIME,TEMP) values(?,?,?);";

    if (!db || !pack)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return -2;
    }

    sqlite3_bind_text(stmt, 1, pack->id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pack->time, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, pack->temperature);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return -3;
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int count_cb(void *data, int argc, char **argv, char **azColName)
{
    int *count = (int *)data;
    (void)argc;
    (void)azColName;

    *count = argv[0] ? atoi(argv[0]) : 0;
    return 0;
}

int database_count(sqlite3 *db)
{
    int count = 0;
    char *errmsg = NULL;

    if (!db)
    {
        return -1;
    }

    if (sqlite3_exec(db, "select count(*) from " TA_NAME ";", count_cb, &count, &errmsg) != SQLITE_OK)
    {
        sqlite3_free(errmsg);
        return -2;
    }

    return count;
}

int database_pop(sqlite3 *db, packet_t *pack)
{
    sqlite3_stmt *stmt = NULL;
    const char *select_sql = "select ID,TIME,TEMP from " TA_NAME " order by COUNT limit 1;";
    const char *delete_sql = "delete from " TA_NAME " where COUNT = (select COUNT from " TA_NAME " order by COUNT limit 1);";
    char *errmsg = NULL;

    if (!db || !pack)
    {
        return -1;
    }

    memset(pack, 0, sizeof(*pack));

    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return -2;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        return -3;
    }

    snprintf(pack->id, sizeof(pack->id), "%s", (const char *)sqlite3_column_text(stmt, 0));
    snprintf(pack->time, sizeof(pack->time), "%s", (const char *)sqlite3_column_text(stmt, 1));
    pack->temperature = (float)sqlite3_column_double(stmt, 2);
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, delete_sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        sqlite3_free(errmsg);
        return -4;
    }

    return 0;
}

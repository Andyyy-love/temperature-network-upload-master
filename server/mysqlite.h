/********************************************************************************
 *      Copyright:  (C) 2024 diancitie
 *                  All rights reserved.
 *
 *       Filename:  database.h
 *    Description:  This file statement of the database.c
 *
 *        Version:  1.0.0(20/02/24)
 *         Author:  Zhang long <1318085470@qq.com>
 *      ChangeLog:  1, Release initial version on "24/11/28 9:43:45"
 *                 
 ********************************************************************************/

#ifndef  _DATABASE_H_
#define  _DATABASE_H_

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include "transferdata.h"
#define TA_NAME		"temperature"

#define TA_CREATE   "create table if not exists " TA_NAME \
					"(" \
						"COUNT integer  primary key autoincrement," \
						"ID    text     not null," \
						"TIME  datetime not null," \
						"TEMP  REAL     not null" \
					")"

typedef int (*db_callback_func)(void *data, int argc, char **argv, char **azColName);
#define SQL_COMMAND_LEN        256
extern void db_open(sqlite3 **db, char *filename);
extern void db_close(sqlite3 *db, char *filename);
extern void db_create(sqlite3 *db, char *sql);
extern void db_insert(sqlite3 *db, char *sql);
extern void db_update(sqlite3 *db, char *sql);
extern void db_delete(sqlite3 *db, char *sql);
extern void db_select(sqlite3 *db, char *sql, db_callback_func callback, void *data);
extern int write_to_db(sqlite3 *db, package *pack);

extern int db_count(sqlite3 *db);
extern int read_from_db(sqlite3 *db, package *pack);
extern int database_pop_packet(void *pack, int size, int *bytes);
extern int database_del_packet(void);




#endif   /* ----- #ifndef _DATABASE_H_  ----- */

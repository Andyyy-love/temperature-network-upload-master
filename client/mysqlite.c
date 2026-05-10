/*********************************************************************************
 *       Filename:  database.c
 *    Description:  This file implement the sqlite3 API
 *
 *        Version:  1.0.0(2024/11/28 09:54:08)
 *         Author:  Zhang Long <1318085470@qq.com>
 *      ChangeLog:  1, Release initial version on "2024/11/28 09:54:08"
 ********************************************************************************/
#include "mysqlite.h"
#include "transferdata.h"


static char 	*db_ErrMsg = 0;
static sqlite3  *s_clidb = NULL;
/**
 * @brief 打开数据库连接
 *
 * @param db 数据库连接指针（指向 `sqlite3*`）
 * @param filename 数据库文件名（字符串）
 * @return void 无返回值
 *
 * 此函数尝试打开指定的 SQLite 数据库文件。如果文件无法打开，将输出错误信息并终止程序。
 * 打开数据库后，它还执行了一些 SQLite 特定的配置，如禁用同步（提高性能）和启用自动清理（`auto_vacuum`）。
 */
void db_open(sqlite3 **db, char *filename)
{
	if (sqlite3_open(filename, db) != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
		exit(0);	
	}
	
    sqlite3_exec(*db, "pragma synchronous = OFF; ", NULL, NULL, NULL);
    sqlite3_exec(*db, "pragma auto_vacuum = 2 ; ", NULL, NULL, NULL);	
   
}
  
/**
 * @brief 创建数据库表
 *
 * @param db 数据库连接（`sqlite3` 指针）
 * @param sql 用于创建表的 SQL 语句（字符串）
 * @return void 无返回值
 *
 * 此函数执行传入的 SQL 创建表语句。如果执行过程中发生错误，错误信息会被释放。
 */
void db_create(sqlite3 *db, char *sql)
{
    if (db == NULL) {
        fprintf(stderr, "Database pointer is NULL in db_create.\n");
        exit(EXIT_FAILURE);
    }

	if (sqlite3_exec(db, sql, NULL, 0, &db_ErrMsg) != SQLITE_OK)
	{		
		sqlite3_free(db_ErrMsg);
	}
}

/**
 * @brief 关闭数据库连接
 *
 * @param db 数据库连接（`sqlite3` 指针）
 * @param filename 数据库文件名（字符串）
 * @return void 无返回值
 *
 * 该函数关闭数据库连接。如果关闭数据库时发生错误，将输出错误信息并终止程序。
 */
void db_close(sqlite3 *db, char *filename)
{
	if (sqlite3_close(db) != SQLITE_OK)
	{
		printf("db_close() error: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	printf("Close database '%s' ok\n", filename);
}

/**
 * @brief 向数据库插入数据
 *
 * @param db 数据库连接（`sqlite3` 指针）
 * @param sql 插入数据的 SQL 语句（字符串）
 * @return void 无返回值
 *
 * 此函数执行插入数据的 SQL 语句。如果执行过程中发生错误，错误信息会被释放。
 */
void db_insert(sqlite3 *db, char *sql)
{
    if (sqlite3_exec(db, sql, NULL, 0, &db_ErrMsg) != SQLITE_OK)
    {
        sqlite3_free(db_ErrMsg);  // 释放错误信息
    }
}
/**
 * @brief 从数据库删除数据
 *
 * @param db 数据库连接（`sqlite3` 指针）
 * @param sql 删除数据的 SQL 语句（字符串）
 * @return void 无返回值
 *
 * 此函数执行删除数据的 SQL 语句。如果执行过程中发生错误，错误信息会被释放。
 */

void db_delete(sqlite3 *db, char *sql)
{ 

	if (sqlite3_exec(db, sql, NULL, 0, &db_ErrMsg) != SQLITE_OK)
	{
		sqlite3_free(db_ErrMsg);
	}

}

/**
 * @brief 执行数据库查询并处理查询结果
 *
 * @param db 数据库连接句柄（`sqlite3` 指针）
 * @param sql 要执行的 SQL 查询语句（字符串）
 * @param callback 查询结果的回调函数，用于处理查询结果（通常是对每行数据进行处理）
 * @param data 回调函数的数据指针，通常传递给回调函数进行数据存储或其他处理
 * @return void 无返回值
 *
 * 该函数通过 `sqlite3_exec` 执行 SQL 查询，并将查询结果传递给指定的回调函数进行处理。
 * 如果执行过程中发生错误，错误信息会存储在 `db_ErrMsg` 中并被释放。
 */
void db_select(sqlite3 *db, char *sql, db_callback_func callback, void *data)
{
	if (sqlite3_exec(db, sql, callback, data, &db_ErrMsg) != SQLITE_OK)
	{
		sqlite3_free(db_ErrMsg);
	}

}

/**
 * @brief 将数据写入数据库表
 *
 * @param db 数据库连接句柄（`sqlite3` 指针）
 * @param pack 要写入的数据包（`package` 类型指针，包含 ID、时间和温度信息）
 * @return int 返回状态码
 *         -  0: 成功写入数据
 *         - -1: 参数无效（`db` 或 `pack` 为 `NULL`）
 *         - -2: 插入数据失败（`db_insert` 内部处理错误）
 */
int write_to_db(sqlite3 *db, package *pack)
{
	char		sql[256] = {0};

	if (!db || !pack)
	{
		printf("Invalid parameters\n");
		return -1;
	}

	snprintf(sql, sizeof(sql), "insert into %s values (NULL, '%s', '%s', '%.2f');", TA_NAME, pack->id, pack->time, pack->temperature);

	db_insert(db, sql);
	printf("Push pack in database ok\n");

	return 0;
}

/**
 * @brief SQLite 查询回调函数，用于获取记录总数
 *
 * @param data 用户传入的指针（此处为 `int` 类型指针，用于存储记录总数）
 * @param argc 查询结果中字段的数量（通常为1，因为是 `count(*)` 查询）
 * @param argv 查询结果中字段值的数组
 * @param azColName 查询结果中字段名称的数组
 * @return int 始终返回 0，表示回调执行成功
 */
static int callback_count(void *data, int argc, char **argv, char **azColName)
{
    int *count = (int *)data;

    *count = atoi(argv[0]);

    return 0;
}

/**
 * @brief 获取数据库表中记录的总数
 *
 * @param db 数据库连接句柄（`sqlite3` 指针）
 * @return int 返回记录总数，或错误代码：
 *         -  >0: 表中记录的数量
 *         -  -1: 数据库句柄无效（`db` 为 `NULL`）
 *         -  -2: 查询执行失败（`sqlite3_exec` 调用错误）
 */
int db_count(sqlite3 *db)
{
	int 	count = 0;
	char	sql[256] = {0};

	if (!db)
	{
		return -1;
	}

	db_create(db, TA_CREATE);

	snprintf(sql, sizeof(sql), "select count(*) from %s;", TA_NAME);

	if (sqlite3_exec(db, sql, callback_count, &count, &db_ErrMsg) != SQLITE_OK)
	{	
		sqlite3_free(db_ErrMsg);
		return -2;
	}

	return count;
}

/** 
 * @brief SQLite 查询回调函数，用于解析查询结果并存储到 `package` 结构体中
 *
 * @param data 用户传入的数据指针（此处为 `package` 类型指针）
 * @param argc 查询结果中字段的数量
 * @param argv 查询结果中字段值的数组
 * @param azColName 查询结果中字段名称的数组
 * @return int 始终返回 0，表示回调执行成功
 */
static int callback_pull(void *data, int argc, char **argv, char **azColName)
{
	package   *pack = (package *)data;
	
	strncpy(pack->id, argv[1], sizeof(pack->id));
	strncpy(pack->time, argv[2], sizeof(pack->time));
	pack->temperature = atof(argv[3]);

    return 0;
}


/** 
 * @brief 从数据库读取一条记录，并删除该记录
 *
 * @param db 数据库连接句柄（`sqlite3` 指针）
 * @param pack 用于存储读取结果的结构体指针
 * @return int 返回状态码
 *         -  0: 成功读取并删除记录
 *         - -1: 参数无效（`db` 或 `pack` 为 `NULL`）
 *         - -2: 查询或删除操作失败（具体原因由内部函数日志记录）
 */
int read_from_db(sqlite3 *db, package *pack)
{
	char		sql_select[256] = {0};
	char		sql_delete[256] = {0};

	if (!db || !pack)
	{
		printf("Invalid parameters\n");
		return -1;
	}

	memset(pack, 0, sizeof(*pack));
	snprintf(sql_select, sizeof(sql_select), "select * from %s limit 1;", TA_NAME);
	snprintf(sql_delete, sizeof(sql_delete), "delete from %s where rowid = (select rowid from %s limit 1);", TA_NAME, TA_NAME);

	db_select(db, sql_select, callback_pull, pack);
	db_delete(db, sql_delete);

	printf("Pull pack ok: ID[%s], TIME[%s], TEMP[%.2f]\n", pack->id, pack->time, pack->temperature);

	return 0;
}

/**
 * 从数据库中提取第一个数据包，并将其复制到提供的缓冲区。
 * 
 * @param pack 指向目标缓冲区的指针，用于存储从数据库中提取的数据包。
 * @param size 指定目标缓冲区的大小，以字节为单位。
 * @param bytes 指针，用于返回从数据库中提取的数据的实际字节数。
 * 
 * @return 如果成功，从数据库中提取数据包并复制到缓冲区，返回 0。
 *         如果输入参数无效或数据提取失败，返回负值作为错误码。
 * 
 * 错误码说明：
 * - -1：输入参数无效或缓冲区大小不足。
 * - -2：数据库未打开。
 * - -3：`sqlite3_prepare_v2` 调用失败。
 * - -5：`sqlite3_step` 调用失败。
 * - -6：数据库中提取的 BLOB 数据为 `NULL`。
 */
int database_pop_packet(void *pack, int size, int *bytes)
{
    char               sql[SQL_COMMAND_LEN]={0};
    int                rv = 0;
    sqlite3_stmt      *stat = NULL;
    const void        *blob_ptr;
 
    if( !pack || size<=0 )
    {
        // log_error("%s() Invalid input arguments\n", __func__);
        return -1;
    }
 
    if( ! s_clidb )
    {
        // log_error("sqlite database not opened\n");
        return -2;
    }
 
    /* Only query the first packet record */
    snprintf(sql, sizeof(sql), "SELECT packet FROM %s WHERE rowid = (SELECT rowid FROM %s LIMIT 1);", TA_NAME, TA_NAME);
    rv = sqlite3_prepare_v2(s_clidb, sql, -1, &stat, NULL);
    if(SQLITE_OK!=rv || !stat)
    {
        // log_error("firehost sqlite3_prepare_v2 failure\n");
        rv = -3;
        goto out;
    }
 
    rv = sqlite3_step(stat);
    if( SQLITE_DONE!=rv && SQLITE_ROW!=rv )
    {
        // log_error("firehost sqlite3_step failure\n");
        rv = -5;
        goto out;
    }
 
    /* 1rd argument<0> means first segement is packet  */
    blob_ptr = sqlite3_column_blob(stat, 0);
    if( !blob_ptr )
    {
        rv = -6;
        goto out;
    }
 
    *bytes = sqlite3_column_bytes(stat, 0);
 
    if( *bytes > size )
    {
        // log_error("blob packet bytes[%d] larger than bufsize[%d]\n", *bytes, size);
        *bytes = 0;
        rv = -1;
    }
 
    memcpy(pack, blob_ptr, *bytes);
    rv = 0;
 
out:
    sqlite3_finalize(stat);
    return rv;
}
 
 
/**
 * 从数据库中提取第一个数据包，并将其复制到提供的缓冲区。
 * 
 * @param pack 指向目标缓冲区的指针，用于存储从数据库中提取的数据包。
 * @param size 指定目标缓冲区的大小，以字节为单位。
 * @param bytes 指针，用于返回从数据库中提取的数据的实际字节数。
 * 
 * @return 如果成功，从数据库中提取数据包并复制到缓冲区，返回 0。
 *         如果输入参数无效或数据提取失败，返回负值作为错误码。
 * 
 * 错误码说明：
 * - -1：输入参数无效或缓冲区大小不足。
 * - -2：数据库未打开。
 * - -3：`sqlite3_prepare_v2` 调用失败。
 * - -5：`sqlite3_step` 调用失败。
 * - -6：数据库中提取的 BLOB 数据为 `NULL`。
 */
int database_del_packet(void)
{
    char               sql[SQL_COMMAND_LEN]={0};
    char              *errmsg = NULL;
 
    if( ! s_clidb )
    {
        // log_error("sqlite database not opened\n");
        return -2;
    }
 
    /*  remove packet from db */
    memset(sql, 0, sizeof(sql));
    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE rowid = (SELECT rowid FROM %s LIMIT 1);", TA_NAME, TA_NAME);
    if( SQLITE_OK != sqlite3_exec(s_clidb, sql, NULL, 0, &errmsg) )
    {
        // log_error("delete first blob packet from database failure: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -2;
    }
    // log_warn("delete first blob packet from database ok\n");
 
    /*  Vacuum the database */
    sqlite3_exec(s_clidb, "VACUUM;", NULL, 0, NULL);
 
    return 0;
}
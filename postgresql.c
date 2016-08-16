#include "header.h"

//初始化数据库
PGconn* sql_init()
{
	char conninfo[128] = {0};
	snprintf(conninfo, 127, "dbname=%s user=%s password=%s hostaddr=%s",
			DB_NAME, DB_USERNAME, DB_PASSWORD, DB_HOSTADDR);
	// 获取链接
	PGconn* conn = PQconnectdb(conninfo);

	// 检查链接状态
	if(PQstatus(conn) != CONNECTION_OK){
		xyprintf(0, "SQL_ERROR:connect to database failed!\n\t%s", PQerrorMessage(conn));
		PQfinish(conn);
		return 0;
	}
	
	return conn;
}

//执行非select语句
int sql_exec(PGconn *conn, char *sql_str)
{
	PGresult *res = PQexec(conn, sql_str);

	if( PQresultStatus(res) != PGRES_COMMAND_OK ){
		xyprintf(0, "SQL_ERROR:exec error -- %s\n\t%s", sql_str, PQerrorMessage(conn) );
		PQclear(res);
		return -1;
	}

	PQclear(res);
	return 0;
}

//执行select语句
int sql_exec_select(PGconn *conn, char *sql_str, PGresult** res)
{
	*res = PQexec(conn,sql_str);

	if(PQresultStatus(*res) != PGRES_TUPLES_OK){
		xyprintf(0, "SQL_ERROR:exec error -- %s\n\t%s", sql_str, PQerrorMessage(conn));
		PQclear(*res);
		return -1;
	}

	return 0;
}

// 返回结果res中第h行第l列字段的值，以字符串形式
char* sql_getvalue_string(PGresult *res, int h, int l)
{
	return PQgetvalue(res,h,l);
}

// 关闭数据库链接
void sql_destory(PGconn *conn)
{
	PQfinish(conn);
}

// 数据库测试
void sql_test()
{
	xyprintf(0, "sql init......");
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "sql init failed!");
		goto ERROR;
	}
	xyprintf(0, "sql init success!");

	char *sql_str = "select * from users;";
	
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		goto SQL_ERROR;
	}

	int tuples = PQntuples(sql_res);
	xyprintf(0, "tuples = %d", tuples);

	xyprintf(0, "sql_getvalue_string() -- %s", sql_getvalue_string(sql_res, 1, 1) );

	PQclear(sql_res);
	

SQL_ERROR:
	sql_destory(conn);
ERROR:
	return;
}

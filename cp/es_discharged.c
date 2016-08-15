/*****************************************************
 *
 * discharged操作函数
 *
 *****************************************************/

#include "header.h"

// 插入记录到临时放行表
int insert_discharged(char* userip, char* acip)
{
	// 建立数据库连接	
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	// 删除已有记录
	char sql_str[1024] = {0};
	snprintf(sql_str, 1023, "DELETE FROM wifi_user_temp_discharged WHERE userip = '%s' AND acip = '%s'", userip, acip);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 添加临时放行记录
	snprintf(sql_str, 1023, "INSERT INTO wifi_user_temp_discharged(userip, acip, discharged_time) VALUES('%s','%s',NOW())", userip, acip);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
	
	// 获取插入的id值
	snprintf(sql_str, 1023, "SELECT CURRVAL('wifi_user_temp_discharged_id_seq')");	//获取刚插入记录的主键id号
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
	
	int id = atoi(sql_getvalue_string(sql_res, 0, 0));

	PQclear(sql_res);
	
	sql_destory(conn);
	return id;
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

// 删除临时放行表纪录
int delete_discharged(char* userip, char* acip)
{
	// 建立数据库连接	
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	// 删除记录
	char sql_str[1024] = {0};
	snprintf(sql_str, 1023, "DELETE FROM wifi_user_temp_discharged WHERE userip = '%s' AND acip = '%s'", userip, acip);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	sql_destory(conn);
	return 0;
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

#define LOOP_TEMP_DISCHARGED_INTERVAL	10
#define TEMP_DISCHARGED					300

void* loop_temp_discharged_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Loop discharged thread is running!!!");

	unsigned int id = 0;
	char userip[32] = {0};
	char acip[32] = {0};
	char sql_str[1024] = {0};
	PGresult* sql_res;
	int tuples;
	int i;

	while(1){
		xyprintf(0, "** (～﹃～)~zZ ~~ Loop discharged thread will sleep %u s!!!\n", LOOP_TEMP_DISCHARGED_INTERVAL );
		sleep( LOOP_TEMP_DISCHARGED_INTERVAL );
		xyprintf(0, "TFREE:** O(∩ _∩ )O ~~ Loop discharged thread is get up!!!");
		
		// 建立连接
		PGconn* conn = sql_init();
		if(!conn){
			xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
			continue;
		}
		
		//查出来
		snprintf(sql_str, 1023,
				"SELECT id, userip, acip from wifi_user_temp_discharged"
				" WHERE EXTRACT(epoch FROM CURRENT_TIMESTAMP) - EXTRACT(epoch FROM discharged_time) > %u",
				TEMP_DISCHARGED);
		
		if( sql_exec_select(conn, sql_str, &sql_res) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}
		
		// 获取查询出来的条数
		tuples = PQntuples(sql_res);
		xyprintf(0, "Get recond num is %d", tuples);
		for(i = 0; i < tuples; i++){
			// 获取查询出来的数据
			id = atoi(sql_getvalue_string(sql_res, i, 0));
			snprintf(userip, 31, "%s", sql_getvalue_string(sql_res, i, 1));
			snprintf(acip, 31, "%s", sql_getvalue_string(sql_res, i, 2));
			
			xyprintf(0, "%u -- %s -- %s", id, userip, acip);
			
			// 发送登出信息
			//if( SendReqLogoutAndRecv(userip, acip, PORTAL_TO_AC_PORT) ){
			//	xyprintf(0, "logout failed!!!!!");
			//}
			//else {
			//	xyprintf(0, "logout success!!!!!");
			//}
			
			// 删除记录
			snprintf(sql_str, 1023, "DELETE FROM wifi_user_temp_discharged WHERE id = %u", id);
			if( sql_exec(conn, sql_str) ){
				xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
				PQclear(sql_res);
				goto SQLED_ERROR;
			}
			xyprintf(0, "Delete %u success!", id);
		}

		// 清空查询语句的回复资源
		PQclear(sql_res);
SQLED_ERROR:
		//当一轮循环执行完成的时候,销毁数据库连接资源在下次执行的时候重新创建,如果执行出现错误时,提前结束
		sql_destory(conn);
	}//while(1);

ERROR:
	xyprintf(0, "LOGLASTTOLOG:✟ ✟ ✟ ✟ -- %s %d:Loglast to log pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}
		//处理时间测试 -- 开始时间
//		struct timeval start, end;
//		gettimeofday( &start, NULL );
		//处理时间 -- 结束时间
//		gettimeofday( &end, NULL );
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺ ");
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺  Loglast to log's bao ping'an, , used %d s %d us!!!", end.tv_sec - start.tv_sec, end.tv_usec -start.tv_usec);
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺ ");

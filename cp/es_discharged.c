/*****************************************************
 *
 * discharged操作函数
 *
 *****************************************************/

#include "header.h"



#define SQL_INSERT_tb_TmpUserDischarged \
	"INSERT INTO tb_TmpUserDischarged(userip, acip, dischargedtime) VALUES('%s','%s',GETDATE())"

#define SQL_SELECT_tb_TmpUserDischarged \
	"SELECT id, userip, acip from tb_TmpUserDischarged WHERE DATEDIFF(s, dischargedtime, GETDATE()) > %u"

#define SQL_DELETE_tb_TmpUserDischarged_ID \
	"DELETE tb_TmpUserDischarged WHERE id = %u"

#define SQL_DELETE_tb_TmpUserDischarged_IP \
	"DELETE tb_TmpUserDischarged WHERE userip = '%s' and acip = '%s'"

// 插入记录到临时放行表
int insert_discharged(char* userip, char* acip)
{
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}
	
	snprintf(handle.sql_str, 1024, SQL_DELETE_tb_TmpUserDischarged_IP, userip, acip);
	if(wt_sql_exec(&handle)){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	
	snprintf(handle.sql_str, 1024, SQL_INSERT_tb_TmpUserDischarged, userip, acip);
	if(wt_sql_exec(&handle)){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	
	unsigned int id;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &id,		20, &handle.sql_err);
	snprintf(handle.sql_str, 1024, "SELECT SCOPE_IDENTITY()");	//获取刚插入记录的主键id号
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);

	wt_sql_destroy(&handle);
	return id;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}

// 删除临时放行表纪录
int delete_discharged(char* userip, char* acip)
{
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}
	
	snprintf(handle.sql_str, 1024, SQL_DELETE_tb_TmpUserDischarged_IP, userip, acip);
	if(wt_sql_exec(&handle)){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	
	wt_sql_destroy(&handle);
	return 0;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}

#define LOOP_TEMP_DISCHARGED_INTERVAL	10
#define TEMP_DISCHARGED					300

void* loop_temp_discharged_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Loop discharged thread is running!!!");

	while(1){
//		xyprintf(0, "** (～﹃～)~zZ ~~ Loop discharged thread will sleep %u s!!!", LOOP_TEMP_DISCHARGED_INTERVAL );
		sleep( LOOP_TEMP_DISCHARGED_INTERVAL );
//		xyprintf(0, "TFREE:** O(∩ _∩ )O ~~ Loop discharged thread is get up!!!");
	
		// 数据库访问资源
		wt_sql_handle *s_handle = malloc(sizeof(wt_sql_handle));		//SELECT用
		memset(s_handle, 0, sizeof(wt_sql_handle));
		wt_sql_handle *iud_handle = malloc(sizeof(wt_sql_handle));		//INSERT UPDATE DELETE用`
		memset(iud_handle, 0, sizeof(wt_sql_handle));
	
		
		while( wt_sql_init(s_handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){			// 数据库初始化
			xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
			sleep( 10 );
		}
		
		while( wt_sql_init(iud_handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){		// 数据库初始化
			xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
			sleep( 10 );
		}
		
		//处理时间测试 -- 开始时间
//		struct timeval start, end;
//		gettimeofday( &start, NULL );
		
		// 用户上网记录表 参数
		unsigned int id;
		char userip[32], acip[32];
		
		SQLBindCol(s_handle->sqlstr_handle, 1,	SQL_C_ULONG,	&id,		32, &s_handle->sql_err);
		SQLBindCol(s_handle->sqlstr_handle, 2,	SQL_C_CHAR,		&userip,	32, &s_handle->sql_err);
		SQLBindCol(s_handle->sqlstr_handle, 3,	SQL_C_CHAR,		&acip,		32, &s_handle->sql_err);
		
		//查出来
		sprintf(s_handle->sql_str, SQL_SELECT_tb_TmpUserDischarged, TEMP_DISCHARGED);
		if(wt_sql_exec(s_handle)){
			xyprintf(0, "SQL_ERROR: %s %d -- s_handle->sql_str: %s", __FILE__, __LINE__, s_handle->sql_str);
			goto STR_ERR;
		}

		//获取第一条记录
		s_handle->sql_ret = SQLFetch(s_handle->sqlstr_handle);

		// 循环遍历查询出的信息
		while( s_handle->sql_ret != SQL_NO_DATA){
			xyprintf(0, "Get a recond, id %u, userip: %s, acip: %s", id, userip, acip);
		
			// 发送登出信息
			if( SendReqLogoutAndRecv(userip, acip, PORTAL_TO_AC_PORT) ){
				xyprintf(0, "logout failed!!!!!");
			}
			else {
				xyprintf(0, "logout success!!!!!");
			}

			// 删除记录
			sprintf(iud_handle->sql_str, SQL_DELETE_tb_TmpUserDischarged_ID, id);
			if(wt_sql_exec(iud_handle)){
				xyprintf(0, "SQL_ERROR: %s %d -- iud_handle->sql_str: %s", __FILE__, __LINE__, iud_handle->sql_str);
				goto STR_ERR;
			}

			//打印一下
			xyprintf(0, "Delete a recond of id = %u", id);
			
			s_handle->sql_ret = SQLFetch(s_handle->sqlstr_handle);	
		}// end while

		//当一轮循环执行完成的时候,销毁数据库连接资源在下次执行的时候重新创建,如果执行出现错误时,提前结束
STR_ERR:
		//处理时间 -- 结束时间
//		gettimeofday( &end, NULL );
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺ ");
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺  Loglast to log's bao ping'an, , used %d s %d us!!!", end.tv_sec - start.tv_sec, end.tv_usec -start.tv_usec);
//		xyprintf(0, "LOGLASTTOLOG:☻ ☺ ☻ ☺ ");

		wt_sql_destroy(s_handle);
		free(s_handle);
		wt_sql_destroy(iud_handle);
		free(iud_handle);
	}//while(1);

ERR:
	xyprintf(0, "LOGLASTTOLOG:✟ ✟ ✟ ✟ -- %s %d:Loglast to log pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

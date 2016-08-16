/*****************************************************
 *
 * 数据库操作函数
 *
 *****************************************************/

#include "header.h"

#define EXEC_SQL_DEBUG		1

// 用apmac查询门店apid、ap域名、门店id
int get_apinfo(char* apmac, unsigned int *apid, char* domain, unsigned int *s_id)
{
	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询
	snprintf(sql_str, 1023, "SELECT id, domain, s_id FROM ap WHERE mac = '%s'", apmac);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:Not find recond of ap mac is %s", apmac);
		goto RESULTED_ERROR;
	}

	// 取值
	*apid = atoi(sql_getvalue_string(sql_res, 0, 0));
	strcpy(domain, sql_getvalue_string(sql_res, 0, 1));
	*s_id = atoi(sql_getvalue_string(sql_res, 0, 2));

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%u), domain(%s), s_id(%u) FROM ap WHERE apmac = %s success!", *apid, domain, *s_id, apmac);
#endif

	PQclear(sql_res);
	sql_destory(conn);
	return 0;
	
RESULTED_ERROR:
	PQclear(sql_res);
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

// 用acname查询门店acip
int get_acinfo(char* acname, unsigned int *acid, char* acip, int* acport)
{
	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询
	snprintf(sql_str, 1023, "SELECT id, ip, port FROM ac WHERE wlanacname = '%s'", acname);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:Not find recond of ac name is %s", acname);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	*acid = atoi(sql_getvalue_string(sql_res, 0, 0));
	strcpy(acip, sql_getvalue_string(sql_res, 0, 1));
	*acport = atoi(sql_getvalue_string(sql_res, 0, 2));

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%u), ip(%s), port(%d) FROM ac WHERE wlanacname = %s success!", *acid, acip, acport, acname);
#endif

	PQclear(sql_res);
	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

// 用户手机登录信息
// s_id 门店id
// type 认证类型，weixin，phone
// para 认证参数1
// para2 认证参数2
// acid
// wlanparameter
// wu_id 传回参数
int get_wuid(unsigned int s_id, char* type, char* para1, char* para2, unsigned int acid, char* wlanparameter, unsigned int *wu_id)
{
	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};

	// 查询
	char usermac[256] = {0};
	snprintf(sql_str, 1023, "SELECT mac FROM wifi_user_wlanparameter WHERE wlanparameter = '%s' AND acid = %u", wlanparameter, acid);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到 没有查询到使用wlanparameter替代mac
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		strcpy(usermac, wlanparameter);
#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s failed, usermac use wlanparameter!", sql_str);
#endif
	}
	else {
		strcpy(usermac, sql_getvalue_string(sql_res, 0, 0));
#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s success!\n\t\tusermac is %s", sql_str, usermac);
#endif
	}
	PQclear(sql_res);

	// 查询mac是否存在
	snprintf(sql_str, 1023, "SELECT id, phonenum, wx_openid, wx_tid FROM wifi_user WHERE s_id = %u AND mac = '%s'", s_id, usermac);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到 没有查询到创建新的记录
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		PQclear(sql_res);

#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s failed!", sql_str);
#endif
		
		// 没有查询到 添加新的记录
		if( !strcmp(type, "phone") ){
			snprintf(sql_str, 1023, "INSERT INTO wifi_user(s_id, mac, phonenum, created_at, updated_at)"
					" VALUES(%u, '%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
					s_id, usermac, para1);
		}
		else if( !strcmp(type, "weixin") ){
			snprintf(sql_str, 1023, "INSERT INTO wifi_user(s_id, mac, wx_openid, wx_tid, created_at, updated_at)"
					" VALUES(%u, '%s', '%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
					s_id, usermac, para1, para2);
		}
		else {
			xyprintf(0, "ERROR:%s %d -- type(%s) is error!", __FILE__, __LINE__, type);
			goto SQLED_ERROR;
		}

		if( sql_exec(conn, sql_str) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}

#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif

		// 获取插入的id值
		snprintf(sql_str, 1023, "SELECT CURRVAL('user_wifi_user_id_seq')");	//获取刚插入记录的主键id号
		if( sql_exec_select(conn, sql_str, &sql_res) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}
	
		*wu_id = atoi(sql_getvalue_string(sql_res, 0, 0));
		PQclear(sql_res);
#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s(%u) success!", sql_str, *wu_id);
#endif
	}
	else {
		xyprintf(0, "mac is cunzai!");
		*wu_id = atoi(sql_getvalue_string(sql_res, 0, 0));
		// 比较手机号码是否变化，变化则修改

		if( !strcmp(type, "phone") ){
			if( strcmp( para1, sql_getvalue_string(sql_res, 0, 1)) ){
				snprintf(sql_str, 1023, "UPDATE wifi_user SET phonenum = '%s' WHERE id = %u",
						para1, *wu_id);
				if( sql_exec(conn, sql_str) ){
					xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
					goto RESULTED_ERROR;
				}
#if EXEC_SQL_DEBUG
				xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif
			}
		}
		else if( !strcmp(type, "weixin") ){
			if( strcmp( para1, sql_getvalue_string(sql_res, 0, 2)) ||
					strcmp( para2, sql_getvalue_string(sql_res, 0, 3)) ){

				snprintf(sql_str, 1023, "UPDATE wifi_user SET wx_openid = '%s', wx_tid = '%s' WHERE id = %u",
						para1, para2, *wu_id);
				if( sql_exec(conn, sql_str) ){
					xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
					goto RESULTED_ERROR;
				}
#if EXEC_SQL_DEBUG
				xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif
			}
		}
		else if( !strcmp(type, "white") ){
#if EXEC_SQL_DEBUG
			xyprintf(0, "This user is in the white list!");
#endif
			
		}
		else {
			xyprintf(0, "ERROR:%s %d -- type(%s) is error!", __FILE__, __LINE__, type);
			goto RESULTED_ERROR;
		}
		PQclear(sql_res);
	}

	sql_destory(conn);
	return 0;

RESULTED_ERROR:
	PQclear(sql_res);
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

int update_wifi_user(char* username, char* acip, char* usermac)
{
	int wu_id, login_type;
	if( res_username(username, &wu_id, &login_type) ){
		xyprintf(0, "ERROR:%s %d -- Res username error!", __FILE__, __LINE__);
		return -1;
	}

	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询mac
	snprintf(sql_str, 1023, "SELECT mac FROM wifi_user WHERE id = %d", wu_id);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:Not find recond of wu_id is %d", wu_id);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	char sqlmac[256] = {0};
	strcpy(sqlmac, sql_getvalue_string(sql_res, 0, 0));

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT mac(%s) FROM wifi_user WHERE id = %d success!", sqlmac, wu_id);
#endif

	PQclear(sql_res);

	
	if( strcmp(sqlmac, usermac) ){

		snprintf(sql_str, 1023, "SELECT id FROM ac WHERE ip = '%s'", acip);
		if( sql_exec_select(conn, sql_str, &sql_res) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}

		// 判断是否查询到
		tuples = PQntuples(sql_res);
		if( tuples < 1 ){
			xyprintf(0, "ERROR:Not find recond of acip is %s", acip);
			PQclear(sql_res);
			goto SQLED_ERROR;
		}

		// 取值
		int acid = atoi(sql_getvalue_string(sql_res, 0, 0));

#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%d) FROM ac WHERE ip = %s success!", acid, acip);
#endif

		PQclear(sql_res);

		// 更新mac
		snprintf(sql_str, 1023, "UPDATE wifi_user SET mac = '%s' WHERE id = %d", usermac, wu_id);
		if( sql_exec(conn, sql_str) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}
#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif
		// 插入mac对应表
		snprintf(sql_str, 1023, "INSERT INTO wifi_user_wlanparameter(wlanparameter, mac, acid)"
				" VALUES('%s', '%s', %d)",
				sqlmac, usermac, acid);
		if( sql_exec(conn, sql_str) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}
#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif
	}

	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

int user_online(char* username, char* userip, char* acip, char* apmac)
{
	int wu_id, login_type;
	if( res_username(username, &wu_id, &login_type) ){
		xyprintf(0, "ERROR:%s %d -- Res username error!", __FILE__, __LINE__);
		return -1;
	}

	if( login_type == LOGIN_TYPE_TEMP ){
		return 0;
	}

	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询s_id
	snprintf(sql_str, 1023, "SELECT s_id FROM wifi_user WHERE id = %d", wu_id);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of wu_id is %d", __FILE__, __LINE__, wu_id);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int s_id = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT s_id(%d) FROM wifi_user WHERE id = %d success!", s_id, wu_id);
#endif

	PQclear(sql_res);

	// 查询auth_time
	snprintf(sql_str, 1023, "SELECT auth_time FROM store WHERE id = %d", s_id);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:Not find recond of s_id is %d", s_id);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int auth_time = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT auth_time(%d) FROM store WHERE id = %d success!", auth_time, s_id);
#endif

	PQclear(sql_res);
	
	// 查询apid
	snprintf(sql_str, 1023, "SELECT id FROM ap WHERE mac = '%s'", apmac);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of apmac is %s", __FILE__, __LINE__, apmac);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int apid = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%d) FROM ap WHERE mac = %s success!", apid, apmac);
#endif

	PQclear(sql_res);
	
	// 查询apid
	snprintf(sql_str, 1023, "SELECT port FROM ac WHERE ip = '%s'", acip);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of acip is %s", __FILE__, __LINE__, acip);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int acport = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT port(%d) FROM ac WHERE ip = %s success!", acport, acip);
#endif

	PQclear(sql_res);


	// 在线状态
	snprintf(sql_str, 1023, "UPDATE wifi_user SET isonline = true WHERE id = %d", wu_id);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif

	// log
	snprintf(sql_str, 1023, "INSERT INTO wifi_user_log(wu_id, ip, type, time, apid, login_type)"
			" VALUES(%d, '%s', 1, CURRENT_TIMESTAMP, %d, %d)",
			wu_id, userip, apid, login_type);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif

	// deadline
	if( insert_deadline(userip, acip, acport, auth_time) ){
		xyprintf(0, "ERROR:%s %d -- insert deadline error!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

int user_offline(char* username, char* userip, char* acip, char* apmac)
{
	int wu_id, login_type;
	if( res_username(username, &wu_id, &login_type) ){
		xyprintf(0, "ERROR:%s %d -- Res username error!", __FILE__, __LINE__);
		return -1;
	}

	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询s_id
	snprintf(sql_str, 1023, "SELECT s_id FROM wifi_user WHERE id = %d", wu_id);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of wu_id is %d", __FILE__, __LINE__, wu_id);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int s_id = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT s_id(%d) FROM wifi_user WHERE id = %d success!", s_id, wu_id);
#endif

	PQclear(sql_res);

	// 查询apid
	snprintf(sql_str, 1023, "SELECT id FROM ap WHERE mac = '%s'", apmac);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of apmac is %s", __FILE__, __LINE__, apmac);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int apid = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%d) FROM ap WHERE mac = %s success!", apid, apmac);
#endif

	PQclear(sql_res);
	
	// 查询apid
	snprintf(sql_str, 1023, "SELECT port FROM ac WHERE ip = '%s'", acip);
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到
	tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		xyprintf(0, "ERROR:%s %d -- Not find recond of acip is %s", __FILE__, __LINE__, acip);
		PQclear(sql_res);
		goto SQLED_ERROR;
	}

	// 取值
	int acport = atoi( sql_getvalue_string(sql_res, 0, 0) );

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT port(%d) FROM ac WHERE ip = %s success!", acport, acip);
#endif

	PQclear(sql_res);


	// 在线状态
	snprintf(sql_str, 1023, "UPDATE wifi_user SET isonline = false WHERE id = %d", wu_id);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif

	// log
	snprintf(sql_str, 1023, "INSERT INTO wifi_user_log(wu_id, ip, type, time, apid, login_type)"
			" VALUES(%d, '%s', 0, CURRENT_TIMESTAMP, %d, %d)",
			wu_id, userip, apid, login_type);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: %s success!", sql_str);
#endif

	// 删除已有记录
	snprintf(sql_str, 1023, "DELETE FROM wifi_user_deadline WHERE userip = '%s' AND acip = '%s'", userip, acip);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
	
	xyprintf(0, "EXEC_SQL_DEBUG:%s success!", sql_str);

	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

// 插入记录到临时放行表
int insert_deadline(char* userip, char* acip, int acport, int discharged_time)
{
	// 建立数据库连接	
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	// 删除已有记录
	char sql_str[1024] = {0};
	snprintf(sql_str, 1023, "DELETE FROM wifi_user_deadline WHERE userip = '%s' AND acip = '%s'", userip, acip);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	xyprintf(0, "EXEC_SQL_DEBUG:%s success!", sql_str);

	// 添加临时放行记录
	snprintf(sql_str, 1023, "INSERT INTO wifi_user_deadline(userip, acip, acport, deadline_time)"
			" VALUES('%s','%s', %d, CURRENT_TIMESTAMP + interval '%d S')",
			userip, acip, acport, discharged_time);
	if( sql_exec(conn, sql_str) ){
		xyprintf(0, "ERROR:%s %d -- sql exec failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}
	
	xyprintf(0, "EXEC_SQL_DEBUG:%s success!", sql_str);
	
	sql_destory(conn);
	return 0;
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

void* loop_deadline_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Loop deadline thread is running!!!");

	unsigned int id = 0;
	char userip[32] = {0};
	char acip[32] = {0};
	int acport;
	char sql_str[1024] = {0};
	PGresult* sql_res;
	int tuples;
	int i;

	while(1){
//		xyprintf(0, "** (～﹃～)~zZ ~~ Loop deadline thread will sleep %u s!!!\n", LOOP_DEADLINE_INTERVAL );
		sleep( LOOP_DEADLINE_INTERVAL );
//		xyprintf(0, "TFREE:** O(∩ _∩ )O ~~ Loop deadline thread is get up!!!");
		
		// 建立连接
		PGconn* conn = sql_init();
		if(!conn){
			xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
			continue;
		}
		
		//查出来
		snprintf(sql_str, 1023,
				"SELECT id, userip, acip, acport from wifi_user_deadline"
				" WHERE EXTRACT(epoch FROM CURRENT_TIMESTAMP) - EXTRACT(epoch FROM deadline_time) > 0");
		
		if( sql_exec_select(conn, sql_str, &sql_res) ){
			xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
			goto SQLED_ERROR;
		}
		
		// 获取查询出来的条数
		tuples = PQntuples(sql_res);
		xyprintf(0, "DEADLINE:%d recond!", tuples);
		for(i = 0; i < tuples; i++){
			// 获取查询出来的数据
			id = atoi(sql_getvalue_string(sql_res, i, 0));
			snprintf(userip, 31, "%s", sql_getvalue_string(sql_res, i, 1));
			snprintf(acip, 31, "%s", sql_getvalue_string(sql_res, i, 2));
			acport = atoi(sql_getvalue_string(sql_res, i, 3));
			

			xyprintf(0, "%u -- %s -- %s -- %d", id, userip, acip, acport);
			
			// 发送登出信息
			SendReqLogoutAndRecv(userip, acip, acport);
/*
			if( SendReqLogoutAndRecv(userip, acip, acport) ){
				xyprintf(0, "logout failed!!!!!");
			}
			else {
				xyprintf(0, "logout success!!!!!");
			}
*/			
			// 删除记录
			snprintf(sql_str, 1023, "DELETE FROM wifi_user_deadline WHERE id = %u", id);
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

int exec_sql_test()
{
	char *apmac = "123456789012";
	unsigned int apid, s_id;
	char domain[256] = {0};
	int ret;
	ret = get_apinfo(apmac, &apid, domain, &s_id);
	xyprintf(0, "ret = %d", ret);

	char *wlanacname = "123.0539.0531.000";
	unsigned int acid, acport;
	char acip[256] = {0};
	ret = get_acinfo(wlanacname, &acid, acip, &acport);
	xyprintf(0, "ret = %d", ret);

	return 0;
}


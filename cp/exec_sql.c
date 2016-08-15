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
int get_acinfo(char* acname, unsigned int *acid, char* acip)
{
	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};
	// 查询
	snprintf(sql_str, 1023, "SELECT id, ip FROM ac WHERE wlanacname = '%s'", acname);
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

#if EXEC_SQL_DEBUG
	xyprintf(0, "EXEC_SQL_DEBUG: SELECT id(%u), ip(%s) FROM ac WHERE wlanacname = %s success!", *acid, acip, acname);
#endif

	PQclear(sql_res);
	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
}

// 查询wlanparameter是否存在于wifi_user_wlanparameter
int get_usermac(PGconn* conn, char* sql_str, char* usermac, unsigned int acid, char* wlanparameter)
{
	// 查询
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
		xyprintf(0, "EXEC_SQL_DEBUG: %s success!\n\t\tusermac is %s", usermac);
#endif
	}
	PQclear(sql_res);
	return 0;

SQLED_ERROR:
	return -1;
}

// 用户手机登录信息
int get_wuid_phone(unsigned int s_id, char* phonenum, unsigned int acid, char* wlanparameter, unsigned int *wu_id)
{
	// 建立数据库连接
	PGconn* conn = sql_init();
	if(!conn){
		xyprintf(0, "ERROR:%s %d -- sql init failed!", __FILE__, __LINE__);
		goto ERROR;
	}

	char sql_str[1024] = {0};

	char usermac[256] = {0};
	// 查询wlanparameter是否存在于wifi_user_wlanparameter
	if( get_usermac(conn, sql_str, usermac, acid, wlanparameter) ){
		xyprintf(0, "ERROR:%s %d -- get usermac failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 查询mac是否存在
	snprintf(sql_str, 1023, "SELECT id, phonenum FROM wifi_user WHERE s_id = %u AND mac = '%s'", s_id, usermac);
	PGresult* sql_res;
	if( sql_exec_select(conn, sql_str, &sql_res) ){
		xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
		goto SQLED_ERROR;
	}

	// 判断是否查询到 没有查询到创建新的记录
	int tuples = PQntuples(sql_res);
	if( tuples < 1 ){
		PQclear(sql_res);

#if EXEC_SQL_DEBUG
		xyprintf(0, "EXEC_SQL_DEBUG: %s failed!", sql_str);
#endif
		
		// 没有查询到 添加新的记录
		snprintf(sql_str, 1023, "INSERT INTO wifi_user(s_id, mac, phonenum, created_at, updated_at)"
				" VALUES(%u, '%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
				s_id, usermac, phonenum);
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
		xyprintf(0, "EXEC_SQL_DEBUG: SELECT CURRVAL('user_wifi_user_id_seq')(%u) success!", *wu_id);
#endif
	}
	else {
		PQclear(sql_res);
		*wu_id = atoi(sql_getvalue_string(sql_res, 0, 0));
		// 比较手机号码是否变化，变化则修改
		if( strcmp( phonenum, sql_getvalue_string(sql_res, 0, 1)) ){
			snprintf(sql_str, 1023, "UPDATE wifi_user SET phonenum = '%s' WHERE id = %u",
					phonenum, *wu_id);
			if( sql_exec(conn, sql_str) ){
				xyprintf(0, "ERROR:%s %d -- sql exec select failed!", __FILE__, __LINE__);
				goto SQLED_ERROR;
			}
		}
	}

	sql_destory(conn);
	return 0;
	
SQLED_ERROR:
	sql_destory(conn);
ERROR:
	return -1;
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
	unsigned int acid;
	char acip[256] = {0};
	ret = get_acinfo(wlanacname, &acid, acip);
	xyprintf(0, "ret = %d", ret);

	return 0;
}
#if 0
// 获取ac ip
int get_acinfo(char* acname,unsigned int *acid, char* acip, int acip_len)
{
	xyprintf(0, "EXEC SQL -- select ac info");
	

	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, acid,			20, &handle.sql_err);
	SQLBindCol(handle.sqlstr_handle, 2, SQL_C_CHAR,	 acip,			acip_len, &handle.sql_err);
	SQLBindCol(handle.sqlstr_handle, 3, SQL_C_ULONG, CompanyId,		20, &handle.sql_err);
	SQLBindCol(handle.sqlstr_handle, 4, SQL_C_ULONG, AgentId,		20, &handle.sql_err);
	snprintf(handle.sql_str, 1024, "SELECT TOP 1 id, ip, CompanyId, AgentId FROM tb_AC WHERE name = '%s'", acname);	//获取刚插入记录的主键id号
	
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		goto SQLED_ERR;
	}

	wt_sql_destroy(&handle);
	return 0;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}

// 添加用户信息
int add_user(char* parameter, char* apmac, char* type, char* value, int *id)
{
	xyprintf(0, "EXEC SQL -- add user");
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}

	char s_value[128] = {0};
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, id,		128, &handle.sql_err);
	SQLBindCol(handle.sqlstr_handle, 2, SQL_C_CHAR, &s_value,		128, &handle.sql_err);
		// 使用加密后的用户mac和apmac去查询 openid 或者 mobilenum
	snprintf(handle.sql_str, 1024,
			"SELECT TOP 1 id, %s FROM tb_UserDetail WHERE parameter = '%s'",
			type, parameter);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	
	if( handle.sql_ret == SQL_NO_DATA ){
		// 不存在则插入
		snprintf(handle.sql_str, 1024,
				"INSERT INTO tb_UserDetail(%s, apmac, parameter, mobilemac, act, addtime)"
				" VALUES('%s', '%s', '%s', '%s', 1, GETDATE())",
				type, value, apmac, parameter, parameter);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
		
		snprintf(handle.sql_str, 1024, "SELECT SCOPE_IDENTITY()");	//获取刚插入记录的主键id号
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
		handle.sql_ret = SQLFetch(handle.sqlstr_handle);
		SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);
	}
	else if ( strcmp(value, s_value) ){
		// 存在并不一样则修改
		SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);	//释放游标
		snprintf(handle.sql_str, 1024,
				"UPDATE tb_UserDetail SET %s = '%s' WHERE parameter = '%s'",
				type, value, parameter);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
	}

	wt_sql_destroy(&handle);
	return 0;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}

// 更新用户的mac信息
int update_mac(char* mac, int id)
{
	xyprintf(0, "EXEC SQL -- update mac");
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}

	int sid;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &sid,		128, &handle.sql_err);
		// 查找对应id的mac是否需要更新 使用加密后的用户mac和apmac去查询 openid 或者 mobilenum
	snprintf(handle.sql_str, 1024,
			"SELECT TOP 1 id FROM tb_UserDetail WHERE id = %d AND parameter = mobilemac",
			id);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	
	if( handle.sql_ret != SQL_NO_DATA ){
		// 存在则更新mac
		SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);	//释放游标
		snprintf(handle.sql_str, 1024,
				"UPDATE tb_UserDetail SET mobilemac = '%s' WHERE id = %d",
				mac, id);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
	}

	wt_sql_destroy(&handle);
	return 0;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}

// 用户上线
int user_online(char*apmac, char* parameter)
{
	xyprintf(0, "EXEC SQL -- user online");
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}
	
	// 查找apid
	unsigned int apid;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &apid,		128, &handle.sql_err);
	snprintf(handle.sql_str, 1024,
			"SELECT TOP 1 Id FROM TB_Ap WHERE Mac = '%s'", apmac);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		xyprintf(0, "GET Ap id error -- %s", apmac);
		goto SQLED_ERR;
	}
	SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);

	// 查找shopid
	unsigned int shopid;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &shopid,		128, &handle.sql_err);
	snprintf(handle.sql_str, 1024,
			"SELECT TOP 1 ShopId FROM TB_ApDeploy WHERE ApId = %u", apid);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		xyprintf(0, "GET shop id error -- ap id is %u", apid);
		goto SQLED_ERR;
	}
	SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);

	xyprintf(0, "shopid = %u", shopid);

	// 查找usermac
	char usermac[32] = {0};
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_CHAR, usermac,		32, &handle.sql_err);
	snprintf(handle.sql_str, 1024,
			"SELECT TOP 1 mobilemac FROM tb_UserDetail WHERE parameter = '%s'", parameter);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		xyprintf(0, "GET user mac error -- parameter is %s", parameter);
		goto SQLED_ERR;
	}
	SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);

	xyprintf(0, "usermac = %s", usermac);

	// 插入记录
	snprintf(handle.sql_str, 1024,
			"INSERT INTO Center_MobileVisitAp(ApId, ApMac, ShopId, MobileMac, RSSI, PackageTime, InsertTime)"
			" VALUES(%u, '%s', %u, '%s', 88, GETDATE(), GETDATE())",
			apid, apmac, shopid, usermac);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}

	// 获取当前日期
	char date[32] = {0};
	get_curr_date_str(date);

	sprintf(handle.sql_str,
			"exec usp_http_insertCiscodata @ApMac = '%s', @MsisdnMac = '%s', @Rssi = 88, @PackageTime = '%s'",
			apmac, usermac, date);
	if(wt_sql_exec_stored_procedure(&handle)){
		xyprintf(0, "SQL_ERROR:%s %d -- handle->sql_str: %s", __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}


/*
	snprintf(handle.sql_str, 1024,
			"INSERT INTO tb_VisitFre(shopid, tagmac, frequency, staytime, addtime) "
			"VALUES(%u, '%s', 10, 1500, '2016-7-14')",
			shopid, mac);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
*/
	// TODO
	char usermac_12[16] = {0};
	mac_change_12(usermac_12, usermac);
/*
	snprintf(handle.sql_str, 1024,
			"INSERT INTO tb_MobileInfo(MobileMac, msisdn, weixinID, openID) "
			"VALUES('%s', '%u', '%s%u', '%s%u')",
			usermac_12, shopid, usermac_12, shopid, usermac_12, apid);
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
*/

	snprintf(handle.sql_str, 1024,
			"INSERT INTO tb_Visitlist(shopid, tagMac, Addtime) VALUES(%u, '%s', '%s')",
			shopid, usermac_12, date);
	if( wt_sql_exec(&handle) ){
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

// SQL 测试线程
void* sql_test_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Sql test thread is running!!!");

	char *acap[9][2]	={
		{"0298.0539.531.00", "98:f6:2e:ae:09:70"},
		{"0298.0539.531.00", "98:f6:2e:ae:09:71"},
		{"0298.0539.531.00", "98:f6:2e:ae:09:72"},
		{"0298.0539.531.00", "98:f6:2e:ae:09:73"},
		{"0298.0539.531.00", "98:f6:2e:ae:09:74"},
		{"0298.0539.531.00", "99:f6:2e:ae:09:70"},
		{"0299.0539.531.00", "99:f6:2e:ae:09:70"},
		{"0299.0539.531.00", "99:f6:2e:ae:09:71"},
		{"0299.0539.531.00", "99:f6:2e:ae:09:72"} };

	srand(time(NULL));
	int r, i;
	int count;

	while(1){
		// 获取ac信息
		unsigned int acid = 0, CompanyId = 0, AgentId = 0;
		char acip[32] = {0};
		char acip_len = 32;
		
		int num = rand() % 8;
		xyprintf(0, "acname = %s, apmac = %s", acap[num][0], acap[num][1]);

		if( get_acinfo(acap[num][0], &acid, acip, acip_len, &CompanyId, &AgentId) ){
			xyprintf(0, "ERROR: %s -- %d", __FILE__, __LINE__);
			goto SLEEP;
		}

		xyprintf(0, "GET acid = %d, acip = %s, CompanyId = %d, AgentId = %d",
				acid, acip, CompanyId, AgentId);
	
		char ssid[32] = {0};
		snprintf(ssid, 31, "ssid-%d", count);
		xyprintf(0, "ssid = %s", ssid);

		if ( add_apinfo(acap[num][1], ssid, acap[num][0], acid, CompanyId, AgentId) ){
			xyprintf(0, "ERROR: %s -- %d", __FILE__, __LINE__);
			goto SLEEP;
		}
		
		char parameter[64] = {0};
		for(i = 0; i < 48; i += 2){
			r = rand() % 255;
			snprintf(parameter + i, 3, "%02x", r);
		}
		
		char* type;
		char value[64] = {0};
		r = rand() % 2;
		if(r){
			type = "mobilenum";
			for(i = 0; i < 11; i++){
				if(i == 0){
					value[i] = '1';
				}
				else {
					r = rand() % 10;
					value[i] = '0' + r;
				}
			}
		}
		else {
			type = "openid";
			for(i = 0; i < 32; i+=2){
				r = rand() % 255;
				snprintf(value + i, 3, "%x", r);
			}
		}

		xyprintf(0, "parameter = %s, type = %s, value = %s",
				parameter, type, value);
		
		int userid = 0;

		if( add_user(parameter, acap[num][1], type, value, &userid) ){
			xyprintf(0, "ERROR: %s -- %d", __FILE__, __LINE__);
			goto SLEEP;
		}
	
		xyprintf(0, "GET userid = %d", userid);

		char mac[32] = {0};
		for(i = 0; i < 12; i++){
			r = rand() % 255;
			snprintf(mac + i, 3, "%02x", r);
			i++;
		}
		
		xyprintf(0, "mac = %s", mac);

		if( update_mac(mac, userid) ){
			xyprintf(0, "ERROR: %s -- %d", __FILE__, __LINE__);
			goto SLEEP;
		}
		
		if( user_online(acap[num][1], mac) ){
			xyprintf(0, "ERROR: %s -- %d", __FILE__, __LINE__);
			goto SLEEP;
		}
		
		xyprintf(0, "\n");

SLEEP:
		count++;
		sleep(60);
	}
	
/*

	//获得当前时间 并组装log字符串(时间 设备ip 是否需要错误描述)
	time_t tt = time(0);
	struct tm *ttm = localtime(&tt);
	char buf[256] = { 0 };
	sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",
			ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday,
			ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
	xyprintf(0, "%s",buf);	//在屏幕上打印log
	wt_sql_destroy(handle);
	free(handle);
*/
	pthread_exit(NULL);
ERR:
	pthread_exit(NULL);
}
#endif

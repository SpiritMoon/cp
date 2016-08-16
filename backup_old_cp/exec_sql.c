/*****************************************************
 *
 * 数据库操作函数
 *
 *****************************************************/

#include "header.h"

// 获取ac ip CompanyId AgentId
int get_acinfo(char* acname,unsigned int *acid, char* acip, int acip_len, unsigned int *CompanyId, unsigned int *AgentId)
{
	xyprintf(0, "EXEC SQL -- get ac info");
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}

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

// 添加ap信息
int add_apinfo(char* apmac, char* ssid, char* acname, unsigned int acid, unsigned int CompanyId, unsigned int AgentId)
{
	xyprintf(0, "EXEC SQL -- add ap info");
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}

	// 先查询ap是否存在
	unsigned int apid;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &apid,		20, &handle.sql_err);
	snprintf(handle.sql_str, 1024, "SELECT TOP 1 id FROM TB_Ap WHERE Mac = '%s'", apmac);	//获取刚插入记录的主键id号
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		// 不存插入ap数据
		snprintf(handle.sql_str, 1024, 
				"INSERT INTO TB_Ap(Mac, DeviceType, Statas, AddTime, SsId, deviceNo) VALUES('%s', 0, 1, GETDATE(), '%s', '%s' )",
				apmac, ssid, apmac);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
		
		//获取刚插入记录的主键id号
		snprintf(handle.sql_str, 1024, "SELECT SCOPE_IDENTITY()");
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
		handle.sql_ret = SQLFetch(handle.sqlstr_handle);
		SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);
		xyprintf(0, "Insert a ap record, get id is %u", apid);
	
		// 关联ap
		// TODO ShopId 现在用850 需要修改一下
		snprintf(handle.sql_str, 1024, 
				"INSERT INTO TB_ApDeploy(ApId, ShopId, PositionDesc, AgentId, CompanyId, AcId) VALUES(%u, 850, '%s', %u, %u, %u)",
				apid, acname, AgentId, CompanyId, acid);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
	}
	else {
		SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);
		// 存在更新
		snprintf(handle.sql_str, 1024, "UPDATE TB_Ap SET SsId = '%s' WHERE id = %d", ssid, apid);
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

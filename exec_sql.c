/*****************************************************
 *
 * 数据库操作函数
 *
 *****************************************************/

#include "header.h"

#define SQL_SELECT_ApId_ShopId \
	"SELECT ApId, ShopId FROM TB_ApDeploy WHERE ApId IN (SELECT id FROM TB_Ap WHERE mac = '%s')"

#define SQL_INSERT_Aplogtemp \
	"INSERT INTO Aplogtemp(apid,shopid,ApMac,MobileMac,RSSI,PackageTime,InsertTime) VALUES(%d, %d, '%s', '%s', %d, GETDATE(), GETDATE())"

#define SQL_INSERT_TB_ApDeploy \
	"INSERT INTO TB_ApDeploy(ShopId, ApId, PositionDesc, AgentId, CompanyId) VALUES(%d, %d, '%s', %d, %d)"

#define SQL_INSERT_TB_Ap \
	"INSERT INTO TB_Ap(Mac, DeviceType, Statas, AddTime, SsId, deviceNo) VALUES('%s', 61, 1, GETDATE(), 'ILINYI', '%s' )"

// 获取ac ip
int get_wlanacip(char* acname, char* acip, int acip_len)
{
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}
	
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_CHAR, acip,		acip_len, &handle.sql_err);
	snprintf(handle.sql_str, 1024, "SELECT TOP 1 ip FROM tb_AC WHERE name = '%s'", acname);	//获取刚插入记录的主键id号
	
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

// SQL 测试线程
void* sql_test_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Sql test thread is running!!!");

	
	char *acname = "0298.0539.531.00";
	char acip[64] = {0};
	
	int ret = get_wlanacip(acname, acip, 64);

	xyprintf(0, "ret = %d, acname:%s, acip:%s", ret, acname, acip);

	acname = "123456";
	
	ret = get_wlanacip(acname, acip, 64);

	xyprintf(0, "ret = %d, acname:%s, acip:%s", ret, acname, acip);

	
	
	
	
	
	
/*
	// 数据库访问资源
	wt_sql_handle *handle = malloc(sizeof(wt_sql_handle));		//SELECT用
	memset(handle, 0, sizeof(wt_sql_handle));
	
		
	while( wt_sql_init(handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){			// 数据库初始化
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		sleep( 10 );
	}
		
	sprintf(handle->sql_str, "exec usp_insert_probedata @mobilemac = '%s', @apmac = '%s', @rssi = %d", "b4:0b:44:1a:63:12", "b4:0b:44:1a:63:12", 88);
	if(wt_sql_exec_stored_procedure(handle)){
		xyprintf(0, "SQL_ERROR:%s %d -- handle->sql_str: %s", __FILE__, __LINE__, handle->sql_str);
		goto STR_ERR;
	}

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

/*
int user_online(char* apmac, char* acname, char* usermac)
{
	//数据库操作所需参数
	wt_sql_handle	handle;
	
	//初始化数据库连接
	if( wt_sql_init(&handle, SQL_NAME, SQL_USER, SQL_PASSWD) ){
		xyprintf(0, "SQL_INIT_ERROR:%s %s %d -- Datebase connect error!", __func__, __FILE__, __LINE__);
		goto ERR;
	}
	
	// 先查询apid 和 shopid 有没有
	unsigned int ap_id, shop_id;
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &ap_id,		20, &handle.sql_err);
	SQLBindCol(handle.sqlstr_handle, 1, SQL_C_ULONG, &shop_id,		20, &handle.sql_err);
	snprintf(handle.sql_str, 1024, SQL_SELECT_ApId_ShopId, apmac);	//获取刚插入记录的主键id号
	if( wt_sql_exec(&handle) ){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if( handle.sql_ret == SQL_NO_DATA ){
		// 没有ap数据 插入数据
		snprintf(handle.sql_str, 1024, SQL_INSERT_TB_Ap, apmac, acname);
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
		xyprintf(0, "Insert a ap record, get id is %u", ap_id);
		
		// 给shop_id一个默认值 然后关联ap
		shop_id = 851; // 临沂移动 agentid 94, companyid 275, shopid 851
		snprintf(handle.sql_str, 1024, SQL_INSERT_TB_ApDeploy, shop_id, ap_id, acname, 94, 275);
		if( wt_sql_exec(&handle) ){
			xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
			goto SQLED_ERR;
		}
	}
	// 
	wt_sql_destroy(&handle);
	return 0;
SQLED_ERR:
	wt_sql_destroy(&handle);
ERR:
	return -1;
}
*/

#if 0
	//3.2绑定变量和SQL语句查询结果
	__u32 id, shanghuid, rid, isok, iscandoing, xinghaoid;
	char xinghao[16];
	SQLBindCol( handle.sqlstr_handle, 1, SQL_C_ULONG, &id,				20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 2, SQL_C_ULONG, &shanghuid,		20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 3, SQL_C_ULONG, &rid,				20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 4, SQL_C_ULONG, &isok,			20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 5, SQL_C_ULONG, &iscandoing,		20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 6, SQL_C_ULONG, &xinghaoid,		20, &handle.sql_err);
	SQLBindCol( handle.sqlstr_handle, 7, SQL_C_CHAR,  xinghao,			16, &handle.sql_err);

	//3.3执行SQL语句
	sprintf(handle.sql_str, "SELECT TOP 1 id, shanghuid, rid, \
			isok, iscandoing, xinghaoid, xinghao FROM mx_view_useshebei WHERE Hard_seq = '%s'", msg->hard_seq);
	if(wt_sql_exec(&handle)){
		xyprintf(0, "SQL_ERROR:%s %s %d -- sql string is -- %s", __func__, __FILE__, __LINE__, handle.sql_str);
		goto SQLED_ERR;
	}
	//4.3处理查询结果
	handle.sql_ret = SQLFetch(handle.sqlstr_handle);
	if(handle.sql_ret == SQL_NO_DATA){
		xyprintf(0, "ROUTER_ERROR: %s %d -- Not has router's message in datebase, msg->hard_seq = %s!", __FILE__, __LINE__, msg->hard_seq);
		goto SQLED_ERR;
	}

	if(shanghuid == 0){
		xyprintf(0, "ROUTER_ERROR: %s %d -- Shanghu id is error, shanghuid = %u, msg->hard_seq = %s!", __FILE__, __LINE__, shanghuid, msg->hard_seq);
		goto SQLED_ERR;
	}
	//xyprintf(0, "iscandoing = %u, startdate_res = %d, enddate_res = %u", iscandoing, startdate_res, enddate_res);
	if(iscandoing == 0){
		xyprintf(0, "ROUTER_ERROR: %s %d -- Router not candoing or is expired, shanghuid = %u, msg->hard_seq = %s!", __FILE__, __LINE__, shanghuid, msg->hard_seq);
		goto SQLED_ERR;
	}
	SQLFreeStmt(handle.sqlstr_handle, SQL_CLOSE);
#endif

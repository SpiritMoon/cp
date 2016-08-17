/*****************************************************
 *
 * 
 * 
 *****************************************************/
#include "header.h"

// 将mac地址转换成 aa:aa:aa:aa:aa:aa 的形式
int mac_change_17(char* dest, const char* src)
{
	if(strlen(src) < 17){
		return -1;
	}

	int i = 0;
	for(; i < 17; i++){
		if(i % 3 == 0 || i % 3 == 1){
			dest[i] = tolower(src[i]);
		}
		else{
			dest[i] = ':';
		}
	}
	
//	xyprintf(0, "Change mac success, %s --> %s", src, dest);

	return 0;
}

// 将mac地址转换成 没有冒号  的形式
int mac_change_12(char* dest, char* src)
{
	if(strlen(src) < 17){
		return -1;
	}

	char *buf = src;

	int i = 0;
	while(*buf){
		if(*buf >= '0' && *buf <= '9'){
			dest[i] = *buf;
			i++;
		}
		else if( *buf >= 'a' && *buf <= 'z' ){
			dest[i] = *buf;
			i++;
		}
		else if( *buf >= 'A' && *buf <= 'Z' ){
			dest[i] = *buf + 32;
			i++;
		}
		buf++;
		if( i == 12 ){
			break;
		}
	}

	if( i != 12 ){
		xyprintf(0, "ERROR:change mac12 error, src = %s", src);
		return -1;
	}

//	xyprintf(0, "Change mac success, %s --> %s", src, dest);

	return 0;
}

void get_curr_time_str(char* buf)
{
	//获得当前时间 并组装log字符串(时间 设备ip 是否需要错误描述)
	time_t tt = time(0);
	struct tm *ttm = localtime(&tt);
	sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",
			ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday,
			ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
//	xyprintf(0, "get time success -- %s",buf);	//在屏幕上打印log
}

void get_curr_date_str(char* buf)
{
	//获得当前时间 并组装log字符串(时间 设备ip 是否需要错误描述)
	time_t tt = time(0);
	struct tm *ttm = localtime(&tt);
	sprintf(buf,"%d-%02d-%02d",
			ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday);
//	xyprintf(0, "get date success -- %s",buf);	//在屏幕上打印log
}

// 
int res_username(char* username, int* wu_id, int* login_type)
{
	// username, wu_id-login_type
	*wu_id = atoi(username);
	if( *wu_id <= 0 ){
		xyprintf(0, "ERROR:%s %d -- resolve usernmae error, username is %s!", __FILE__, __LINE__, username);
		return -1;
	}
#if EXEC_SQL_DEBUG
	xyprintf(0, "Get wu_id(%d) success!", *wu_id);
#endif

	char* temp = strstr(username, "-");
	if( !temp ){
		xyprintf(0, "ERROR:%s %d -- resolve usernmae error, username is %s!", __FILE__, __LINE__, username);
		return -1;
	}
	temp++;
	*login_type = atoi(temp);
	if( *login_type <= 0 ){
		xyprintf(0, "ERROR:%s %d -- resolve usernmae error, username is %s!", __FILE__, __LINE__, username);
		return -1;	
	}
#if EXEC_SQL_DEBUG
		xyprintf(0, "Get login_type(%d) success!", *login_type);
#endif

	return 0;
}


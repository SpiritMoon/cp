/*****************************************************
 *
 * 
 * 
 *****************************************************/
#include "header.h"

// 将mac地址转换成 aa:aa:aa:aa:aa:aa 的形式
int mac_change(char* dest, const char* src)
{
	int i = 0;
	int len = strlen(src);
	
	if(len != 17){
		return -1;
	}

	for(; i < len; i++){
		if(i % 3 == 0 || i % 3 == 1){
			dest[i] = tolower(src[i]);
		}
		else{
			dest[i] = ':';
		}
	}
	
	xyprintf(0, "src is %s\n\t\t\tdest is %s", src, dest);

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
	xyprintf(0, "%s",buf);	//在屏幕上打印log
}

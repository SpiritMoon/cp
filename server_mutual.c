/*****************************************************
 *
 * 与web服务器的交互
 *
 *****************************************************/
#include "header.h"

#define SERVER_MUTUAL_DEBUG			1

// 
struct plat_para{
	char* type;					// 报文类型 auth-tel, auth-wx, auth-temp
	char* usernum;				// auth-tel：手机号，auth-wx：微信openid，auth-temp：空
	char* usercode;				// auth-tel：验证码，auth-wx：tid，auth-temp：空
	char* wlanuserip;			// 用户ip
	char* wlanacname;			// ac标识名称
	char* ssid;					// ssid
	char* wlanacip;				// ac ip
	char* apmac;				// ap mac
	char* wlanparameter;		// 加密的mac
	char* wlanuserfirsturl;		// 访问的url
};


/** 
 *@brief  打印struct plat_para的信息
 *@param  
 *@return 
 */
void xyprintf_plat_para(struct plat_para *para)
{
	xyprintf(0, "plat_para->type = %s\n\
			->usernum = %s\n\
			->usercode = %s\n\
			->wlanuserip = %s\n\
			->wlanacname = %s\n\
			->ssid = %s\n\
			->wlanacip = %s\n\
			->apmac = %s\n\
			->wlanparameter = %s\n\
			->wlanuserfirsturl = %s",
			para->type,
			para->usernum,
			para->usercode,
			para->wlanuserip,
			para->wlanacname,
			para->ssid,
			para->wlanacip,
			para->apmac,
			para->wlanparameter,
			para->wlanuserfirsturl
			);
}

/** 
 *@brief  装换获取到的json数据
 *@param  
 *@return 
 */
int get_plat_para(cJSON *json, struct plat_para *para)
{
	cJSON *temp = NULL;
	
	// 获取type
	temp = cJSON_GetObjectItem(json,"type");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->type = temp->valuestring;

	if( strcmp( para->type, "auth-temp" ) ){
	
		// get usernum
		temp = cJSON_GetObjectItem(json,"usernum");
		if (!temp){
			xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
			return -1;
		}
		para->usernum = temp->valuestring;

		// get usercode
		temp = cJSON_GetObjectItem(json,"usercode");
		if (!temp){
			xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
			return -1;
		}
		para->usercode = temp->valuestring;
	}
	else {
		para->usernum = "auth-temp";
		para->usercode = "auth-temp";
	}

	// get wlanuserip
	temp = cJSON_GetObjectItem(json,"wlanuserip");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->wlanuserip = temp->valuestring;

	// get wlanacname
	temp = cJSON_GetObjectItem(json,"wlanacname");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->wlanacname = temp->valuestring;

	// get ssid 
	temp = cJSON_GetObjectItem(json,"ssid");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->ssid = temp->valuestring;

	// get wlanacip 
	temp = cJSON_GetObjectItem(json,"wlanacip");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->wlanacip = temp->valuestring;

	// get apmac
	temp = cJSON_GetObjectItem(json,"apmac");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->apmac = temp->valuestring;

	// get wlanparameter
	temp = cJSON_GetObjectItem(json,"wlanparameter");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->wlanparameter = temp->valuestring;

	// get wlanuserfirsturl 
	temp = cJSON_GetObjectItem(json,"wlanuserfirsturl");
	if (!temp){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		return -1;
	}
	para->wlanuserfirsturl = temp->valuestring;

	return 0;
}

/** 
 *@brief  平台命令处理函数 在platform_fun内被调用
 *@param  fd		类型 void*				连接设备的sockfd
 *@return 无意义
 */
void* platform_process(void *fd)
{
	pthread_detach(pthread_self());
	
	int			sockfd	= (int)(long)fd;		//64bits下 void* 要先转换成long 然后再转换成int
	int			ret = 0;
	int			i;

	//报文体存放位置
	char buf[1024] = { 0 };

	//接收
	if( recv(sockfd, buf, 1024, 0) <= 0){
		xyprintf(0, "PLATFORM_ERROR:%d %s %d -- Recv platform's massage error!", sockfd, __FILE__, __LINE__);
		goto DATA_ERR;
	}

#if SERVER_MUTUAL_DEBUG
	//晒一下
	xyprintf(0, "PLATFORM:Platform's msg: %s", buf);
#endif
	
	// 主体json
	cJSON *json=cJSON_Parse( buf );
	if (!json){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto DATA_ERR;
	}

	// 获取json内容
	struct plat_para para;
	memset(&para, 0, sizeof(para));
	if( get_plat_para(json, &para) ){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto DATA_ERR;
	}

#if SERVER_MUTUAL_DEBUG
	xyprintf_plat_para(&para);
#endif

	if(!strcmp(para.type, "auth-tel")){
		//TODO sql	
	
	}
	else if(!strcmp(para.type, "auth-wx")){
		//TODO sql	
	
	}
	else if(!strcmp(para.type, "auth-temp")){
		//TODO sql	
	
	}
	else {
		xyprintf(0,"PLATFORM_ERROR:Type unknown(%s) -- %s -- %d!!!", para.type, __FILE__, __LINE__);
		goto DATA_ERR;
	}

	// 准备发送数据到ac
	ST_REQ_AUTH req_auth;
	memset(&req_auth, 0 , sizeof(req_auth));
	strcpy(req_auth.userip, para.wlanuserip);
	strcpy(req_auth.name, para.usernum);
	strcpy(req_auth.password, para.usercode);

	char *res;
/*
	if( SendReqAuthAndRecv(&req_auth, para.wlanacip, PORTAL_TO_AC_PORT ) ){
		res = "{\"stat\":\"failed\"}";
	}
	else {
		res = "{\"stat\":\"ok\"}";
	}
*/
	
	//res = "{\"stat\":\"ok\"}";
	res = "{\"stat\":\"e8:4e:06:2e:5a:8b\"}";

	// 发送返回值
	xyprintf(0, "** res -- %d -- %s", strlen(res), res);
	if( send(sockfd, res, strlen(res), 0) <= 0){
		xyprintf(0, "PLATFORM_ERROR:%d %s %d -- Res platform's massage error!", sockfd, __FILE__, __LINE__);
		goto JSON_ERR;
	}
	
	cJSON_Delete(json);
	wt_close_sock( &sockfd );
	pthread_exit(NULL);

	//错误处理 使用内核中常用的goto模式～
JSON_ERR:
	cJSON_Delete(json);
DATA_ERR:
	wt_close_sock( &sockfd );
	pthread_exit(NULL);
} 

/** 
 *@brief  平台连接监听线程函数
 *@param  fd		类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* platform_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Platform connection thread is running!!!");
	while(1){
		int sockfd;
		if( wt_sock_init( &sockfd, TO_PLATFORM_PORT, MAX_EPOLL_NUM) ){		//初始化监听连接
			xyprintf(errno, "PLATFORM_ERROR:0 %s %d -- wt_sock_init()", __FILE__, __LINE__);
			continue;
		}
		struct sockaddr_in client_address;				//存放客户端地址信息
		int client_len = sizeof(client_address);		//存放客户端地址信息结构体长度
		
		while(1){										//开始循环监听
			int client_sockfd;
			client_sockfd = accept(sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
			
			if(client_sockfd == -1){					//监听出错
				xyprintf(errno, "PLATFORM_ERROR:%d %s %d -- accept()", sockfd, __FILE__, __LINE__);
				break;
			}
			
#if SERVER_MUTUAL_DEBUG
			//监听到一个连接 先打印一下
			xyprintf(0, "PLATFORM:O(∩ _∩ )O ~~ platform %s connection, sockfd is %d", inet_ntoa( client_address.sin_addr) , client_sockfd);
#endif
			
			pthread_t thread;//创建线程维护与第三方监听程序的连接 
			if(pthread_create(&thread, NULL, platform_process, (void*)(long)client_sockfd) != 0){//创建子线程
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				break;
			}
		}
		close(sockfd);									//如果出错了 就关掉连接 重新初始化
	}

	//到不了的地方～～～
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Platform pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

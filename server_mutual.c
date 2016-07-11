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
	unsigned int acid;
	unsigned int CompanyId;
	unsigned int AgentId;
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
			->wlanuserfirsturl = %s\n\
			->acid = %u\n\
			->CompanyId = %u\n\
			->AgentId = %u\n",
			para->type,
			para->usernum,
			para->usercode,
			para->wlanuserip,
			para->wlanacname,
			para->ssid,
			para->wlanacip,
			para->apmac,
			para->wlanparameter,
			para->wlanuserfirsturl,
			para->acid,
			para->CompanyId,
			para->AgentId
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
	struct plat_para para = {0};
	memset(&para, 0, sizeof(para));
	if( get_plat_para(json, &para) ){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}

	// 如果没有ACip 取查询数据库
	char acip[64] = {0};
	if( get_acinfo(para.wlanacname, &(para.acid), acip, 64, &(para.CompanyId), &(para.AgentId) ) ){
		xyprintf(0,"PLATFORM_ERROR:Get acinfo error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	
	if( !strlen(para.wlanacip) ){
		para.wlanacip = acip;
		//para.wlanacip = "223.99.130.172";
	}

#if SERVER_MUTUAL_DEBUG
	xyprintf_plat_para(&para);
#endif

	char res[128] = {0};
	snprintf(res, 127, "{\"stat\":\"failed\"}");

	if( !strlen(para.wlanacip) ){
	}
	else if(!strcmp(para.type, "auth-tel") ){
		//sql 查询数据库对应id值
		int id;
		if( add_user(para.wlanparameter, para.apmac, "mobilenum", para.usernum, &id) ){
			xyprintf(0, "ERROR %s -- %d", __FILE__, __LINE__);
			goto JSON_ERR;
		}
		
#if SERVER_MUTUAL_DEBUG
	xyprintf(0, "Get user id %d", id);
#endif
		// 准备发送数据到ac
		char username[128] = {0};
		snprintf(username, 127, "%s-%u@ilinyi", &para.type[5], id);
		if( !SendReqAuthAndRecv(para.wlanuserip, username, "123456", para.wlanacip, PORTAL_TO_AC_PORT ) ){
			snprintf(res, 127, "{\"stat\":\"ok\"}");
		}
	}
	else if(!strcmp(para.type, "auth-white")) {
		// 准备发送数据到ac
		int id = ((int)time(0)) % 10000000 + 10000000;
		char username[128] = {0};
		snprintf(username, 127, "%s-%u@ilinyi", &para.type[5], id);
		if( !SendReqAuthAndRecv(para.wlanuserip, username, "123456", para.wlanacip, PORTAL_TO_AC_PORT ) ){
			snprintf(res, 127, "{\"stat\":\"ok\"}");
		}
	}
	else if(!strcmp(para.type, "auth-temp") ){
		unsigned int id;
		//根据wlanparameter查找是否存在对应用户 插入临时放行表 获取临时表id
		int ret = insert_discharged(para.wlanuserip, para.wlanacip);
		if(ret > 0){
			id = ret;
			xyprintf(0, "Get temp id is %u", id);
		}
		else {
			xyprintf(0, "%s - %s - %d ERROR!", __FILE__, __func__, __LINE__);
			goto JSON_ERR;
		}
		
		// 准备发送数据到ac
		char username[128] = {0};
		snprintf(username, 127, "%s-%u@ilinyi", &para.type[5], id);
	
		//if( !SendReqAuthAndRecv(para.wlanuserip, username, "123456", para.wlanacip, PORTAL_TO_AC_PORT ) ){
		if( SendReqAuthAndRecv(para.wlanuserip, username, "123456", para.wlanacip, PORTAL_TO_AC_PORT ) ){
			// 获取radius获取到的mac地址
			int i = 0;
			int find_flag = -1;
			char usermac[64] = {0};
		// TODO test			
			snprintf(res, 127, "{\"stat\":\"ok\"}");
			snprintf(res, 127, "{\"stat\":\"b4:0b:44:1a:63:12\"}");
			
			for(; i < 10; i++){
				if(!user_mp_list_find_and_del(id, usermac)){
					xyprintf(0, "get mac success -- %s", usermac);
					snprintf(res, 127, "{\"stat\":\"%s\"}", usermac);
					find_flag = 0;
					break;
				}
				usleep(100);
				xyprintf(0, "Can not find usermac, sleep 100 us continue!");
			}
		}
	}
	else if(!strcmp(para.type, "auth-wx")){
		//sql 查询数据库对应id值
		int id;
		add_user(para.wlanparameter, para.apmac, "openid", para.usernum, &id);
		
		int ret = delete_discharged(para.wlanuserip, para.wlanacip);
		if(ret < 0){
			xyprintf(0, "%s - %s - %d ERROR!", __FILE__, __func__, __LINE__);
			goto JSON_ERR;
		}
		
		snprintf(res, 127, "{\"stat\":\"ok\"}");
	}
	else {
		xyprintf(0,"PLATFORM_ERROR:Type unknown(%s) -- %s -- %d!!!", para.type, __FILE__, __LINE__);
		goto JSON_ERR;
	}

	
	// 发送返回值
	xyprintf(0, "** res -- %d -- %s", strlen(res), res);
	if( send(sockfd, res, strlen(res), 0) <= 0){
		xyprintf(0, "PLATFORM_ERROR:%d %s %d -- Res platform's massage error!", sockfd, __FILE__, __LINE__);
		goto JSON_ERR;
	}

	// 更新AP信息
	add_apinfo(para.apmac, para.ssid, para.wlanacname, para.acid, para.CompanyId, para.AgentId);

	
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

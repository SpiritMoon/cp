/*****************************************************
 *
 * 引导服务器 与 其他服务器的交互
 *
 *****************************************************/
#include "header.h"

#define SERVER_MUTUAL_DEBUG			1

/** 
 *@brief  平台命令处理函数 在platform_fun内被调用
 *@param  fd		类型 void*				连接设备的sockfd
 *@return 无意义
 */
void* platform_process(void *fd)
{
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

	//比较是否是json头部
	//if(strncmp( buf,"php-stream:",11)) {
	//	xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
	//	goto DATA_ERR;
	//}
	
	//解析json用到的临时变量
	cJSON *json = NULL;

	// 主体json
	json=cJSON_Parse( buf );
	if (!json){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto DATA_ERR;
	}

	cJSON *json_wlanuserip = NULL;
	json_wlanuserip = cJSON_GetObjectItem(json,"wlanuserip");
	if (!json_wlanuserip){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_wlanuserip : %s", json_wlanuserip->valuestring);

	cJSON *json_wlanacname = NULL;
	json_wlanacname = cJSON_GetObjectItem(json,"wlanacname");
	if (!json_wlanacname){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_wlanacname : %s", json_wlanacname->valuestring);

	cJSON *json_ssid = NULL;
	json_ssid = cJSON_GetObjectItem(json,"ssid");
	if (!json_ssid){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_ssid : %s", json_ssid->valuestring);

	cJSON *json_wlanparameter = NULL;
	json_wlanparameter = cJSON_GetObjectItem(json,"wlanparameter");
	if (!json_wlanparameter){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_wlanparameter : %s", json_wlanparameter->valuestring);

	cJSON *json_acip = NULL;
	json_acip = cJSON_GetObjectItem(json,"wlanacip");
	if (!json_acip){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_acip : %s", json_acip->valuestring);

	cJSON *json_usernum = NULL;
	json_usernum = cJSON_GetObjectItem(json,"usernum");
	if (!json_usernum){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_usernum : %s", json_usernum->valuestring);

	cJSON *json_usercode = NULL;
	json_usercode = cJSON_GetObjectItem(json,"usercode");
	if (!json_usercode){
		xyprintf(0,"PLATFORM_ERROR:JSON's data error -- %s -- %d!!!", __FILE__, __LINE__);
		goto JSON_ERR;
	}
	xyprintf(0,"json_usercode : %s", json_usercode->valuestring);

	ST_REQ_AUTH req_auth;
	strcpy(req_auth.userip, json_wlanuserip->valuestring);
	strcpy(req_auth.name, json_usernum->valuestring);
	strcpy(req_auth.password, json_usercode->valuestring);


	if( SendReqAuthAndRecv(&req_auth, json_acip->valuestring, PORTAL_TO_AC_PORT ) ){

	}




	// 发送返回值
	char *res = "{\"stat\":\"ok\"}";
	xyprintf(0, "** res -- %d -- %s", strlen(res), res);
	if( send(sockfd, res, strlen(res), 0) <= 0){
		xyprintf(0, "PLATFORM_ERROR:%d %s %d -- Res platform's massage error!", sockfd, __FILE__, __LINE__);
		goto JSON_ERR;
	}
	
	cJSON_Delete(json);
	wt_close_sock( &sockfd );
	return (void*)0;

	//错误处理 使用内核中常用的goto模式～
JSON_ERR:
	cJSON_Delete(json);
DATA_ERR:
	wt_close_sock( &sockfd );
	xyprintf(0, "PLATFORM_ERROR:%s %s %d -- Request pthread is unnatural deaths!!!", __func__, __FILE__, __LINE__);
	return (void*)0;
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
		if( wt_sock_init( &sockfd, cgv_platform_port, MAX_EPOLL_NUM) ){		//初始化监听连接
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

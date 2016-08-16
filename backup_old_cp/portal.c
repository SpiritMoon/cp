/*****************************************************
 *
 * Protal 服务器端
 *
 *****************************************************/
#include "header.h"

#define PORTAL_DEBUG 1
#define PORTAL_RECV_PORT	50100

static int				portal_recv_sockfd		= 0;	// portal recv socket
static pthread_mutex_t	portal_recv_sockfd_lock;		// radius socket 互斥锁

/** 
 *@brief  打印ST_PORTAL_AC的信息
 *@param  
 *@return 
 */
void xyprintf_portal_ac(ST_PORTAL_AC* pa)
{
	xyprintf(0, "ST_PORTAL_AC->ver = 0x%x\n\
			->type = 0x%x\n\
			->pap_chap = 0x%x\n\
			->rsv = 0x%0x\n\
			->serialNo = %u\n\
			->reqID = %u\n\
			->userIP = %u\n\
			->userPort = %u\n\
			->errCode = 0x%02x\n\
			->attrNum = 0x%02x\n",
			pa->ver,
			pa->type,
			pa->pap_chap,
			pa->rsv,
			pa->serialNo,
			pa->reqID,
			pa->userIP,
			pa->userPort,
			pa->errCode,
			pa->attrNum);
	ST_PORTAL_AC_ATTR *temp = (ST_PORTAL_AC_ATTR*)pa->ac_attr;
	char buf[256];
	int i = 0;
	for(; i < pa->attrNum; i++){
		memset(buf, 0, 256);
		memcpy(buf, temp->attrVal, temp->len -2);
		xyprintf(0, "ST_PORTAL_AC_ATTR->type = 0x%x\n\
				->len = %u\n\
				->attrVal = %s",
				temp->type,
				temp->len,
				buf);
		temp = (ST_PORTAL_AC_ATTR*)((char*)temp + temp->len);
	}
}

/** 
 *@brief  获取验证码
 *@param  
 *@return 
 */
unsigned short generateSerialNo()
{
    unsigned short retNo = 0;
	srand((unsigned)time(NULL));
	int randval = rand();
	retNo = randval & 0xffff;

	return retNo;
}

/** 
 *@brief  发送认证报文
 *@param  
 *@return 
 */
int SendReqAuthAndRecv(char* userip, char* usernum, char* usercode, char* ac_ip, int port)
{
    int sockfd = 0;
    if (UDP_create(&sockfd)){
		xyprintf(0, "ERROR - %s - %s - %d - create udp socket failed!", __FILE__, __func__, __LINE__);
        return -1;
    }
    
    ST_PORTAL_AC stReqAuth;
    stReqAuth.ver = 0x01;
    stReqAuth.type = 0x03;
    stReqAuth.pap_chap = 0x01;
    stReqAuth.rsv = 0x00;
    stReqAuth.serialNo = generateSerialNo();
    stReqAuth.reqID = 0x0000;
    inet_pton(AF_INET, userip, (void*)&stReqAuth.userIP);
//	stReqAuth.userIP = htonl(stReqAuth.userIP);	// Portal用户的IP地址，长度为 4 字节
	stReqAuth.userPort = 0x0000;		// 长度为 2 字节，在所有报文中其值为0
	stReqAuth.errCode = 0x00;
	stReqAuth.attrNum = 2;

	ST_PORTAL_AC_ATTR reqAuthAttr[2] = { 0 };
	// 组属性username
	reqAuthAttr[0].type = UserName;
	reqAuthAttr[0].len = 2;
	reqAuthAttr[0].len += strlen( (char*)usernum );
	strncpy((char*)reqAuthAttr[0].attrVal, usernum, sizeof(reqAuthAttr[0].attrVal)-1);

	// 打包password
	reqAuthAttr[1].type = PassWord;
	reqAuthAttr[1].len  = 2; 
	reqAuthAttr[1].len += strlen( usercode );
	strncpy( (char*)reqAuthAttr[1].attrVal, usercode, sizeof(reqAuthAttr[1].attrVal)-1); 
	
	// 10.REQ_AUTH 组大包 

	memcpy(stReqAuth.ac_attr, reqAuthAttr, reqAuthAttr[0].len);
	memcpy(stReqAuth.ac_attr + reqAuthAttr[0].len, &reqAuthAttr[1], reqAuthAttr[1].len);

	xyprintf(0, "**** send portal auth bag to %s:%d!", ac_ip, port);
	xyprintf(0, "UserName: %s, Password %s", usernum, usercode);
	//xyprintf_portal_ac(&stReqAuth);

	// 10.REQ_AUTH
	if (UDP_send_block(sockfd, ac_ip, port, (unsigned char *)&stReqAuth, 16+reqAuthAttr[0].len+reqAuthAttr[1].len) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

    ST_PORTAL_AC stAckAuth;
	
	// 13.ACK_AUTH
	if( UDP_recv_block(sockfd, (unsigned char*)&stAckAuth, sizeof (stAckAuth)) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - recv udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

	xyprintf(0, "**** recv portal auth req bag, errCode = %d!", stReqAuth.errCode);
//	xyprintf_portal_ac(&stAckAuth);

  
	if(stAckAuth.errCode == 0){
		// 中兴设备需要portal返回ac确认信息
		stReqAuth.type = 0x07;
		stReqAuth.attrNum = 0;
		
		xyprintf(0, "**** send  portal auth bag to %s:%d, type is 0x7!", ac_ip, port);
		//xyprintf_portal_ac(&stReqAuth);

		// 10.REQ_AUTH
		if (UDP_send_block(sockfd, ac_ip, port, (unsigned char *)&stReqAuth, 16) < 0){
			xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
			close(sockfd);
			return -1;
		}
	
		close(sockfd);
		return 0;
	}
	else {
		close(sockfd);
		return -1;
	}
}

/** 
 *@brief  发送登出报文
 *@param  
 *@return 
 */
int SendReqLogoutAndRecv(char* userip, char* ac_ip, int port)
{
    int sockfd = 0;
    if (UDP_create(&sockfd)){
		xyprintf(0, "ERROR - %s - %s - %d - create udp socket failed!", __FILE__, __func__, __LINE__);
        return -1;
    }
    
    // 2.REQ_LOGOUT 组包	
    ST_PORTAL_AC stReqLogout;
    stReqLogout.ver = 0x01;
    stReqLogout.type = 0x05;
    stReqLogout.pap_chap = 0x01;
    stReqLogout.rsv = 0x00;
    stReqLogout.serialNo = generateSerialNo();
    stReqLogout.reqID = 0x0000;
    inet_pton(AF_INET, userip, (void*)&stReqLogout.userIP);
//	stReqLogout.userIP = htonl(stReqLogout.userIP);	// Portal用户的IP地址，长度为 4 字节
	stReqLogout.userPort = 0x0000;		// 长度为 2 字节，在所有报文中其值为0
	stReqLogout.errCode = 0;
	stReqLogout.attrNum = 0;

	xyprintf_portal_ac(&stReqLogout);

	if (UDP_send_block(sockfd, ac_ip, port, (unsigned char *)&stReqLogout, 16) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}
	xyprintf(0, "send logout to %s:%d success, userip is %s", ac_ip, port, userip);
   /* 
	// TODO 中兴设备不返回信息
    ST_PORTAL_AC stAckLogout;
	// 5.ACK_LOGOUT
	if( UDP_recv_block(sockfd, (unsigned char*)&stAckLogout, sizeof (stAckLogout)) < 0 ) {
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

	xyprintf_portal_ac(&stAckLogout);
*/
	close(sockfd);
   /* 
	if(stAckLogout.errCode == 0){
		return 0;
	}
	else {
		return -1;
	}
	*/
	return 0;
}


/** 
 *@brief  测试线程
 *@param  
 *@return 
 */
void* portal_test_thread(void* fd)
{
	pthread_detach(pthread_self());
	sleep(1);
	xyprintf(0, "protal test thread is working!!");
	
	SendReqLogoutAndRecv("10.221.144.42", "223.99.130.172", 2000);
	
	/*
	if( SendReqAuthAndRecv("10.187.226.4", "18866120427", "123456", "111.17.237.28", PORTAL_TO_AC_PORT ) ){
		xyprintf(0, "stat: failed");
	}
	else{
		xyprintf(0, "stat: ok");
	}
	*/
		
	sleep(180);
	
	pthread_exit(NULL);
}

/** 
 *@brief  radius数据包处理
 *@param
 *@return
 */
void* portal_pro_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Protal process thread is running!!!");
	
	struct portal_recv *pr = (struct portal_recv*)fd;
	struct portal_ac  *pa = (struct portal_ac*)(pr->buf);

#if PORTAL_DEBUG
	xyprintf(0, "pr = %p, pr->recv_ret = %d", pr, pr->recv_ret);
	xyprintf_portal_ac(pa);
	/*
	2016-07-20 18:12:39 -- recv a portal msg set in 0x7fb5240008e0, ret is 16
	2016-07-20 18:12:39 -- ** O(∩ _∩ )O ~~ Protal process thread is running!!!
	2016-07-20 18:12:39 -- pr = 0x7fb5240008e0, pr->recv_ret = 16
	2016-07-20 18:12:39 -- ST_PORTAL_AC->ver = 0x1
									->type = 0x8
									->pap_chap = 0x1
									->rsv = 0x0
									->serialNo = 0
									->reqID = 0
									->userIP = 647027978
									->userPort = 0
									->errCode = 0x00
									->attrNum = 0x00
	*/
#endif
	
	//TODO 数据库操作
	
	// 回复报文


	free(pr);
	pthread_exit(NULL);
DATA_ERR:
	free(pr);
	xyprintf(0, "** O(∩ _∩ )O ~~ Portal process thread is end!!!");
	pthread_exit(NULL);
}

/** 
 *@brief  radius服务器监听线程
 *@param  fd类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* portal_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	xyprintf(0, "** O(∩ _∩ )O ~~ Portal connection thread is running!!!");
	
	pthread_mutex_init(&portal_recv_sockfd_lock, 0);
	
	pthread_t pt;
	
	while(1){
		// 初始化socket
		if((portal_recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		// 创建绑定端口信息 并绑定端口
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons( PORTAL_RECV_PORT );
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(portal_recv_sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(portal_recv_sockfd);
			portal_recv_sockfd = 0;
			sleep(10);
			break;
	    } 
		
		xyprintf(0, "UDP socket Ready!!! port is %d!", PORTAL_RECV_PORT);
		
		// 循环接收信息
		while(1){

			// 客户端信息
			struct portal_recv *recv_temp = malloc( sizeof(struct portal_recv) );
			memset(recv_temp, 0, sizeof( struct portal_recv ) );
			recv_temp->addrlen = sizeof( recv_temp->client );
			
			// 接收信息
			recv_temp->recv_ret = recvfrom(portal_recv_sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &(recv_temp->addrlen) );
			
			// 判断返回值
			if (recv_temp->recv_ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				free(recv_temp);
				break;
			}

			xyprintf(0, "recv a portal msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
		
			// 创建线程处理
			if( pthread_create(&pt, NULL, portal_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}

        }

		// 关闭socket 等待重新创建
		close(portal_recv_sockfd);
		portal_recv_sockfd = 0;
	}

	//到不了的地方～～～
	pthread_mutex_destroy(&portal_recv_sockfd_lock);
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Portal pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

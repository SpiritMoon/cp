#include "header.h"

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

	//xyprintf(0, "UserName: %s ,len= %d", usernum, reqAuthAttr[0].len);

	// 打包password
	reqAuthAttr[1].type = PassWord;
	reqAuthAttr[1].len  = 2; 
	reqAuthAttr[1].len += strlen( usercode );
	strncpy( (char*)reqAuthAttr[1].attrVal, usercode, sizeof(reqAuthAttr[1].attrVal)-1); 
	
	//xyprintf(0, "password: %s ,len = %d", usercode, reqAuthAttr[1].len);
	// 10.REQ_AUTH 组大包 

	memcpy(stReqAuth.ac_attr, reqAuthAttr, reqAuthAttr[0].len);
	memcpy(stReqAuth.ac_attr + reqAuthAttr[0].len, &reqAuthAttr[1], reqAuthAttr[1].len);

	xyprintf(0, "**** send portal auth bag!");
	xyprintf_portal_ac(&stReqAuth);

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

	xyprintf(0, "recv portal auth req bag!");
	xyprintf_portal_ac(&stAckAuth);

	close(sockfd);
  
	if(stAckAuth.errCode == 0){
		return 0;
	}
	else {
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

	if (UDP_send_block(sockfd, ac_ip, port, (unsigned char *)&stReqLogout, sizeof (stReqLogout)) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}
	xyprintf(0, "send success, port:%d, ac_ip:%s\n",port, ac_ip);
    
	//11.12 AC向radius发起认证

    ST_PORTAL_AC stAckLogout;
	// 5.ACK_LOGOUT
	if( UDP_recv_block(sockfd, (unsigned char*)&stAckLogout, sizeof (stAckLogout)) < 0 ) {
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

	xyprintf_portal_ac(&stAckLogout);

	close(sockfd);
    
	if(stAckLogout.errCode == 0){
		return 0;
	}
	else {
		return -1;
	}
}


/** 
 *@brief  测试线程
 *@param  
 *@return 
 */
void* portal_test_thread(void* fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "protal test thread is working!!");
	while(1){
		if( SendReqLogoutAndRecv("10.187.226.4", "111.17.237.28", PORTAL_TO_AC_PORT) ){
			xyprintf(0, "stat: failed");
		}
		else{
			xyprintf(0, "stat: ok");
		}
		
		/*
		if( SendReqAuthAndRecv("10.187.226.4", "18866120427", "123456", "111.17.237.28", PORTAL_TO_AC_PORT ) ){
			xyprintf(0, "stat: failed");
		}
		else{
			xyprintf(0, "stat: ok");
		}
		*/
		
		sleep(180);
	}
	pthread_exit(NULL);
}

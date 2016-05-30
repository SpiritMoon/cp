#include "header.h"

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
			->attrNum = 0x%02x",
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
		xyprintf(0, "temp = %p", temp);
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



int udpSendTo(int socket, char* ip, int port, unsigned char *buf, int len)
{
	struct sockaddr_in s_addr;
	socklen_t addr_len = sizeof(s_addr);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = inet_addr(ip);

	if( sendto(socket, buf, len, 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) <= 0){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}

	return 0;
}

int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen)
{
	time_t tFirstTime = 0;

	int slRead = 0;
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(cli_addr);
	int count = 0;

	while(1){
		slRead = recvfrom(socket, rcvBuf, slLen, MSG_DONTWAIT, (struct sockaddr *)&cli_addr, &addr_len);
		if (slRead == -1){
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)){
				xyprintf(0,"no msg recv, sleep 1s continue!");
				sleep(1);
				count++;
				if(count >= 10){
					return -1;
				}
				continue;
			}
			else {
				xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
				return -1;
			}
		}
		else{
			break;
		}
	}

	return 0;
}

int CreateUDPSock(int *sockfd)
{
	if((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}

    //int reuse = 1;
    //setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	int on=1;
	setsockopt(*sockfd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on));
    
	struct timeval r;
    r.tv_sec = 5;
    setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
	setsockopt(*sockfd, SOL_SOCKET, SO_SNDTIMEO, &r, sizeof(r));

	return 0;
}

unsigned short generateSerialNo()
{
    unsigned short retNo = 0;
	srand((unsigned)time(NULL));
	int randval = rand();
	retNo = randval & 0xffff;

	return retNo;
}

int SendReqAuthAndRecv(ST_REQ_AUTH *req_auth, char* ac_ip, int port)
{
    int sockfd = 0;
    if (CreateUDPSock(&sockfd)){
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
    inet_pton(AF_INET,req_auth->userip,(void*)&stReqAuth.userIP);
//	stReqAuth.userIP = htonl(stReqAuth.userIP);	// Portal用户的IP地址，长度为 4 字节
	stReqAuth.userPort = 0x0000;		// 长度为 2 字节，在所有报文中其值为0
	stReqAuth.errCode = 0x00;
	stReqAuth.attrNum = 2;

	ST_PORTAL_AC_ATTR reqAuthAttr[2] = { 0 };
	// 组属性username
	reqAuthAttr[0].type = UserName;
	reqAuthAttr[0].len = 2;
	reqAuthAttr[0].len += strlen( (char*)req_auth->name );
	strncpy((char*)reqAuthAttr[0].attrVal, req_auth->name, sizeof(reqAuthAttr[0].attrVal)-1);

	xyprintf(0, "UserName: %s ,len= %d", req_auth->name, reqAuthAttr[0].len);

	// 打包password
	reqAuthAttr[1].type = PassWord;
	reqAuthAttr[1].len  = 2; 
	reqAuthAttr[1].len += strlen( req_auth->password );
	strncpy( (char*)reqAuthAttr[1].attrVal, req_auth->password, sizeof(reqAuthAttr[1].attrVal)-1); 
	
	xyprintf(0, "password: %s ,len = %d", req_auth->password, reqAuthAttr[1].len);

	// 10.REQ_AUTH 组大包 

	memcpy(stReqAuth.ac_attr, reqAuthAttr, reqAuthAttr[0].len);
	memcpy(stReqAuth.ac_attr + reqAuthAttr[0].len, &reqAuthAttr[1], reqAuthAttr[1].len);

	xyprintf_portal_ac(&stReqAuth);

	// 10.REQ_AUTH
	if (udpSendTo(sockfd, ac_ip, port, (unsigned char *)&stReqAuth, 16+reqAuthAttr[0].len+reqAuthAttr[1].len) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

    ST_PORTAL_AC stAckAuth;
	
	// 13.ACK_AUTH
	if( udpReadFrom(sockfd, (unsigned char*)&stAckAuth, sizeof (stAckAuth)) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}

	xyprintf_portal_ac(&stAckAuth);

	close(sockfd);
  
	if(stAckAuth.errCode == 0){
		return 0;
	}
	else {
		return -1;
	}
}

int SendReqLogoutAndRecv(char* userip, char* ac_ip, int port)
{
    int sockfd = 0;
    if (CreateUDPSock(&sockfd)){
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
    inet_pton(AF_INET,userip,(void*)&stReqLogout.userIP);
//	stReqLogout.userIP = htonl(stReqLogout.userIP);	// Portal用户的IP地址，长度为 4 字节
	stReqLogout.userPort = 0x0000;		// 长度为 2 字节，在所有报文中其值为0
	stReqLogout.errCode = 0;
	stReqLogout.attrNum = 0;

	xyprintf_portal_ac(&stReqLogout);

	if (udpSendTo(sockfd, ac_ip, port, (unsigned char *)&stReqLogout, sizeof (stReqLogout)) < 0){
		xyprintf(0, "ERROR - %s - %s - %d - send udp failed!", __FILE__, __func__, __LINE__);
		close(sockfd);
		return -1;
	}
	xyprintf(0, "send success, port:%d, ac_ip:%s\n",port, ac_ip);
    
	//11.12 AC向radius发起认证

    ST_PORTAL_AC stAckLogout;
	// 5.ACK_LOGOUT
	if( udpReadFrom(sockfd, (unsigned char*)&stAckLogout, sizeof (stAckLogout)) < 0 ) {
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


void* protal_test_thread(void* fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "protal test thread is working!!");
	while(1){
		ST_REQ_AUTH req_auth;
		strcpy(req_auth.userip, "10.187.226.4");
		strcpy(req_auth.name, "18866120427");
		strcpy(req_auth.password, "123456");
		
		if( SendReqAuthAndRecv(&req_auth, "111.17.237.28", PORTAL_TO_AC_PORT ) ){
			xyprintf(0, "stat: failed");
		}
		else{
			xyprintf(0, "stat: ok");
		}
		sleep(60);
	}
	pthread_exit(NULL);
}

/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	Portal.cpp

作者    :	耿斌   版本: v1.0    创建日期: 2015-04-08

功能描述:	处理portal协议接收和发送

其他说明:

修改记录:
*****************************************************************/
#include "Portal.h"
#include <openssl/md5.h>
#include <arpa/inet.h>
int g_ACPort = 11111;
int SendReqAuthAndRecv(const ST_REQ_AUTH& req_auth, const char* ac_ip, const int port)
{
    char logBuf[256] = { 0 };
    ST_PORTAL_AC stAckAuth;
    int sockfd = 0;
    if (!CreateUDPSock(sockfd))
    {
        g_Log.Add("CreateUDPSock error");
        return -1;
    }
    
    ST_PORTAL_AC stReqAuth;
    stReqAuth.ver = 0x01;
    stReqAuth.type = 0x03;
    stReqAuth.pap_chap = 0x01;
    stReqAuth.rsv = 0x00;
    stReqAuth.serialNo = generateSerialNo();
    stReqAuth.reqID = 0x0000;
    inet_pton(AF_INET,req_auth.userip,(void*)&stReqAuth.userIP);
//	stReqAuth.userIP = htonl(stReqAuth.userIP);	// Portal用户的IP地址，长度为 4 字节
	stReqAuth.userPort = 0x0000;		// 长度为 2 字节，在所有报文中其值为0
	stReqAuth.errCode = 0x00;
	stReqAuth.attrNum = 2;

	ST_PORTAL_AC_ATTR reqAuthAttr[2];


	// 组属性username
	reqAuthAttr[0].type = UserName;
	reqAuthAttr[0].len = 2;
	reqAuthAttr[0].len += strlen((char*)req_auth.name);
	strncpy((char*)reqAuthAttr[0].attrVal,req_auth.name,sizeof(reqAuthAttr[0].attrVal)-1);

//    sprintf(logBuf, "UserName:%s ,len=%d\n", req_auth.name,reqAuthAttr[0].len);
//    printf("%s",logBuf);

	// 打包password
	char password[20] = { 0 };
	strncpy(password,req_auth.password, sizeof(password)-1);
	reqAuthAttr[1].type = PassWord;
	reqAuthAttr[1].len  = 2; 
	reqAuthAttr[1].len += strlen(password);
	strncpy((char*)reqAuthAttr[1].attrVal, password,sizeof(reqAuthAttr[1].attrVal)-1); 
	
	// 10.REQ_AUTH 组大包 

	memcpy(stReqAuth.ac_attr,reqAuthAttr, reqAuthAttr[0].len);
	memcpy(stReqAuth.ac_attr+reqAuthAttr[0].len,&reqAuthAttr[1], reqAuthAttr[1].len);

	// 10.REQ_AUTH
	if (udpSendTo(sockfd, ac_ip, port, (unsigned char *)&stReqAuth,16+reqAuthAttr[0].len+reqAuthAttr[1].len) == -1)
	{
		snprintf(logBuf, sizeof(logBuf), "Err:udpSendTo failed.errno:%d errstr:%s %s %d", errno, strerror(errno), __FILE__, __LINE__);
		g_Log.Add(logBuf);
		close(sockfd);
		return -1;
	}
	// 13.ACK_AUTH
	int readRet = udpReadFrom(sockfd, (unsigned char*)&stAckAuth, sizeof (stAckAuth));
	if (readRet == -1)
	{
		snprintf(logBuf, sizeof(logBuf), "Err:udpReadFrom failed.errno:%d errstr:%s %s %d", errno, strerror(errno), __FILE__, __LINE__);
		g_Log.Add(logBuf);
		close(sockfd);
		return -1;
	}

	//log begin
	bzero(logBuf,sizeof(logBuf));
	snprintf(logBuf,sizeof(logBuf),"13.ACK_AUTH begin serialNo:%hu------------\n", stReqAuth.serialNo);
	int len = strlen(logBuf);
	unsigned char * p = (unsigned char *)&stAckAuth;
	for (int i = 0; i < readRet; i++)
	{
		snprintf(logBuf+len+i*3,(sizeof(logBuf)-len-i),"%02x ",p[i]);
	}
	snprintf(logBuf+len+readRet*3,(sizeof(logBuf)-len-readRet*3),"\n13.ACK_AUTH end------------");
	g_Log.Add(logBuf);
	printf("%s\n", logBuf);
	//log end
	
	close(sockfd);
   
	return stAckAuth.errCode;
}

int SendReqLogoutAndRecv(const char* userip, const char* ac_ip, const int port)
{
    char logBuf[256] = { 0 };
    ST_PORTAL_AC stAckLogout;
    int sockfd = 0;
    if (!CreateUDPSock(sockfd))
    {
        g_Log.Add("CreateUDPSock error");
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

	if (udpSendTo(sockfd, ac_ip, port, (unsigned char *)&stReqLogout, sizeof (stReqLogout)) == -1)
	{
		snprintf(logBuf, sizeof(logBuf), "Err:udpSendTo failed.errno:%d errstr:%s %s %d", errno, strerror(errno), __FILE__, __LINE__);
		g_Log.Add(logBuf);
		close(sockfd);
		return -1;
	}
	printf("send success, port:%d, ac_ip:%s\n",port, ac_ip);
    //11.12 AC向radius发起认证

	// 5.ACK_LOGOUT
	int readRet = udpReadFrom(sockfd, (unsigned char*)&stAckLogout, sizeof (stAckLogout));
	if (readRet == -1)
	{
		snprintf(logBuf, sizeof(logBuf), "Err:udpReadFrom failed.errno:%d errstr:%s %s %d", errno, strerror(errno), __FILE__, __LINE__);
		g_Log.Add(logBuf);
		close(sockfd);
		return -1;
	}

	//log begin
	bzero(logBuf,sizeof(logBuf));
	sprintf(logBuf, "13.ACK_LOGOUT begin serialNo:%hu------------\n", stReqLogout.serialNo);
	int len = strlen(logBuf);
	unsigned char * p = (unsigned char *)&stAckLogout;
	for (int i = 0; i < readRet; i++)
	{
		snprintf(logBuf+len+i*3,(sizeof(logBuf)-len-i),"%02x ",p[i]);
	}
	snprintf(logBuf+len+readRet*3,(sizeof(logBuf)-len-readRet*3),"\n13.ACK_LOGOUT end------------");
	
	g_Log.Add(logBuf);
	printf("%s\n", logBuf);
	//log end
	close(sockfd);
    
	return stAckLogout.errCode;

}

unsigned short generateSerialNo()
{
    unsigned short retNo = 0;
	srand((unsigned)time(NULL));
	int randval = rand();
	retNo = randval & 0xffff;

	return retNo;
}

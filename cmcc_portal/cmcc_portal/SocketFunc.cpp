/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	SocketFunc.h

作者    :	张帅   版本: v1.0    创建日期: 2013-05-28

功能描述:	网络传输工具函数

其他说明:

修改记录:
*****************************************************************/
#include "SocketFunc.h"
#include "Global.h"

using namespace std;


bool setNonBlock(int sockfd)
{
	char logBuf[256] = {0};

	int opts = fcntl(sockfd , F_GETFL);
	if(-1 == opts)
	{
		snprintf(logBuf, sizeof (logBuf), "Err:fcntl F_GETFL is failed:%s", strerror(errno));
		g_Log.Add(logBuf);
		return false;
	}

	opts = opts | O_NONBLOCK;
	if(fcntl(sockfd , F_SETFL , opts) < 0)
	{
		snprintf(logBuf, sizeof (logBuf), "Err:fcntl F_SETFL is failed:%s", strerror(errno));
		g_Log.Add(logBuf);
		return false;
	}
	return true;
}

int tcpRead(int socket, unsigned char *pucMsg, int slLen)
{
	int slLeft, slRead;
	unsigned char *pucPtr;

	pucPtr = pucMsg;
	slLeft = slLen;

	while(slLeft > 0)
	{
		slRead = recv(socket, (char *)pucPtr, slLeft, 0);
		if(slRead > 0)
		{
			slLeft -= slRead;
			pucPtr += slRead;
			continue;
		}
		else if(slRead == 0)
		{
			return 0;
		}
		else
		{
			if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				usleep(1000);
				continue;
			}
		}

		return -1;
	}

	return 0;
}

int tcpSend(int socket, unsigned char *pucMsg, int slLen)
{
	int slMsgLen = slLen, slRet = 0;
	char *pscData = (char *)pucMsg;
	time_t tFirstTime = 0;

	while (slMsgLen > 0)
	{
		slRet = send(socket, pscData, slMsgLen, MSG_NOSIGNAL);
		//发送成功，则把相应标志位设置为0
		if (slRet >= slMsgLen)
		{
			break;
		}
		//没有发送完，则继续发送
		else if (slRet > 0)
		{
			slMsgLen -= slRet;
			pscData += slRet;
			continue;
		}
		//发送失败
		else
		{
			//系统调用失败或发送缓冲区满需要继续发送该消息
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				if (tFirstTime == 0)
				{
					tFirstTime = time(NULL);
				}
				else
				{
					if ((time(NULL) - tFirstTime) > 30)
					{
						return -1;
					}
				}
				continue;
			}
		}

		return -1;
	}

	return 0;
}

int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen)
{
	char logBuf[256] = {0};

	time_t tFirstTime = 0;

	int slRead = 0;
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(cli_addr);

	while(true)
	{
		slRead = recvfrom(socket, (unsigned char *)rcvBuf, slLen,MSG_DONTWAIT, (struct sockaddr *)&cli_addr, &addr_len);
		if (slRead == -1)
		{
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				if (tFirstTime == 0)
				{
					tFirstTime = time(NULL);
				}
				else
				{
					if ((time(NULL) - tFirstTime) > 5)
					{
						return -1;
					}
				}

				usleep(1000);
				continue;
			}
			snprintf(logBuf, sizeof (logBuf), "Err:udpReadFrom:%s", strerror(errno));
			g_Log.Add(logBuf);
		}
		else
		{
			break;
		}
	}

	return slRead;
}

int udpSendTo(int socket, string strIp, int port, unsigned char *sendBuf, int slLen)
{
	char logBuf[256] = {0};

	int sLen = 0;
	struct sockaddr_in s_addr;
	socklen_t addr_len = sizeof(s_addr);
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = inet_addr(strIp.c_str());

	sLen = sendto(socket, (unsigned char*)sendBuf, slLen, 0, (struct sockaddr *)&s_addr, addr_len);
	if (sLen == -1)
	{
		snprintf(logBuf, sizeof (logBuf), "Err:udpSendTo:%s", strerror(errno));
		g_Log.Add(logBuf);
	}

	return sLen;
}

bool CreateUDPSock(int &sockfd)
{
	char logBuf[256] = {0};

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		snprintf(logBuf, sizeof (logBuf), "Err:create udp socket:%s", strerror(errno));
		g_Log.Add(logBuf);
		return false;
	}

    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct timeval r;
    r.tv_sec = 5;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &r, sizeof(r));

	return true;
}

bool CreateTCPSocket(int &sockfd)
{
	char logBuf[256] = {0};

	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		snprintf(logBuf, sizeof (logBuf), "Err:create tcp socket:%s", strerror(errno));
		g_Log.Add(logBuf);
		return false;
	}

	return true;
}

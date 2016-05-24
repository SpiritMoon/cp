/*****************************************************************
��Ȩ����:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

�ļ���  :	SocketFunc.h

����    :	��˧   �汾: v1.0    ��������: 2013-05-28

��������:	���紫�乤�ߺ���

����˵��:

�޸ļ�¼:
*****************************************************************/
#ifndef _SOCKETFUNC_H
#define _SOCKETFUNC_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>

using namespace std;


/*****************************************************************
����ԭ��:bool setNonBlock(int sockfd)
��������:����sockfdΪ������ģʽ
����˵��:sockfd:Ҫ����Ϊ������ģʽ��socket������
����ֵ	:�ɹ�:true,ʧ��:false
*****************************************************************/
bool setNonBlock(int sockfd);

/*****************************************************************
����ԭ��:int tcpRead(int socket, unsigned char *pucMsg, int slLen)
��������:socket���ݶ�ȡ TCP
����˵��:socket:socket������,pucMsg:�������ݻ�����ָ��,slLen:Ҫ��ȡ�����ݳ���
����ֵ	:�ɹ�:0,ʧ��:-1
*****************************************************************/
int tcpRead(int socket, unsigned char *pucMsg, int slLen);

/*****************************************************************
����ԭ��:int tcpSend(int socket, unsigned char *pucMsg, int slLen)
��������:socket���ݷ��� TCP
����˵��:socket:socket������,pucMsg:���������ݻ�����ָ��,slLen:Ҫ���͵����ݳ���
����ֵ	:�ɹ�:0,ʧ��:-1
*****************************************************************/
int tcpSend(int socket, unsigned char *pucMsg, int slLen);

/*****************************************************************
����ԭ��:int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen)
��������:socket���ݷ��� TCP
����˵��:socket:socket������,strIp:Ŀ��ip,port:Ŀ��port,rcvBuf:�������ݻ�����,slLen:Ҫ��ȡ�����ݳ���
����ֵ	:�ɹ�:��ȡ���ֽ���,ʧ��:-1
*****************************************************************/
int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen);

/*****************************************************************
����ԭ��:int udpSendTo(int socket, string strIp, int port, unsigned char *sendBuf, int slLen)
��������:socket���ݷ��� UDP
����˵��:socket:socket������,strIp:Ŀ��IP,port:Ŀ��˿�,sendBuf:���͵���Ϣ,slLen:������Ϣ�ĳ���
����ֵ	:�ɹ�:���͵��ֽ���,ʧ��:-1
*****************************************************************/
int udpSendTo(int socket, string strIp, int port, unsigned char *sendBuf, int slLen);

/*****************************************************************
����ԭ��:bool CreateUDPSock(int &sock)
��������:����UDP socket
����˵��:sockfd:socket������
����ֵ	:�ɹ�:true,ʧ��:false
*****************************************************************/
bool CreateUDPSock(int &sockfd);

/*****************************************************************
����ԭ��:bool CreateTCPSocket(int &sockfd)
��������:����TCP socket
����˵��:sockfd:socket������
����ֵ	:�ɹ�:true,ʧ��:false
*****************************************************************/
bool CreateTCPSocket(int &sockfd);

#endif

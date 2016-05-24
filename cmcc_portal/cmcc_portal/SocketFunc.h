/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	SocketFunc.h

作者    :	张帅   版本: v1.0    创建日期: 2013-05-28

功能描述:	网络传输工具函数

其他说明:

修改记录:
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
函数原型:bool setNonBlock(int sockfd)
函数功能:设置sockfd为非阻塞模式
参数说明:sockfd:要设置为非阻塞模式的socket描述符
返回值	:成功:true,失败:false
*****************************************************************/
bool setNonBlock(int sockfd);

/*****************************************************************
函数原型:int tcpRead(int socket, unsigned char *pucMsg, int slLen)
函数功能:socket数据读取 TCP
参数说明:socket:socket描述符,pucMsg:接收数据缓冲区指针,slLen:要读取的数据长度
返回值	:成功:0,失败:-1
*****************************************************************/
int tcpRead(int socket, unsigned char *pucMsg, int slLen);

/*****************************************************************
函数原型:int tcpSend(int socket, unsigned char *pucMsg, int slLen)
函数功能:socket数据发送 TCP
参数说明:socket:socket描述符,pucMsg:待发送数据缓冲区指针,slLen:要发送的数据长度
返回值	:成功:0,失败:-1
*****************************************************************/
int tcpSend(int socket, unsigned char *pucMsg, int slLen);

/*****************************************************************
函数原型:int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen)
函数功能:socket数据发送 TCP
参数说明:socket:socket描述符,strIp:目标ip,port:目标port,rcvBuf:接收数据缓冲区,slLen:要读取的数据长度
返回值	:成功:读取的字节数,失败:-1
*****************************************************************/
int udpReadFrom(int socket, unsigned char *rcvBuf, int slLen);

/*****************************************************************
函数原型:int udpSendTo(int socket, string strIp, int port, unsigned char *sendBuf, int slLen)
函数功能:socket数据发送 UDP
参数说明:socket:socket描述符,strIp:目标IP,port:目标端口,sendBuf:发送的信息,slLen:发送信息的长度
返回值	:成功:发送的字节数,失败:-1
*****************************************************************/
int udpSendTo(int socket, string strIp, int port, unsigned char *sendBuf, int slLen);

/*****************************************************************
函数原型:bool CreateUDPSock(int &sock)
函数功能:创建UDP socket
参数说明:sockfd:socket描述符
返回值	:成功:true,失败:false
*****************************************************************/
bool CreateUDPSock(int &sockfd);

/*****************************************************************
函数原型:bool CreateTCPSocket(int &sockfd)
函数功能:创建TCP socket
参数说明:sockfd:socket描述符
返回值	:成功:true,失败:false
*****************************************************************/
bool CreateTCPSocket(int &sockfd);

#endif

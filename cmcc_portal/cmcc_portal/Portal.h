/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	Portal.h

作者    :	耿斌   版本: v1.0    创建日期: 2015-04-08

功能描述:	处理portal协议接收和发送

其他说明:

修改记录:
*****************************************************************/
#ifndef __PORTAL_H__
#define __PORTAL_H__

#include "kaitone_std.h"
#include "Common.h"
#include "SocketFunc.h"
#include "UtilFunc.h"
#include "Global.h"
int SendReqAuthAndRecv(const ST_REQ_AUTH& req_auth,const char* ac_ip, const int port);
int SendReqLogoutAndRecv(const char* userip, const char *ac_ip, const int port);

unsigned short generateSerialNo();

#endif


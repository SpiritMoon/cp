/*****************************************************************
��Ȩ����:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

�ļ���  :	Portal.h

����    :	����   �汾: v1.0    ��������: 2015-04-08

��������:	����portalЭ����պͷ���

����˵��:

�޸ļ�¼:
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


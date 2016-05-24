#ifndef _CMCC_COMMON_H_
#define _CMCC_COMMON_H_

#include "kaitone_std.h"
#include <string.h>

#pragma pack(1)

#define PORTAL_USERNAME_LEN  128
#define PORTAL_PASSWORD_LEN  128
#define PORTAL_IP_LEN  16
#define PORTAL_MAC_LEN  18

//#define AUTHSERVER_IP	"127.0.0.1"	//鉴权服务器地址
//#define AUTHSERVER_PORT 10087	    //鉴权服务器端口

//Portal报文类型
const UINT8 REQ_CHALLENGE = 0x01;   //Client----->Server  Portal Server 向AC设备发送的请求Challeng报文
const UINT8 ACK_CHALLENGE = 0x02;   //Client<-----Server  Server AC设备对Portal Server请求Challeng报文的响应报文
const UINT8 REQ_AUTH      = 0x03;   //Client----->Server  Server Portal Server向AC设备发送的请求认证报文
const UINT8 ACK_AUTH      = 0x04;   //Client<-----Server  Server AC设备对Portal Server请求认证报文的响应报文
const UINT8 REQ_LOGOUT    = 0x05;   //Client----->Server  Server  若ErrCode字段值为0x00，表示此报文是Portal Server向AC设备发送的请求用户下线报文；
                                    //若ErrCode字段值为0x01，表示该报文是Portal Server发送的超时报文，其原因是Portal Server发出的各种请求在规定时间内没有收到响应报文。  
const UINT8 ACK_LOGOUT    = 0x06;   //Client<-----Server  Server AC设备对Portal Server请求下线报文的响应报文  
const UINT8 AFF_ACK_AUTH  = 0x07;   //Client----->Server  Server  Portal Server对收到的认证成功响应报文的确认报文
const UINT8 NTF_LOGOUT    = 0x08;   //Server --> Client   Client 用户被强制下线通知报文 
const UINT8 REQ_INFO      = 0x09;   //Client --> Server   信息询问报文 
const UINT8 ACK_INFO      = 0x0a;   //Server --> Client   信息询问的应答报文


//属性类型
const UINT8 UserName        = 0x01;        //<=253 （可变） 用 户 名
const UINT8 PassWord        = 0x02;        //<=16（可变） 用户提交的明文密码
const UINT8 Challenge       = 0x03;        //16（固定） Chap方式加密的魔术字
const UINT8 ChapPassWord    = 0x04;        //16（固定）  经过Chap方式加密后的密码

//======================AC begin======================
typedef struct portal_ac_attr
{
	UINT8 type;
	UINT8 len;
	UINT8 attrVal[253];

//	portal_ac_attr()
//	{
//		memset(this, 0, sizeof(portal_ac_attr));
//	}
}ST_PORTAL_AC_ATTR;

typedef struct portal_ac
{
	UINT8 ver;			// 协议的版本号，长度为 1 字节，目前定义的值为 0x01
	UINT8 type;			// 报文的类型，长度为 1 字节
	UINT8 pap_chap;		// 认证方式，长度为 1 字节
	UINT8 rsv;			// 保留字段，长度为 1 字节，在所有报文中值为 0
	UINT16 serialNo;	// 报文的序列号，长度为 2 字节
	UINT16 reqID;		// 2个字节，由AC设备随机生成
	UINT32 userIP;		// Portal用户的IP地址，长度为 4 字节，其值由Portal Server根据其获得的IP地址填写
	UINT16 userPort;	// UserPort字段目前没有用到，长度为 2 字节，在所有报文中其值为0
	UINT8 errCode;		// ErrCode字段和Type字段一起表示一定的意义，长度为 1字节
	UINT8 attrNum;		// 表示其后边可变长度的属性字段属性的个数，长度为 1 字节
//	ST_PORTAL_AC_ATTR ac_attr[5];
	char ac_attr[512];

	portal_ac()
	{
		bzero(this,sizeof(portal_ac));
	}
}ST_PORTAL_AC;
//======================AC end======================

typedef struct req_auth
{
    char userip[PORTAL_IP_LEN];
    char name[PORTAL_USERNAME_LEN];
    char password[PORTAL_PASSWORD_LEN];
    
    req_auth()
    {
        bzero(this,sizeof(req_auth));
    }
}ST_REQ_AUTH;


typedef struct req_mac_query
{
	int serial;
	int stat;//=1:查询到已存在  =0:没有查询到  =-1:执行查询失败
	char userName[PORTAL_USERNAME_LEN];//查询到的登录用户名
	char password[PORTAL_PASSWORD_LEN];//查询到的用户登录密码
    char acip[PORTAL_IP_LEN]; 
    char userip[PORTAL_IP_LEN];
    char usermac[PORTAL_MAC_LEN];
    req_mac_query()
    {
    	bzero(this,sizeof(req_mac_query));
    }
}ST_REQ_MAC_QUERY;

#pragma pack()

#endif

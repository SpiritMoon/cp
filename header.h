/*****************************************************
 *
 * 头文件 宏
 *
 *****************************************************/
#ifndef WT_HEADER_H
#define WT_HEADER_H

#define LOGS_MAX_SIZE			( 5 * 1024 * 1024 )	//单个log文件大小 1M
#define WRITE_LOG				1					//是否写log文件

#define MAX_EPOLL_NUM			65536				//epoll 最大数量

#if 0


#define CONFIG_FILE_NAME		"config.ini"		//配置文件名

#define WT_SQL_ERROR			6l					// 全局数据库错误标识码

#define PRINT_USER_LOG			1					// 是否打印user操作的log 比如上下线等

#define SOCK_STAT_ADD		 1				// 设备需要添加到epoll列表
#define SOCK_STAT_ADDED		 0				// 设备已添加到epoll列表
#define SOCK_STAT_DEL		-1				// 设备出错,应从当前链表删除

//#define ADVERTISING			1				// 广告系统Advertising
#endif 

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <stdarg.h>

#include <signal.h>

#include <errno.h>
#include <unistd.h>  
#include <fcntl.h>
#include <assert.h>

#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include <iconv.h>

#include <pthread.h>  

#include "cJSON.h"

// logs 函数
int logs_init(char* prefix);
void logs_destroy();
int xyprintf(int err_no, char* format, ...);

// net 函数
int wt_sock_init(int *sockfd, int port, int listen_num);
void wt_close_sock(int *sock);
int wt_send_block(int sock, unsigned char *buf, int len);
int wt_recv_block(int sock, unsigned char *buf, int len);

// platform
void* platform_conn_thread(void *fd);

extern int  cgv_platform_port;





#if 0
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;

extern char cgv_sql_name[32];
extern char cgv_sql_user[32];
extern char cgv_sql_pass[32];

//数据库操作所需参数
typedef struct wt_sql_handle{
	SQLHENV		env_handle;						// Handle ODBC environment 环境句柄
	SQLHDBC		conn_handle;					// Handle connection 连接句柄
	SQLHSTMT	sqlstr_handle;					// 数据库执行语句句柄
	SQLRETURN	sql_ret;
	char		sql_stat[12];					// Status SQL
	SQLLEN		sql_err;
	SQLSMALLINT	sql_mlen;
	char		err_msg[200];
	char		sql_str[1024];
}wt_sql_handle;
#endif

#if 0
#define PORTAL_USERNAME_LEN  128
#define PORTAL_PASSWORD_LEN  128
#define PORTAL_IP_LEN  16
#define PORTAL_MAC_LEN  18
//Portal报文类型
const unsigned char REQ_CHALLENGE = 0x01;   //Client----->Server  Portal Server 向AC设备发送的请求Challeng报文
const unsigned char ACK_CHALLENGE = 0x02;   //Client<-----Server  Server AC设备对Portal Server请求Challeng报文的响应报文
const unsigned char REQ_AUTH      = 0x03;   //Client----->Server  Server Portal Server向AC设备发送的请求认证报文
const unsigned char ACK_AUTH      = 0x04;   //Client<-----Server  Server AC设备对Portal Server请求认证报文的响应报文
const unsigned char REQ_LOGOUT    = 0x05;   //Client----->Server  Server  若ErrCode字段值为0x00，表示此报文是Portal Server向AC设备发送的请求用户下线报文；
                                    //若ErrCode字段值为0x01，表示该报文是Portal Server发送的超时报文，其原因是Portal Server发出的各种请求在规定时间内没有收到响应报文。  
const unsigned char ACK_LOGOUT    = 0x06;   //Client<-----Server  Server AC设备对Portal Server请求下线报文的响应报文  
const unsigned char AFF_ACK_AUTH  = 0x07;   //Client----->Server  Server  Portal Server对收到的认证成功响应报文的确认报文
const unsigned char NTF_LOGOUT    = 0x08;   //Server --> Client   Client 用户被强制下线通知报文 
const unsigned char REQ_INFO      = 0x09;   //Client --> Server   信息询问报文 
const unsigned char ACK_INFO      = 0x0a;   //Server --> Client   信息询问的应答报文


//属性类型
const unsigned char UserName        = 0x01;        //<=253 （可变） 用 户 名
const unsigned char PassWord        = 0x02;        //<=16（可变） 用户提交的明文密码
const unsigned char Challenge       = 0x03;        //16（固定） Chap方式加密的魔术字
const unsigned char ChapPassWord    = 0x04;        //16（固定）  经过Chap方式加密后的密码

//======================AC begin======================
typedef struct portal_ac_attr
{
	unsigned char type;
	unsigned char len;
	unsigned char attrVal[253];

//	portal_ac_attr()
//	{
//		memset(this, 0, sizeof(portal_ac_attr));
//	}
}ST_PORTAL_AC_ATTR;

typedef struct portal_ac
{
	unsigned char ver;			// 协议的版本号，长度为 1 字节，目前定义的值为 0x01
	unsigned char type;			// 报文的类型，长度为 1 字节
	unsigned char pap_chap;		// 认证方式，长度为 1 字节
	unsigned char rsv;			// 保留字段，长度为 1 字节，在所有报文中值为 0
	unsigned short serialNo;	// 报文的序列号，长度为 2 字节
	unsigned short reqID;		// 2个字节，由AC设备随机生成
	unsigned int userIP;		// Portal用户的IP地址，长度为 4 字节，其值由Portal Server根据其获得的IP地址填写
	unsigned short userPort;	// UserPort字段目前没有用到，长度为 2 字节，在所有报文中其值为0
	unsigned char errCode;		// ErrCode字段和Type字段一起表示一定的意义，长度为 1字节
	unsigned char attrNum;		// 表示其后边可变长度的属性字段属性的个数，长度为 1 字节
//	ST_PORTAL_AC_ATTR ac_attr[5];
	char ac_attr[512];
}ST_PORTAL_AC;
//======================AC end======================

typedef struct req_auth
{
    char userip[PORTAL_IP_LEN];
    char name[PORTAL_USERNAME_LEN];
    char password[PORTAL_PASSWORD_LEN];
    
    //req_auth()
    //{
     ///   bzero(this,sizeof(req_auth));
    //}
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
    //req_mac_query()
    //{
   // 	bzero(this,sizeof(req_mac_query));
    //}
}ST_REQ_MAC_QUERY;
#endif










#endif //WT_HEADER_H

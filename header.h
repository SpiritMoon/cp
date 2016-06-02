/*****************************************************
 *
 * 头文件 宏
 *
 *****************************************************/
#ifndef WT_HEADER_H
#define WT_HEADER_H

#define IS_DAEMON_EXIST			0					// 精灵线程

#define LOGS_MAX_SIZE			( 5 * 1024 * 1024 )	// 单个log文件大小 1M
#define WRITE_LOG				1					// 是否写log文件

#define MAX_EPOLL_NUM			65536				// epoll 最大数量

#define CONFIG_FILE_NAME		"config.ini"		// 配置文件名

#define PORTAL_TO_AC_PORT		2000				// AC开放给portal的端口
#define AC_TO_PORTAL_PORT		50100				// PORTAL开放给AC的端口

#define WT_SQL_ERROR			6l					// 全局数据库错误标识码

//#define SOCK_STAT_ADD		 1				// 设备需要添加到epoll列表
//#define SOCK_STAT_ADDED		 0				// 设备已添加到epoll列表
//#define SOCK_STAT_DEL		-1				// 设备出错,应从当前链表删除




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

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

// logs 函数
int logs_init(char* prefix);
void logs_destroy();
int xyprintf(int err_no, char* format, ...);

// net 函数
int wt_sock_init(int *sockfd, int port, int listen_num);
void wt_close_sock(int *sock);
int wt_send_block(int sock, unsigned char *buf, int len);
int wt_recv_block(int sock, unsigned char *buf, int len);
int UDP_create(int *sockfd);
int UDP_send_block(int socket, char* ip, int port, unsigned char *buf, int len);
int UDP_recv_block(int socket, unsigned char *rcvBuf, int slLen);

// platform
void* platform_conn_thread(void *fd);
// 平台连接的端口
extern int  cgv_platform_port;

// 配置文件
int init_ini(char* filename, int *fd, char* buf, int len);
int get_ini(char *buf, const char* key, char* value);
void destroy_ini(int fd);



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


/************************* Portal *******************************/
//Portal报文类型

#define REQ_CHALLENGE 0x01;   //Client----->Server  Portal Server 向AC设备发送的请求Challeng报文
#define ACK_CHALLENGE 0x02;   //Client<-----Server  Server AC设备对Portal Server请求Challeng报文的响应报文
#define REQ_AUTH    0x03;   //Client----->Server  Server Portal Server向AC设备发送的请求认证报文
#define ACK_AUTH       0x04;   //Client<-----Server  Server AC设备对Portal Server请求认证报文的响应报文
#define REQ_LOGOUT     0x05;   //Client----->Server  Server  若ErrCode字段值为0x00，表示此报文是Portal Server向AC设备发送的请求用户下线报文；
                                    //若ErrCode字段值为0x01，表示该报文是Portal Server发送的超时报文，其原因是Portal Server发出的各种请求在规定时间内没有收到响应报文。  
#define ACK_LOGOUT     0x06;   //Client<-----Server  Server AC设备对Portal Server请求下线报文的响应报文  
#define AFF_ACK_AUTH   0x07;   //Client----->Server  Server  Portal Server对收到的认证成功响应报文的确认报文
#define NTF_LOGOUT     0x08;   //Server --> Client   Client 用户被强制下线通知报文 
#define REQ_INFO       0x09;   //Client --> Server   信息询问报文 
#define ACK_INFO       0x0a;   //Server --> Client   信息询问的应答报文


//属性类型
#define UserName         0x01;        //<=253 （可变） 用 户 名
#define PassWord         0x02;        //<=16（可变） 用户提交的明文密码
#define Challenge        0x03;        //16（固定） Chap方式加密的魔术字
#define ChapPassWord     0x04;        //16（固定）  经过Chap方式加密后的密码

typedef struct portal_ac_attr
{
	unsigned char type;
	unsigned char len;
	unsigned char attrVal[253];
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
	
	char ac_attr[512];
}ST_PORTAL_AC;

#define PORTAL_USERNAME_LEN  128
#define PORTAL_PASSWORD_LEN  128
#define PORTAL_IP_LEN  16
#define PORTAL_MAC_LEN  18

typedef struct req_auth
{
    char userip[PORTAL_IP_LEN];
    char name[PORTAL_USERNAME_LEN];
    char password[PORTAL_PASSWORD_LEN];
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
}ST_REQ_MAC_QUERY;


// 发送认证上线函数
int SendReqAuthAndRecv(ST_REQ_AUTH *req_auth, char* ac_ip, int port);

// protal 测试线程
void* protal_test_thread(void* fd);

/********************** Radius ****************************/

// RADIUS协议的报文 属性域
struct radius_attr{
	/*
	 * Type：Attribute Number 属性号
	 * 用来说明数据包中表示的属性类型
	 * 范围是1~255
	 * 服务器和客户端都可以忽略不可识别类型的属性
	 * 值为26，标识厂商私有属性。
	 */
	unsigned char type;
	/*
	 * length 属性长度 说明整个属性域的长度
	 * 长度域最小值为3 表示属性域至少占3个字节
	 * 如果AAA服务器收到的接入请求中属性长度无效，则发送接入拒绝包
	 * 如果NAS收到的接入允许、接入拒绝和接入盘问中属性的长度也无效，则必须以接入拒绝对待，或者被简单的直接丢弃。
	 */
	unsigned char length;
	/*
	 * Value 值 表示属于自己的特性和特征
	 * 即使该域为null，这个域也必须出现在属性域中
	 * 有6种属性值——整数(INT) ；枚举(ENUM) ；IP地址(IPADDR) ；文本(STRING) ；日期(DATE) ；二进制字符串（BINARY) ；
	 */
	unsigned char value[0];
};

// RADIUS协议的报文
struct radius_bag{
	/*
	 * Code：包类型；1字节，指示RADIUS包的类型。在接收到一个无效编码域的数据包后，该数据包只是会被简单的丢弃
	 * 1:Access-Request认证请求包，必须包含User-Name
	 * 2:Access-Accept认证接受包，回复1
	 * 3:Access-Reject认证拒绝包，回复1
	 * 4:Accounting-Request计费请求包
	 * 5:Accounting-Response认证响应包
	 */
	unsigned char code;
	
	/* 
	 * Identifier： 包标识；1字节，取值范围为0 ～255；
	 * 用于匹配请求包和响应包，同一组请求包和响应包的Identifier应相同。
	 * 如果在一个很短的时间片段里，一个请求有相同的客户源IP地址、源UDP端口号和标识符，RADIUS服务器会认为这是一个重复的请求而不响应处理。
	 * 1字节意味着客户端在收到服务器的响应之前最多发送255（28-1）个请求。
	 * Identifier段里的值是序列增长的。
	 */
	unsigned char identifier;

	/*
	 *  length： 包长度；2字节；标识了分组的长度，整个包中所有域的长度。
	 *  长度域范围之外的字节被认为是附加的，并在接受的时候超长部分将被忽略。
	 *  如果包长比长度域给出的短，也必须丢弃，最小长度为20，最大长度为4096。
	 */
	unsigned short length;

	/*
	 * Authenticator：验证字域；16 字节明文随机数；最重要的字节组最先被传输。
	 * 该值用来认证来自RADIUS服务器的回复，也用于口令隐藏算法（加密）。
	 * 该验证字分为两种：
	 *  请求验证字——Request Authenticator 用在请求报文中，采用唯一的16字节随机码。
	 *  响应验证字——Response Authenticator 用在响应报文中，用于鉴别响应报文的合法性。
	 *  响应验证字＝MD5( Code + ID + length + 请求验证字 + Attributes+Key)
	 */
	unsigned char authenticator[16];

	/*
	 * Attributes：属性域
	 * 用来在请求和响应报文中携带详细的认证、授权、信息和配置细节，来实现认证、授权、计费等功能。
	 * 采用（Type、length、Value）三元组的形式提供，
	 */
	struct radius_attr attributes[0];
};

// 接收到的radius 数据
struct radius_recv{
	int recv_ret;					// 接收到的长度
	struct sockaddr_in client;		// 客户端地址信息
	char buf[1024];					// radius数据包信息
};

// radius 服务器监听线程
void* radius_conn_thread(void *fd);

#endif //WT_HEADER_H

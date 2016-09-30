/*****************************************************
 *
 * 头文件 宏
 *
 *****************************************************/
#ifndef WT_HEADER_H
#define WT_HEADER_H


#define LOGS_MAX_SIZE			( 5 * 1024 * 1024 )	// 单个log文件大小 1M
#define WRITE_LOG				0					// 是否写log文件

#define MAX_EPOLL_NUM			65536				// epoll 最大数量

#define TO_PLATFORM_PORT		5633				// 开放给平台的端口

#define DB_NAME					"wifi"
#define	DB_USERNAME				"postgres"
#define DB_PASSWORD				"Zzwx13869121158"
#define DB_HOSTADDR				"139.129.42.237"

#define LOOP_DEADLINE_INTERVAL	10

#define WX_TEMP_DISCHARGED		5

#define LOGIN_TYPE_PHONE		1
#define LOGIN_TYPE_WX			2
#define LOGIN_TYPE_WHITE		3
#define LOGIN_TYPE_TEMP			4

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

#include <ctype.h>

#include <iconv.h>

#include <pthread.h>  

#include <openssl/md5.h>

#include <postgresql/libpq-fe.h>

#include "cJSON.h"
#include "list.h"

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

// user_mp_list
void user_mp_list_init();
void user_mp_list_add(unsigned int userid, char* usermac);
int user_mp_list_find_and_del(unsigned int userid, char* usermac);

// utils
int mac_change_17(char* dest, const char* src);
int mac_change_12(char* dest, char* src);
void get_curr_time_str(char* buf);
int res_username(char* username, int* wu_id, int* login_type);

// postgresql
void sql_test();
PGconn* sql_init();
int sql_exec(PGconn *conn, char *sql_str);
int sql_exec_select(PGconn *conn, char *sql_str, PGresult** res);
char* sql_getvalue_string(PGresult *res, int h, int l);
void sql_destory(PGconn *conn);

// exec_sql.c
int get_apinfo(char* apmac, unsigned int *apid, char* domain, unsigned int *s_id);
int get_acinfo(char* acname, unsigned int *acid, char* acip, int* acport);
int get_wuid(unsigned int s_id, int type, char* para1, char* para2, unsigned int acid, char* wlanparameter, unsigned int *wu_id);
int update_wifi_user(char* username, char* acip, char* usermac);
int user_online(char* username, char* userip, char* acip, char* apmac);
int user_offline(char* username, char* userip, char* acip);
int insert_deadline(char* userip, char* acip, int acport, int discharged_time);
void* loop_deadline_thread(void *fd);
int exec_sql_test();

int SendReqLogoutAndRecv(char* userip, char* ac_ip, int port);	
void* portal_test_thread(void *fd);
	
	
	
/************************* Portal *******************************/
//Portal报文类型

#define REQ_CHALLENGE	0x01;   //Client----->Server  Portal Server 向AC设备发送的请求Challeng报文
#define ACK_CHALLENGE	0x02;   //Client<-----Server  Server AC设备对Portal Server请求Challeng报文的响应报文
#define REQ_AUTH		0x03;   //Client----->Server  Server Portal Server向AC设备发送的请求认证报文
#define ACK_AUTH		0x04;   //Client<-----Server  Server AC设备对Portal Server请求认证报文的响应报文
#define REQ_LOGOUT		0x05;   //Client----->Server  Server  若ErrCode字段值为0x00，表示此报文是Portal Server向AC设备发送的请求用户下线报文；
                                    //若ErrCode字段值为0x01，表示该报文是Portal Server发送的超时报文，其原因是Portal Server发出的各种请求在规定时间内没有收到响应报文。  
#define ACK_LOGOUT		0x06;   //Client<-----Server  Server AC设备对Portal Server请求下线报文的响应报文  
#define AFF_ACK_AUTH	0x07;   //Client----->Server  Server  Portal Server对收到的认证成功响应报文的确认报文
#define NTF_LOGOUT		0x08;   //Server --> Client   Client 用户被强制下线通知报文 
#define REQ_INFO		0x09;   //Client --> Server   信息询问报文 
#define ACK_INFO		0x0a;   //Server --> Client   信息询问的应答报文

//属性类型
#define UserName		0x01;        //<=253 （可变） 用 户 名
#define PassWord		0x02;        //<=16（可变） 用户提交的明文密码
#define Challenge		0x03;        //16（固定） Chap方式加密的魔术字
#define ChapPassWord	0x04;        //16（固定）  经过Chap方式加密后的密码

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

// 接收到的portal 数据
struct portal_recv{
	unsigned int recv_ret;			// 接收到的长度
	struct sockaddr_in client;		// 客户端地址信息
	socklen_t addrlen;				// 客户端地址长度
	char buf[1024];					// radius数据包信息
};

// 发送认证上线函数
int SendReqAuthAndRecv(char* userip, char* usernum, char* usercode, char* ac_ip, int port);

// protal 测试线程
void* portal_test_thread(void* fd);
// portal 报文打印
void xyprintf_portal_ac(ST_PORTAL_AC* pa);
// portal 监听线程
void* portal_conn_thread(void *fd);
/********************** Radius ****************************/

#define RADIUS_ATTR_TYPE_USER_NAME				1		// 用户名
#define RADIUS_ATTR_TYPE_USER_PASSWORD			2		// 用户密码
#define RADIUS_ATTR_TYPE_NAS_IP_ADDRESS			4		// nas ip地址
#define RADIUS_ATTR_TYPE_NAS_PORT				5		// nac 端口
#define RADIUS_ATTR_TYPE_SERVICE_TYPE			6		// 服务器类型 2-Framed
#define RADIUS_ATTR_TYPE_FRAMED_PROTOCOL		7		// ff-Ascend-ARA
#define RADIUS_ATTR_TYPE_FRAMED_IP_ADDRESS		8		// 用户ip
#define RADIUS_ATTR_TYPE_VENDOR_SPECIFIC		26		//
#define RADIUS_ATTR_TYPE_CALLED_STATION_ID		30		// ap mac和ssid
#define RADIUS_ATTR_TYPE_CALLING_STATION_ID		31		// 用户mac
#define RADIUS_ATTR_TYPE_NAS_IDENTIFIER			32		//
#define RADIUS_ATTR_TYPE_PROXY_STATE			33		// proxy 中兴设备需要返回回去
#define RADIUS_ATTR_TYPE_ACCT_STATUS_TYPE		40		// 计费请求标志 1-start 2-stop 3-update
#define RADIUS_ATTR_TYPE_ACCT_DELAY_TIME		41		//
#define RADIUS_ATTR_TYPE_ACCT_INPUT_OCTETS		42		//
#define RADIUS_ATTR_TYPE_ACCT_OUTPUT_OCTETS		43		//
#define RADIUS_ATTR_TYPE_ACCT_SESSION_ID		44		//
#define RADIUS_ATTR_TYPE_ACCT_AUTHENTIC			45		//
#define RADIUS_ATTR_TYPE_ACCT_SECCION_TIME		46		//
#define RADIUS_ATTR_TYPE_ACCT_INPUT_PACKETS		47		//
#define RADIUS_ATTR_TYPE_ACCT_OUTPUT_PACKETS	48		//
#define RADIUS_ATTR_TYPE_ACCT_TERMINATE_CAUSE	49		// 终止原因 1-user request
#define RADIUS_ATTR_TYPE_ACCT_INPUT_GIGAWORDS	52		//
#define RADIUS_ATTR_TYPE_ACCT_OUTPUT_GIGAWORDS	53		//
#define RADIUS_ATTR_TYPE_EVENT_TIMESTAMP		55		//
#define RADIUS_ATTR_TYPE_NAS_PORT_TYPE			61		// 连接类型 19 - Wireless-802.11
#define RADIUS_ATTR_TYPE_NAS_PORT_ID			87		//


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
	unsigned int recv_ret;			// 接收到的长度
	struct sockaddr_in client;		// 客户端地址信息
	socklen_t addrlen;				// 客户端地址长度
	char buf[1024];					// radius数据包信息
};

// radius 服务器监听线程
void* radius12_conn_thread(void *fd);
void* radius13_conn_thread(void *fd);

#define RADIUS_SECRET	"12345678"
// radius密钥长度
extern unsigned int		RADIUS_SECRET_LEN;
// radius基础数据包长度
extern unsigned int		RADIUS_BASIC_LEN;

#endif //WT_HEADER_H

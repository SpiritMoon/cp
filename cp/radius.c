/*****************************************************
 *
 * radius 服务器端
 * 用户登录认证
 *****************************************************/
#include "header.h"

#define RADIUS_DEBUG 0

#define RADIUS12_UDP_PORT	1812
#define RADIUS13_UDP_PORT	1813

unsigned int RADIUS_SECRET_LEN	= 0;	// radius密钥长度
unsigned int RADIUS_BASIC_LEN
	= sizeof(struct radius_bag);		// radius基础数据包长度

static int	radius12_udp_sockfd	= 0;	// radius12 socket
static pthread_mutex_t	radius12_udp_sockfd_lock;	// radius12 socket 互斥锁
static int	radius13_udp_sockfd	= 0;	// radius13 socket
static pthread_mutex_t	radius13_udp_sockfd_lock;	// radius13 socket 互斥锁

/** 
 *@brief  打印radius数据包
 *@param
 *@return
 */
void xyprintf_radius_bag(struct radius_bag* rb, unsigned int rb_len)
{
	
	xyprintf(0, "radius_bag->code = 0x%x\n\
			->identifier = 0x%x\n\
			->length = %u\n\
			->authenticator = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			rb->code,
			rb->identifier,
			ntohs(rb->length),
			rb->authenticator[0], rb->authenticator[1], rb->authenticator[2], rb->authenticator[3],
			rb->authenticator[4], rb->authenticator[5], rb->authenticator[6], rb->authenticator[7],
			rb->authenticator[8], rb->authenticator[9], rb->authenticator[10], rb->authenticator[11],
			rb->authenticator[12], rb->authenticator[13], rb->authenticator[14], rb->authenticator[15]
			);
	
	struct radius_attr *temp = rb->attributes;
	unsigned char buf[1024];
	for(;;){

		// 判断数据包是否已经结束
		if((char*)temp - (char*)rb >= rb_len){
			break;
		}
		
		memset(buf, 0, 1024);
		memcpy(buf, temp->value, temp->length - 2);
		
		switch( temp->type ){
			//%s
			case RADIUS_ATTR_TYPE_USER_NAME: // 1
			case RADIUS_ATTR_TYPE_CALLED_STATION_ID: //30
			case RADIUS_ATTR_TYPE_CALLING_STATION_ID: //31
			case RADIUS_ATTR_TYPE_NAS_IDENTIFIER: //32
			case RADIUS_ATTR_TYPE_PROXY_STATE: // 33
			case RADIUS_ATTR_TYPE_ACCT_SESSION_ID: //44
			case RADIUS_ATTR_TYPE_NAS_PORT_ID: //87
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %s",
					temp->type,
					temp->length,
					buf);
				break;
			// 24 * %0x2
			case RADIUS_ATTR_TYPE_USER_PASSWORD: // 2
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %02x %02x %02x %02x %02x %02x %02x %02x\n\
					%02x %02x %02x %02x %02x %02x %02x %02x\n\
					%02x %02x %02x %02x %02x %02x %02x %02x",
					temp->type,
					temp->length,
					buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
					buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15],
					buf[16], buf[17], buf[18], buf[19], buf[20], buf[21], buf[22], buf[23]);
				break;
			// ip
			case RADIUS_ATTR_TYPE_NAS_IP_ADDRESS: // 4
			case RADIUS_ATTR_TYPE_FRAMED_IP_ADDRESS: // 8 
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %u.%u.%u.%u",
					temp->type,
					temp->length,
					buf[0], buf[1], buf[2], buf[3]);
				break;
			// %x (%u)
			case RADIUS_ATTR_TYPE_NAS_PORT: // 5
			case RADIUS_ATTR_TYPE_SERVICE_TYPE: // 6
			case RADIUS_ATTR_TYPE_FRAMED_PROTOCOL: // 7
			case RADIUS_ATTR_TYPE_ACCT_STATUS_TYPE: // 40
			case RADIUS_ATTR_TYPE_ACCT_DELAY_TIME: // 41
			case RADIUS_ATTR_TYPE_ACCT_INPUT_OCTETS: //42
			case RADIUS_ATTR_TYPE_ACCT_OUTPUT_OCTETS: // 43
			case RADIUS_ATTR_TYPE_ACCT_AUTHENTIC: // 45
			case RADIUS_ATTR_TYPE_ACCT_SECCION_TIME: // 46
			case RADIUS_ATTR_TYPE_ACCT_INPUT_PACKETS: // 47
			case RADIUS_ATTR_TYPE_ACCT_OUTPUT_PACKETS: // 48
			case RADIUS_ATTR_TYPE_ACCT_TERMINATE_CAUSE: // 49
			case RADIUS_ATTR_TYPE_ACCT_INPUT_GIGAWORDS: // 52
			case RADIUS_ATTR_TYPE_ACCT_OUTPUT_GIGAWORDS: // 53
			case RADIUS_ATTR_TYPE_EVENT_TIMESTAMP: // 55
			case RADIUS_ATTR_TYPE_NAS_PORT_TYPE: //61
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %u(0x%08x)",
					temp->type,
					temp->length,
					ntohl(*((unsigned int*)buf)), ntohl(*((unsigned int*)buf)) );
				break;
			case RADIUS_ATTR_TYPE_VENDOR_SPECIFIC: // 26
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %02x %02x %02x %02x",
					temp->type,
					temp->length,
					buf[0], buf[1], buf[2], buf[3]);
				break;
			default:
				xyprintf(0, "Warning: Invalid Data Format!");
				xyprintf(0, "radius_attr->type = %u\n\
					->len = %u\n\
					->value = %02x %02x %02x %02x",
					temp->type,
					temp->length,
					buf[0], buf[1], buf[2], buf[3]);
				break;
		}
		
		temp = (struct radius_attr*)((char*)temp + temp->length);
	}

	xyprintf(0, "\n");
}



/** 
 *@brief	获取attr信息
 *@param
 *@return
 */
int get_attr_info(struct radius_bag* rb, unsigned char type, char *buf, unsigned int rb_len)
{
	int find_flag = -1;

	struct radius_attr *temp = rb->attributes;

	for(;;){
		// 判断是否已经处理完
		// 在处理完所有属性域以后，temp的地址应该是rb末尾的下一个地址
		if((char*)temp - (char*)rb >= rb_len){
			break;
		}

		if(temp->type == type){
			memcpy(buf, temp->value, temp->length - 2);
			find_flag = 0;
			break;
		}
	
		// 指向下一个属性域
		// 用当前的地址加上当前属性域的长度
		temp = (struct radius_attr*)((char*)temp + temp->length);
	}

	// 返回返回值 找到0 未找到1
	return find_flag;
}

/** 
 *@brief  radius数据包回复
 *@param
 *@return
 */
int radius12_pro_recv(struct radius_recv *rr, char* proxy)
{
	struct radius_bag *rb = (struct radius_bag*)(rr->buf);
	char temp[1024];
	struct radius_bag *rrb = (struct radius_bag*)temp;
	rrb->code = rb->code + 1;
	rrb->identifier = rb->identifier;

	unsigned int size = 0;

	if(strlen(proxy)){
		size = 2 + strlen(proxy);
		rrb->attributes[0].type = 33;
		rrb->attributes[0].length = size;
		memcpy(rrb->attributes[0].value, proxy, size);
	}
	
	rrb->length = RADIUS_BASIC_LEN + size;
	rrb->length = ((rrb->length & 0xff00) >> 8) | ((rrb->length & 0x00ff) << 8); 
	memcpy(rrb->authenticator, rb->authenticator, 16);

	// 计算回复包md5
	unsigned char buf[RADIUS_BASIC_LEN + RADIUS_SECRET_LEN + size];
	memcpy(buf, rrb, RADIUS_BASIC_LEN + size);
	memcpy(buf + RADIUS_BASIC_LEN + size, RADIUS_SECRET, RADIUS_SECRET_LEN);

	MD5(buf, RADIUS_BASIC_LEN + size + RADIUS_SECRET_LEN ,rrb->authenticator);

#if RADIUS_DEBUG	
	xyprintf_radius_bag(rrb, RADIUS_BASIC_LEN + size);
#endif
	
	// 加锁发送数据
	pthread_mutex_lock(&radius12_udp_sockfd_lock);
	int ret = sendto(radius12_udp_sockfd, rrb, RADIUS_BASIC_LEN + size, 0, (struct sockaddr *)(&(rr->client)), rr->addrlen);
	pthread_mutex_unlock(&radius12_udp_sockfd_lock);

	if( ret <= 0 ){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}


/*	
	// 回复包
	struct radius_bag rrb;
	rrb.code = rb->code + 1;
	rrb.identifier = rb->identifier;
	rrb.length = sizeof(rrb);
	rrb.length = ((rrb.length & 0xff00) >> 8) | ((rrb.length & 0x00ff) << 8); 
	memcpy(rrb.authenticator, rb->authenticator, 16);

	// 计算回复包md5
	unsigned char buf[RADIUS_BASIC_LEN + RADIUS_SECRET_LEN];
	memcpy(buf, &rrb, RADIUS_BASIC_LEN);
	memcpy(buf + RADIUS_BASIC_LEN, RADIUS_SECRET, RADIUS_SECRET_LEN);

	MD5(buf, RADIUS_BASIC_LEN + RADIUS_SECRET_LEN ,rrb.authenticator);

#if RADIUS_DEBUG	
	xyprintf_radius_bag(&rrb, sizeof(rrb));
#endif
	
	// 加锁发送数据
	pthread_mutex_lock(&radius12_udp_sockfd_lock);
	int ret = sendto(radius12_udp_sockfd, &rrb, RADIUS_BASIC_LEN, 0, (struct sockaddr *)(&(rr->client)), rr->addrlen);
	pthread_mutex_unlock(&radius12_udp_sockfd_lock);

	if( ret <= 0 ){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}
*/
#if RADIUS_DEBUG
	xyprintf(0, "1812: Send radius bag to client, ret = %d\n", ret);
#endif

	return 0;
}




/** 
 *@brief  radius数据包处理
 *@param
 *@return
 */
void* radius12_pro_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	struct radius_recv *rr = (struct radius_recv*)fd;
	struct radius_bag *rb = (struct radius_bag*)(rr->buf);

	// 判断code是否不为1
	if(rb->code != 1 && rb->code != 4){
		xyprintf(0, "1812: RADIUS DATA ERROR:code = %u", rb->code);
		goto DATA_ERR;
	}

	// 转换长度的字节序，并判断长度是否正确
	unsigned int rb_len = ((rb->length & 0xff00) >> 8) | ((rb->length & 0x00ff) << 8); 
	if(rr->recv_ret != rb_len){
		xyprintf(0, "1812: RADIUS DATA ERROR:recv_ret(%u) != length(%u)", rr->recv_ret, rb->length);
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "1812: printf radius bag in %p, lenght is %d", rr, rr->recv_ret);
	xyprintf_radius_bag(rb, rb_len);
#endif

	// 获取proxy
	char proxy[128] = {0};
	get_attr_info(rb, RADIUS_ATTR_TYPE_PROXY_STATE, proxy, rb_len);
#if RADIUS_DEBUG
	xyprintf(0, "proxy = %s", proxy);
#endif

	// 获取用户帐号
	char username[128] = {0};
	if( get_attr_info(rb, RADIUS_ATTR_TYPE_USER_NAME, username, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get username error!");
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "username = %s", username);
#endif
	
	// 获取用户mac
	char tempmac[128] = {0};
	if( get_attr_info(rb, RADIUS_ATTR_TYPE_CALLING_STATION_ID, tempmac, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get tempmac error!");
		goto DATA_ERR;
	}
	
	char usermac[128] = {0};
	if( mac_change(usermac, tempmac) ){
		xyprintf(0, "RADIUS DATA ERROR:Mac change error!");
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "usermac = %s", usermac);
#endif

	xyprintf(0, "1812: username %s, usermac %s", username, usermac);



	int wu_id, login_type;
	if( res_username(username, &wu_id, &login_type) ){
		xyprintf(0, "ERROR:%s %d", __FILE__, __LINE__);
		goto DATA_ERR;
	}
	
	// 处理是否是临时放行
	if( login_type == LOGIN_TYPE_TEMP ){
		user_mp_list_add(wu_id, usermac);
	}
	
	// 回复报文
	radius12_pro_recv(rr, proxy);

	char acip[32] = {0};
	strcpy(acip, inet_ntoa(rr->client.sin_addr.s_addr));

	if( update_wifi_user(username, acip, usermac) ){
		xyprintf(0, "ERROR:%s %d", __FILE__, __LINE__);
	}

	free(rr);
	pthread_exit(NULL);
DATA_ERR:
	free(rr);
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius process thread is end!!!");
	pthread_exit(NULL);
}

/** 
 *@brief  radius服务器监听线程
 *@param  fd类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* radius12_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius connection thread is running!!!");
	
	pthread_mutex_init(&radius12_udp_sockfd_lock, 0);
	
	pthread_t pt;
	
	while(1){
		// 初始化socket
		if((radius12_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		// 创建绑定端口信息 并绑定端口
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons( RADIUS12_UDP_PORT );
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(radius12_udp_sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(radius12_udp_sockfd);
			radius12_udp_sockfd = 0;
			sleep(10);
			break;
	    } 
		
		xyprintf(0, "UDP socket Ready!!! port is %d!", RADIUS12_UDP_PORT);
		
		// 循环接收信息
		while(1){

			// 客户端信息
			struct radius_recv *recv_temp = malloc( sizeof(struct radius_recv) );
			memset(recv_temp, 0, sizeof( struct radius_recv ) );
			recv_temp->addrlen = sizeof( recv_temp->client );
			
			// 接收信息
			recv_temp->recv_ret = recvfrom(radius12_udp_sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &(recv_temp->addrlen) );
			
			// 判断返回值
			if (recv_temp->recv_ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				free(recv_temp);
				break;
			}
#if RADIUS_DEBUG
			xyprintf(0, "1812: recv a msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
#endif
			// 创建线程处理
			if( pthread_create(&pt, NULL, radius12_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}

        }

		// 关闭socket 等待重新创建
		close(radius12_udp_sockfd);
		radius12_udp_sockfd = 0;
	}

	//到不了的地方～～～
	pthread_mutex_destroy(&radius12_udp_sockfd_lock);
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Radius pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

/* 在用户在线期间，ac会循环发送计费请求包到1813服务器，可以当作心跳*/


/** 
 *@brief  radius数据包回复
 *@param
 *@return
 */
int radius13_pro_recv(struct radius_recv *rr)
{
	struct radius_bag *rb = (struct radius_bag*)(rr->buf);
	
	// 回复包
	struct radius_bag rrb;
	rrb.code = rb->code + 1;
	rrb.identifier = rb->identifier;
	rrb.length = sizeof(rrb);
	rrb.length = ((rrb.length & 0xff00) >> 8) | ((rrb.length & 0x00ff) << 8); 
	memcpy(rrb.authenticator, rb->authenticator, 16);

	// 计算回复包md5
	unsigned char buf[RADIUS_BASIC_LEN + RADIUS_SECRET_LEN];
	memcpy(buf, &rrb, RADIUS_BASIC_LEN);
	memcpy(buf + RADIUS_BASIC_LEN, RADIUS_SECRET, RADIUS_SECRET_LEN);

	MD5(buf, RADIUS_BASIC_LEN + RADIUS_SECRET_LEN ,rrb.authenticator);

#if RADIUS_DEBUG	
	xyprintf_radius_bag(&rrb, sizeof(rrb));
#endif
	
	// 加锁发送数据
	pthread_mutex_lock(&radius13_udp_sockfd_lock);
	int ret = sendto(radius13_udp_sockfd, &rrb, RADIUS_BASIC_LEN, 0, (struct sockaddr *)(&(rr->client)), rr->addrlen);
	pthread_mutex_unlock(&radius13_udp_sockfd_lock);

	if( ret <= 0 ){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}
	
#if RADIUS_DEBUG	
	xyprintf(0, "1813: Send radius13 bag to client, ret = %d\n", ret);
#endif

	return 0;
}

/** 
 *@brief  radius数据包处理
 *@param
 *@return
 */
void* radius13_pro_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	struct radius_recv *rr = (struct radius_recv*)fd;
	struct radius_bag *rb = (struct radius_bag*)(rr->buf);

	// 判断code是否不为1
	if(rb->code != 1 && rb->code != 4){
		xyprintf(0, "RADIUS DATA ERROR:code = %u", rb->code);
		goto DATA_ERR;
	}

	// 转换长度的字节序，并判断长度是否正确
	unsigned int rb_len = ((rb->length & 0xff00) >> 8) | ((rb->length & 0x00ff) << 8); 
	if(rr->recv_ret != rb_len){
		xyprintf(0, "RADIUS DATA ERROR:recv_ret(%u) != length(%u)", rr->recv_ret, rb->length);
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "1813: printf radius bag in %p, lenght is %d", rr, rr->recv_ret);
	xyprintf_radius_bag(rb, rb_len);
#endif

	// 获取用户帐号
	char username[128] = {0};
	if( get_attr_info(rb, 1, username, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get username error!");
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "username = %s", username);
#endif
	
	// 获取操作状态
	unsigned int acct_status_type;
	if( get_attr_info(rb, RADIUS_ATTR_TYPE_ACCT_STATUS_TYPE, (char*)&acct_status_type, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get RADIUS_ATTR_TYPE_ACCT_STATUS_TYPE error!");
		goto DATA_ERR;
	}
	acct_status_type = ntohl(acct_status_type);
#if RADIUS_DEBUG
	xyprintf(0, "acct_status_type = %u", acct_status_type);
#endif

	// 获取apmac
	// 获取userip

	if( acct_status_type == 1 ){
		// TODO 用户上线成功
int user_online(char* username, char* userip, char* acip, char* apmac);
int user_offline(char* username, char* userip, char* acip, char* apmac);
		xyprintf(0, "1813: username %s, usermac %s online!!", username, usermac);
	}
	else if ( acct_status_type == 2 ){
		// TODO 用户下线
		xyprintf(0, "1813: username %s, usermac %s offline!!", username, usermac);
	}
	else if ( acct_status_type == 3 ){
		// TODO 用户状态更新
		xyprintf(0, "1813: username %s, usermac %s update!!", username, usermac);
	}
	else {
		// TODO 错误
		xyprintf(0, "ERROR -- %s -- %d : acct_status_type(u%) is error", __FILE__, __LINE__, acct_status_type);
	}

	// 回复报文
	radius13_pro_recv(rr);


	free(rr);
	pthread_exit(NULL);
DATA_ERR:
	free(rr);
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius process thread is end!!!");
	pthread_exit(NULL);
}

/** 
 *@brief  radius服务器监听线程
 *@param  fd类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* radius13_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius connection thread is running!!!");
	
	pthread_mutex_init(&radius13_udp_sockfd_lock, 0);
	
	pthread_t pt;
	
	while(1){
		// 初始化socket
		if((radius13_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		// 创建绑定端口信息 并绑定端口
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons( RADIUS13_UDP_PORT );
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(radius13_udp_sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(radius13_udp_sockfd);
			radius13_udp_sockfd = 0;
			sleep(10);
			break;
	    } 
		
		xyprintf(0, "UDP socket Ready!!! port is %d!", RADIUS13_UDP_PORT);
		
		// 循环接收信息
		while(1){

			// 客户端信息
			struct radius_recv *recv_temp = malloc( sizeof(struct radius_recv) );
			memset(recv_temp, 0, sizeof( struct radius_recv ) );
			recv_temp->addrlen = sizeof( recv_temp->client );
			
			// 接收信息
			recv_temp->recv_ret = recvfrom(radius13_udp_sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &(recv_temp->addrlen) );
			
			// 判断返回值
			if (recv_temp->recv_ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				free(recv_temp);
				break;
			}
#if RADIUS_DEBUG
			xyprintf(0, "1813: recv a msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
#endif
			// 创建线程处理
			if( pthread_create(&pt, NULL, radius13_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}

        }

		// 关闭socket 等待重新创建
		close(radius13_udp_sockfd);
		radius13_udp_sockfd = 0;
	}

	//到不了的地方～～～
	pthread_mutex_destroy(&radius13_udp_sockfd_lock);
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Radius pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

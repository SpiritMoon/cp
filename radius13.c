/*****************************************************
 *
 * radius 服务器端
 * 
 * 在用户在线期间，ac会循环发送计费请求包到1813服务器，可以当作心跳
 *****************************************************/
#include "header.h"

#define RADIUS13_DEBUG 1

#define RADIUS13_UDP_PORT	1813

// radius socket
static int				radius13_udp_sockfd		= 0;
// radius socket 互斥锁
static pthread_mutex_t	radius13_udp_sockfd_lock;

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

#if 0
	xyprintf(0, "The buf of to count md5:\n\
			%02x %02x %02x %02x\n\
			%02x %02x %02x %02x %02x %02x %02x %02x\n\
			%02x %02x %02x %02x %02x %02x %02x %02x\n\
			RADIUS_SECRET is %s",
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11],
			buf[12], buf[13], buf[14], buf[15], buf[16], buf[17], buf[18], buf[19],
			RADIUS_SECRET);
#endif

	MD5(buf, RADIUS_BASIC_LEN + RADIUS_SECRET_LEN ,rrb.authenticator);

#if RADIUS13_DEBUG	
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
	
	xyprintf(0, "1813: Send radius13 bag to client, ret = %d\n", ret);
	
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
#if RADIUS13_DEBUG
	xyprintf(0, "1813: printf radius bag in %p, lenght is %d", rr, rr->recv_ret);
	xyprintf_radius_bag(rb, rb_len);
#endif

	// 获取用户帐号
	char username[128] = {0};
	if( get_attr_info(rb, 1, username, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get username error!");
		goto DATA_ERR;
	}
#if RADIUS13_DEBUG
	xyprintf(0, "username = %s", username);
#endif
	// 获取用户mac
	char usermac[128] = {0};
	if( get_attr_info(rb, 31, usermac, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get usermac error!");
		goto DATA_ERR;
	}
#if RADIUS13_DEBUG
	xyprintf(0, "usermac = %s", usermac);
#endif
	
	//TODO 数据库操作
	
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

			xyprintf(0, "1813: recv a msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
		
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

/*****************************************************
 *
 * radius 服务器端
 * 用户登录认证
 *****************************************************/
#include "header.h"

#define RADIUS_DEBUG 1

#define RADIUS_UDP_PORT	1812

// radius密钥长度
unsigned int		RADIUS_SECRET_LEN		= 0;
// radius基础数据包长度
unsigned int		RADIUS_BASIC_LEN		= sizeof(struct radius_bag);

// radius socket
static int				radius_udp_sockfd		= 0;
// radius socket 互斥锁
static pthread_mutex_t	radius_udp_sockfd_lock;

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
			rb->length,
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
			case 1:	// User-Name
				xyprintf(0, "radius_attr->type = 1(User-Name)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
				break;
			case 2: // User-Password
				xyprintf(0, "radius_attr->type = 2(User-Password)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x",
						temp->length,
						buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
						buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15],
						buf[16], buf[17], buf[18], buf[19], buf[20], buf[21], buf[22], buf[23]);
				break;
//			case 3:	// CHAP-Password
//				break;
			case 4: // NAS-IP-Address
				xyprintf(0, "radius_attr->type = 4(NAS-IP-Address)\n\
						->len = %u\n\
						->value = %u.%u.%u.%u",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
			case 5: // NAS-Port
				xyprintf(0, "radius_attr->type = 5(NAS-Port)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
			case 6: // Service-Type
				xyprintf(0, "radius_attr->type = 5(Service-Type)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
			case 7: // Framed-Protocol
				xyprintf(0, "radius_attr->type = 5(Service-Type)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x\n\
						(Ascend-ARA(255(0xff)))",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
			case 8: // Framed-IP-Address
				xyprintf(0, "radius_attr->type = 8(Framed-IP-Address)\n\
						->len = %u\n\
						->value = %u.%u.%u.%u",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
//			case 9: // Framed-IP-Netmask
//				break;
//			case 10: // Framed-Routing
//				break;
//			case 11: // Filter-ID
//				break;
//			case 12: // Framed-MTU
//				break;
//			case 13: // Framed-Compression
//				break;
//			case 14: // login-IP-Host
//				break;
//			case 15: // login-Service
//				break;
//			case 16: // login-TCP-Port
//				break;
//			case 17: // (unassigned)
//				break;
//			case 18: // Reply_Message
//				break;
//			case 19: // Callback-Number
//				break;
//			case 20: // Callback-ID
//				break;
//			case 21: // (unassigned)
//				break;
//			case 22: // Framed-Route
//				break;
//			case 23: // Framed-IPX-Network
//				break;
//			case 24: // State
//				break;
//			case 25: // Class
//				break;
/*			case 26: // Vendor-Specific
				xyprintf(0, "radius_attr->type = 26(Vendor-Specific)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x\n\
						%02x %02x %02x %02x %02x %02x %02x %02x",
						temp->length,
						buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
						buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15],
						buf[16], buf[17], buf[18], buf[19], buf[20], buf[21], buf[22], buf[23],
						buf[24], buf[25], buf[26], buf[27], buf[28], buf[29], buf[30], buf[31],
						buf[32], buf[33], buf[34], buf[35], buf[36], buf[37], buf[38], buf[39],
						buf[40], buf[41], buf[42], buf[43], buf[44], buf[45], buf[46], buf[47],
						buf[48], buf[49], buf[50], buf[51], buf[52], buf[53], buf[54], buf[55],
						buf[56], buf[57], buf[58], buf[59], buf[60], buf[61], buf[62], buf[63],
						buf[64], buf[65], buf[66], buf[67], buf[68], buf[69], buf[70], buf[71],
						buf[72], buf[73], buf[74], buf[75], buf[76], buf[77], buf[78], buf[79],
						buf[80], buf[81], buf[82], buf[83], buf[84], buf[85], buf[86], buf[87],
						buf[88], buf[89], buf[90], buf[91], buf[92], buf[93], buf[94], buf[95]);
						break;
*/
//			case 27: // Session-Timeout
//				break;
//			case 28: // Idle-Timeout
//				break;
//			case 29: // Termination-Action
//				break;
			case 30: // Called-Station-Id
				xyprintf(0, "radius_attr->type = 30(Called-Station-Id)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
				break;
			case 31: // Calling-Station-Id
				xyprintf(0, "radius_attr->type = 31(Calling-Station-Id)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
				break;
			case 32: // NAS-Identifier
				xyprintf(0, "radius_attr->type = 32(NAS-Identifier)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
				break;
//			case 33: // Proxy-State
//				break;
//			case 34: // login-lAT-Service
//				break;
//			case 35: // login-lAT-Node
//				break;
//			case 36: // login-lAT-Group
//				break;
//			case 37: // Framed-AppleTalk-link
//				break;
//			case 38: // Framed-AppleTalk-Network
//				break;
//			case 39: // Framed-AppleTalk-Zone
//				break;
//			case 40 - 59: // (reserved for accounting)
//				break;
			case 44: // Acct-Session-Id
				xyprintf(0, "radius_attr->type = 44(Acct-Session-Id)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
				break;
//			case 60: // CHAP-Challenge
//				break;
			case 61: // NAS-Port-Type
				xyprintf(0, "radius_attr->type = 61(NAS-Port-Type)\n\
						->len = %u\n\
						->value = %02x %02x %02x %02x\n\
						(Wireless - IEEE 802.11(19(0x13))\n\
						 more in https://www.dialogic.com/webhelp/BorderNet2020/2.2.0/WebHelp/radatt_nas_port_type.htm)",
						temp->length,
						buf[0], buf[1], buf[2], buf[3]);
				break;
//			case 62: // Port-limit
//				break;
//			case 63: // login-lAT-Port
//				break;
			case 87: // NAS-Port-Id
				xyprintf(0, "radius_attr->type = 87(NAS-Port-Id)\n\
						->len = %u\n\
						->value = %s",
						temp->length,
						buf);
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

	xyprintf(0, "**** radius bag over!\n");
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
int radius_pro_recv(struct radius_recv *rr)
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

#if RADIUS_DEBUG	
	xyprintf_radius_bag(&rrb, sizeof(rrb));
#endif
	
	// 加锁发送数据
	pthread_mutex_lock(&radius_udp_sockfd_lock);
	int ret = sendto(radius_udp_sockfd, &rrb, RADIUS_BASIC_LEN, 0, (struct sockaddr *)(&(rr->client)), rr->addrlen);
	pthread_mutex_unlock(&radius_udp_sockfd_lock);

	if( ret <= 0 ){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}
	
	xyprintf(0, "1812: Send radius bag to client, ret = %d\n", ret);
	
	return 0;
}

// 将mac地址转换成 aa:aa:aa:aa:aa:aa 的形式
int mac_change(char* dest, const char* src){
	int i = 0;
	int len = strlen(src);
#if RADIUS_DEBUG
	xyprintf(0,"src mac is %s, len is %d", src, len);
#endif

	if(len > 17){
		return -1;
	}

	for(; i < len; i++){
		if(src[i] >= 65 && src[i] <= 90){
			dest[i] = src[i] + 32;
		}
		else if(src[i] >= 97 && src[i] <= 122){
			dest[i] = src[i];
		}
		else if(src[i] >= 48 && src[i] <= 57){
			dest[i] = src[i];
		}
		else{
			dest[i] = ':';
		}
	}
#if RADIUS_DEBUG
	xyprintf(0, "dest mac is %s", dest);
#endif

	return 0;
}



/** 
 *@brief  radius数据包处理
 *@param
 *@return
 */
void* radius_pro_thread(void *fd)
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

	// 获取用户帐号
	char username[128] = {0};
	if( get_attr_info(rb, 1, username, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get username error!");
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "username = %s", username);
#endif
	// 获取用户mac
	char usermac[128] = {0};
	if( get_attr_info(rb, 31, usermac, rb_len) ){
		xyprintf(0, "RADIUS DATA ERROR:Get usermac error!");
		goto DATA_ERR;
	}
#if RADIUS_DEBUG
	xyprintf(0, "usermac = %s", usermac);
#endif

	// 处理是否是临时放行
	if(!strncmp(username, "temp-", 5)){
		unsigned int id = atoi(&username[5]);
		if(id){
			char mac[128] = {0};
			if( !mac_change(mac, usermac) ){
				user_mp_list_add(id, mac);
			}
		}
	}

	//TODO 数据库操作
	
	// 回复报文
	radius_pro_recv(rr);


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
	
	pthread_mutex_init(&radius_udp_sockfd_lock, 0);
	
	pthread_t pt;
	
	while(1){
		// 初始化socket
		if((radius_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		// 创建绑定端口信息 并绑定端口
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons( RADIUS_UDP_PORT );
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(radius_udp_sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(radius_udp_sockfd);
			radius_udp_sockfd = 0;
			sleep(10);
			break;
	    } 
		
		xyprintf(0, "UDP socket Ready!!! port is %d!", RADIUS_UDP_PORT);
		
		// 循环接收信息
		while(1){

			// 客户端信息
			struct radius_recv *recv_temp = malloc( sizeof(struct radius_recv) );
			memset(recv_temp, 0, sizeof( struct radius_recv ) );
			recv_temp->addrlen = sizeof( recv_temp->client );
			
			// 接收信息
			recv_temp->recv_ret = recvfrom(radius_udp_sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &(recv_temp->addrlen) );
			
			// 判断返回值
			if (recv_temp->recv_ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				free(recv_temp);
				break;
			}

			xyprintf(0, "1812: recv a msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
		
			// 创建线程处理
			if( pthread_create(&pt, NULL, radius_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}

        }

		// 关闭socket 等待重新创建
		close(radius_udp_sockfd);
		radius_udp_sockfd = 0;
	}

	//到不了的地方～～～
	pthread_mutex_destroy(&radius_udp_sockfd_lock);
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Radius pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

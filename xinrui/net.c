/*****************************************************
 *
 * 网络操作函数
 *
 *****************************************************/

#include "header.h"

/** 
 *@brief  初始化服务器监听套接字
 *@param  sockfd	类型 int*	返回创建的socket套接字
 *@param  port		类型 int	要监听的端口
 *@param  listen_num类型 int	最大监听连接数量
 *@return success 0 failed -1
 */
int wt_sock_init(int *sockfd, int port, int listen_num)
{
	struct sockaddr_in sa;

	//创建socket
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		xyprintf(errno, "SOCK_ERROR:%s %s %d -- socket() error!", __func__, __FILE__, __LINE__);
		goto ERR_END;
	}
    
	//解除IP端口没有及时释放而无法绑定 保证服务器关掉后，立即可以启动
	int reuseaddr = 1;
    setsockopt (*sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof (reuseaddr));
	
	//变量
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons( port );
	
	//绑定端口
	if (bind( *sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
		xyprintf(errno, "SOCK_ERROR:%s %s %d -- bind() error!", __func__, __FILE__, __LINE__);
		goto ERR_END;
	}
	//为什么要注释掉这里呢?
	//因为默认就是阻塞的 -- 14.10.12
//	if (wt_setnonblocking( *sockfd ) < 0){
//		goto ERR_END;
//	}
	//开启监听
	if (listen(*sockfd, listen_num) < 0){
		xyprintf(errno, "SOCK_ERROR:%s %s %d -- listen() error!", __func__, __FILE__, __LINE__);
		goto ERR_END;
	}
	//完成
	xyprintf(0, "TCP socket Ready!!! port is %d, listen num is %d!", port, listen_num);
	return 0;

ERR_END:
	xyprintf(errno, "SOCK_ERROR:%s %s %d -- sock init failed, port is %d, listen num is %d!", __func__, __FILE__, __LINE__, port, listen_num);
	if (*sockfd != -1) {
		close( *sockfd );
		*sockfd = -1;
	}
	return -1;
}

/** 
 *@brief  关闭socket
 *@param  sock		类型 int*	要关闭的socket套接字
 *@return nothing
 */
void wt_close_sock(int *sock)
{
	if (sock == NULL){
		return;
	}
	
	if (*sock != -1) {
		close(*sock);
		*sock = -1;
	}
}

/** 
 *@brief  非阻塞
 *@param  sockfd	类型 int	要操作的socket套接字
 *@return success 0 failed -1
 */
int inline wt_setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1){
		xyprintf(errno, "SOCK_ERROR:%s %s %d", __func__, __FILE__, __LINE__);
        return -1;
	}
    return 0;
}

/** 
 *@brief  阻塞
 *@param  sockfd	类型 int	要操作的socket套接字
 *@return success 0 failed -1
 */
int inline wt_setblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) & ~O_NONBLOCK) == -1){
		xyprintf(errno, "SOCK_ERROR:%s %s %d", __func__, __FILE__, __LINE__);
		return -1;
	}
    return 0;
}

/** 
 *@brief  发送数据函数
 *@param  sock		类型 int			要发送到的socket套接字
 *@param  buf		类型 unsigned char*	要发送的内容
 *@param  len		类型 int			要发送的内容长度
 *@return success 0 failed -1
 */
int wt_send_block(int sock, unsigned char *buf, int len)
{
	int ret;			//返回值
	int slen = len;		//未发送字节数
	int yelen = 0;		//已发送字节数
	struct timeval tv;	//超时时间结构体

	//错误判断
	if (sock == -1 || !buf || len <= 0){
		xyprintf(errno, "SOCK_ERROR:%s %s %d -- I can't believe, why here !!! （╯' - ')╯︵ ┻━┻ ", __func__, __FILE__, __LINE__);
		return -1;
	}

	//设置阻塞
//	if ( wt_setblocking(sock) < 0){
//		xyprintf(errno, "SOCK_ERROR: %s %d -- wt_setblocking()", __FILE__, __LINE__);
//		return -1;
//	}

	//设置超时时间
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	//开启发送
	while (slen > 0) {
		
		//发送 -- send -- ┬─┬ ノ( ' - 'ノ)
		ret = send(sock, buf + yelen, slen, 0);		
		
		if (ret < 0) {			// 发送错误
			xyprintf(errno, "SOCK_ERROR:%s %s %d -- send() failed!", __func__, __FILE__, __LINE__);
			//如果错误码是 EAGAIN 则再次尝试 不是那就 goto END喽 o(╯□╰)o
			if (errno == EAGAIN) {
				continue;
			}
			return -1;
	    }
		else if (ret == 0) {	// 对方关闭了套接字
			return -1;
		}
		else {					// 成功
			yelen += ret;	//已发送
			slen -= ret;	//未发送
		}

	}
	//恢复到非阻塞
//	wt_setnonblocking(sock);
	return 0;
}

/** 
 *@brief  接收数据函数
 *@param  sock		类型 int			要接收数据的socket套接字
 *@param  buf		类型 unsigned char*	要接收的内容
 *@param  len		类型 int			要接收的内容长度
 *@param  block_flag类型 int			1 使用阻塞接收, 0 使用非阻塞接收
 *@return success 0 failed -1
 */
int wt_recv_block(int sock, unsigned char *buf, int len/*, int block_flag*/)
{
	int ret;			// 返回值
	int rlen = 0;		// 已接收到的字节数
	struct timeval tv;	// 超时时间结构体

	//错误判断
	if (sock == -1 || !buf || len <= 0){
		xyprintf(errno, "SOCK_ERROR:%s %s %d -- I can't believe, why here !!! （╯' - ')╯︵ ┻━┻ ", __func__, __FILE__, __LINE__);
		return -1;
	}

/*
	//设置阻塞
	if(block_flag){
		if ( wt_setblocking(sock) < 0){
			xyprintf(errno, "SOCK_ERROR: %s %d -- wt_setblocking()", __FILE__, __LINE__);
			return -1;
		}
	}
*/	
	//设置超时时间
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	//开始接收
	while (rlen < len) {
		//接收
		ret =  recv(sock, buf + rlen, len - rlen, 0);
		if (ret < 0 ) {				// 读取错误
			xyprintf(errno, "SOCK_ERROR:%s %s %d -- recv() failed!", __func__, __FILE__, __LINE__);
//			if (errno == EAGAIN) {
//				continue;
//			}
			return -1;
		}
		else if (ret == 0) {		// 对方关闭了套接字
			xyprintf(errno, "SOCK_ERROR:%s %s %d -- recv() failed!", __func__, __FILE__, __LINE__);
			return -1;
		}
		else {						// 读取正常
			rlen += ret;
		}
	}
/*	
	//恢复到非阻塞
	if(block_flag){
		wt_setnonblocking(sock);
	}
	*/
	return 0;
}

/************************  UDP  *******************************/

/** 
 *@brief  UDP create
 *@param  
 *@return 
 */
int UDP_create(int *sockfd)
{
	if((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}

    //int reuse = 1;
    //setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	int on=1;
	setsockopt(*sockfd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on));
    
	struct timeval r;
    r.tv_sec = 5;
    setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
	setsockopt(*sockfd, SOL_SOCKET, SO_SNDTIMEO, &r, sizeof(r));

	return 0;
}

/** 
 *@brief  UDP send
 *@param  
 *@return 
 */
int UDP_send_block(int socket, char* ip, int port, unsigned char *buf, int len)
{
	struct sockaddr_in s_addr;
	socklen_t addr_len = sizeof(s_addr);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = inet_addr(ip);

	if( sendto(socket, buf, len, 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) <= 0){
		xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
		return -1;
	}

	return 0;
}

/** 
 *@brief  UDP recv
 *@param  
 *@return 
 */
int UDP_recv_block(int socket, unsigned char *rcvBuf, int slLen)
{
	time_t tFirstTime = 0;

	int slRead = 0;
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(cli_addr);
	int count = 0;

	while(1){
		slRead = recvfrom(socket, rcvBuf, slLen, MSG_DONTWAIT, (struct sockaddr *)&cli_addr, &addr_len);
		if (slRead == -1){
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)){
				xyprintf(0,"no msg recv, sleep 1s continue, slLen = %d!", slLen);
				sleep(1);
				count++;
				if(count >= 30){
					return -1;
				}
				continue;
			}
			else {
				xyprintf(errno, "ERROR - %s - %s - %d", __FILE__, __func__, __LINE__);
				return -1;
			}
		}
		else{
			break;
		}
	}
	return 0;
}


/*****************************************************
 *
 * Protal 服务器端
 *
 *****************************************************/
#include "header.h"

#define PORTAL_RECV_DEBUG 1

#define PORTAL_RECV_PORT	50100

// portal recv socket
static int				portal_recv_sockfd		= 0;
// radius socket 互斥锁
static pthread_mutex_t	portal_recv_sockfd_lock;

/** 
 *@brief  radius数据包处理
 *@param
 *@return
 */
void* portal_pro_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Protal process thread is running!!!");
	
	struct portal_recv *pr = (struct portal_recv*)fd;
	struct portal_ac  *pa = (struct portal_ac*)(pr->buf);

#if PORTAL_RECV_DEBUG
	xyprintf(0, "pr = %p, pr->recv_ret = %d", pr, pr->recv_ret);
	xyprintf_portal_ac(pa);
#endif
	
	//TODO 数据库操作
	
	// 回复报文


	free(pr);
	pthread_exit(NULL);
DATA_ERR:
	free(pr);
	xyprintf(0, "** O(∩ _∩ )O ~~ Portal process thread is end!!!");
	pthread_exit(NULL);
}

/** 
 *@brief  radius服务器监听线程
 *@param  fd类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* portal_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	
	xyprintf(0, "** O(∩ _∩ )O ~~ Portal connection thread is running!!!");
	
	pthread_mutex_init(&portal_recv_sockfd_lock, 0);
	
	pthread_t pt;
	
	while(1){
		// 初始化socket
		if((portal_recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		// 创建绑定端口信息 并绑定端口
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons( PORTAL_RECV_PORT );
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(portal_recv_sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(portal_recv_sockfd);
			portal_recv_sockfd = 0;
			sleep(10);
			break;
	    } 
		
		xyprintf(0, "UDP socket Ready!!! port is %d!", PORTAL_RECV_PORT);
		
		// 循环接收信息
		while(1){

			// 客户端信息
			struct portal_recv *recv_temp = malloc( sizeof(struct portal_recv) );
			memset(recv_temp, 0, sizeof( struct portal_recv ) );
			recv_temp->addrlen = sizeof( recv_temp->client );
			
			// 接收信息
			recv_temp->recv_ret = recvfrom(portal_recv_sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &(recv_temp->addrlen) );
			
			// 判断返回值
			if (recv_temp->recv_ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				free(recv_temp);
				break;
			}

			xyprintf(0, "recv a portal msg set in %p, ret is %d", recv_temp, recv_temp->recv_ret);
		
			// 创建线程处理
			if( pthread_create(&pt, NULL, portal_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}

        }

		// 关闭socket 等待重新创建
		close(portal_recv_sockfd);
		portal_recv_sockfd = 0;
	}

	//到不了的地方～～～
	pthread_mutex_destroy(&portal_recv_sockfd_lock);
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Portal pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

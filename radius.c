#include "header.h"


struct radius_attr{
	unsigned char type;
	unsigned char length;
	unsigned char value[0];
};

struct radius_bag{
	unsigned char code;			//
	unsigned char identifier;	//
	unsigned short length;		// 长度
	unsigned char authenticator[16];	//md5
	struct radius_attr attributes[0];
};

struct radius_recv{
	int ret;
	struct sockaddr_in client;
	char buf[1024];
};

void xyprintf_radius_bag(struct radius_bag* rb)
{
	unsigned short num = ((rb->length & 0xff00) >> 8) | ((rb->length & 0x00ff) << 8); 
	xyprintf(0, "radius_bag->code = 0x%x\n\
			->identifier = 0x%x\n\
			->length = %u\n\
			->authenticator = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			rb->code,
			rb->identifier,
			num,
			rb->authenticator[0],rb->authenticator[1],rb->authenticator[2],rb->authenticator[3],
			rb->authenticator[4],rb->authenticator[5],rb->authenticator[6],rb->authenticator[7],
			rb->authenticator[8],rb->authenticator[9],rb->authenticator[10],rb->authenticator[11],
			rb->authenticator[12],rb->authenticator[13],rb->authenticator[14],rb->authenticator[15]);
	//xyprintf(0,"rb = %p, rb + %u = %p", rb, num, (char*)rb + num);
	struct radius_attr *temp = rb->attributes;
	char buf[1024];
	for(;;){
		memset(buf, 0, 1024);
		memcpy(buf, temp->value, temp->length - 2);
		xyprintf(0, "radius_attr->type = 0x%x\n\
				->len = %u\n\
				->value = %s",
				temp->type,
				temp->length,
				buf);
		
		//xyprintf(0, "temp = %p", temp);
		temp = (struct radius_attr*)((char*)temp + temp->length);
		//xyprintf(0, "temp = %p", temp);
		
		if((char*)temp - (char*)rb >= num){
			//xyprintf(0, "~~~~~~~~~~~");
			break;
		}
	}
}





void* radius_pro_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius process thread is running!!!");
	
	struct radius_recv *rb = (struct radius_recv*)fd;
	xyprintf(0, "rb = %p, rb->ret = %d", rb, rb->ret);
	
	xyprintf_radius_bag((struct radius_bag*)(rb->buf));


	free(rb);
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius process thread is end!!!");
	pthread_exit(NULL);
}
/** 
 *@brief  平台连接监听线程函数
 *@param  fd类型 void*	线程启动参数,未使用
 *@return nothing
 */
void* radius_conn_thread(void *fd)
{
	pthread_detach(pthread_self());
	xyprintf(0, "** O(∩ _∩ )O ~~ Radius connection thread is running!!!");
	while(1){
		int sockfd;

		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			sleep(10);
			break;
		}
		
		struct sockaddr_in server;
		bzero(&server,sizeof(server));
		server.sin_family=AF_INET;
		server.sin_port=htons(1812);
		server.sin_addr.s_addr= htonl (INADDR_ANY);
		if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
			xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
			close(sockfd);
			sleep(10);
			break;
	    }   
        
		while(1){
			struct radius_recv *recv_temp = malloc(sizeof(struct radius_recv));
			memset(recv_temp, 0, sizeof(struct radius_recv));
			socklen_t addrlen = sizeof(recv_temp->client);
			
			recv_temp->ret = recvfrom(sockfd, recv_temp->buf, 1024, 0, (struct sockaddr*)&(recv_temp->client), &addrlen);
			if (recv_temp->ret < 0){
				xyprintf(errno, "%s %s %d", __FILE__, __func__, __LINE__);
				break;
			}
			xyprintf(0, "recv a msg in %p, ret is %d", recv_temp, recv_temp->ret);
			pthread_t pt;
			if( pthread_create(&pt, NULL, radius_pro_thread, (void*)recv_temp) != 0 ){
				xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
				free(recv_temp);
			}
        }
		close(sockfd);
		sleep(10);
	}

	//到不了的地方～～～
	xyprintf(0, "PLATFORM_ERROR:✟ ✟ ✟ ✟  -- %s %d:Radius pthread is unnatural deaths!!!", __FILE__, __LINE__);
	pthread_exit(NULL);
}

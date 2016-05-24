#include "Portal.h"
#include <stdio.h>
#include "SocketFunc.h"

int main()
{


	int sockfd = 0;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		xyprintf();
		goto ;
	}

    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
	struct timeval r;
    r.tv_sec = 5;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &r, sizeof(r));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(11111);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		printf("bind error\n");
		close(sockfd);
		return -1;
	}
	ST_PORTAL_AC stAckAuth;
	//int readRet = udpReadFrom(sockfd, (unsigned char*)&stAckAuth, sizeof (stAckAuth));
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(cli_addr);
	int ret = recvfrom(sockfd, (unsigned char*)&stAckAuth, sizeof(stAckAuth), 0, (struct sockaddr*)&cli_addr, &addr_len);
	if (ret == -1)
	{
		xyprintf(errno, "recvfrom error");
		goto ;
	}
	printf("read success\n");
	/*char username[253] = { 0 };
	memcpy(username, stAckAuth.ac_attr[0].attrVal, stAckAuth.ac_attr[0].len - 2);
	char password[253] = { 0 };
	memcpy(password, stAckAuth.ac_attr[1].attrVal, stAckAuth.ac_attr[1].len);
	printf("read success , userip: %s\nusername:%s\n", intIp_to_strIp(stAckAuth.userIP).c_str(), username);
*/
	printf("userip : %s\n", intIp_to_strIp(stAckAuth.userIP).c_str());
	stAckAuth.errCode = 8;

	if (-1 == sendto(sockfd, (unsigned char*)&stAckAuth, sizeof(stAckAuth), 0, (struct sockaddr*)&cli_addr, addr_len))
	{
		printf("udpSendTo error\n");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	return 0;
}


#include "stdio.h"
#include "Portal.h"

CKTLog g_Log;

void print_usage(int type)
{
    if(type==0){
        printf("test portal auth \n");
        printf("Usage:send 0 acip acport usr pwd usrIP\n");
    }
    if(type==1){
        printf("test portal logout \n");
        printf("usage:send 1 acip acport usrIP\n");
    }

}

/*
≤‚ ‘portal-authed
./send 0 60.216.6.217 2000 198057015103 0358123  10.0.200.6
≤‚ ‘portal-logout
./send 1 60.216.6.217 2000 10.0.200.6  
*/
int main(int argc ,char* argv[])
{
    int ret;
    int type;
    int acport=0;
    char acip[20]={0};
    char usrIp[20]={0};

    if(argc!=5 && argc!=7)
    {
        print_usage(0);
        print_usage(1);
        return 0;
    }

    type = atoi(argv[1]); 
	strncpy(acip,argv[2],sizeof(acip)-1);
	acport=atoi(argv[3]);

    switch ( type )
    {
        case 0 :
            if(argc==7){
                ST_REQ_AUTH req_auth;
                char usr[64]={0};
                char pwd[64]={0};
                
                strncpy(usr,argv[4],sizeof(usr)-1);
                strncpy(pwd,argv[5],sizeof(pwd)-1);
                strncpy(usrIp,argv[6],sizeof(usrIp)-1);
                
                memcpy(req_auth.name,usr,strlen(usr));
                memcpy(req_auth.password,pwd, strlen(pwd));
                memcpy(req_auth.userip,usrIp, strlen(usrIp));
                ret = SendReqAuthAndRecv(req_auth,acip,acport);
                if(ret==0)
                {
                    printf("auth returned success.\n");
                }
                else
                {
                    printf("auth fail.errcode=%d\n",ret);
                    switch ( ret )
                    {
                        case 1 :
                            printf("auth request is refused.\n");
                            break;
                        case 2 :
                            printf("user is alread passed through.\n"); 
                            break;
                        case 3 : 
                            printf("another auth request is going on,please wait.\n"); 
                            break;
                        case 4 :
                        	printf("error accoured when process this authentication.\n");
                            break;
                        default:
                        	printf("unexpected error occured.\n"); 
                    }
                }

            }
            else{
                print_usage(0);
            }
            break;
        case 1 :
            if(argc==5){
                strncpy(usrIp,argv[4],sizeof(usrIp)-1);
                ret = SendReqLogoutAndRecv(usrIp,acip,acport);
                if(ret==0)
                {
                    printf("logout success.\n");
                }
                else
                {
                    printf("logout failed. errcode=%d\n",ret);
                    switch ( ret )
                    {
                        case 1 : 
							printf("logout request is refused.\n"); 
                            
                            break;
                        case 2 : 
							printf("error accoured when process logout request\n"); 
                            break;
                        default: 
                        	printf("unexpected error occured.\n"); 
                        	break; 
                    }
                }
            }else{
                print_usage(1);
            }

            break;
        default:
            print_usage(0);
            print_usage(1);
    }

    printf("ret : %d\n", ret);
    return 0;
}


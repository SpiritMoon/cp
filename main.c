/*****************************************************
 *
 * 主函数 & 启动
 *
 *****************************************************/
#include "header.h"

int	 cgv_platform_port = 5633;					// 开放给平台的端口
//全局变量 读取自配置文件
char cgv_sql_name[32];					// sql name
char cgv_sql_user[32];					// sql user
char cgv_sql_pass[32];					// sql password
//pthread_mutex_t		gv_task_lock;					// 数据库定时任务互斥锁
//pthread_mutex_t		gv_boss_flag_lock;
//int					gv_boss_flag = 0;				// boss控制的是否可以引导的标志
//int					gv_heart_interval = 300;		// 心跳时间
//int					gv_num_falg = 0;				// 数量控制falg
//int					gv_total_num = 0;				// 设备数量
//int					gv_rt_num = 0;					// 设备类型数量
//LIST_HEAD(			gv_boss_rt_head);				// 设备类型链表

/** 
 *@brief  获取程序配置
 *@return success 0 failed -1
 */
int get_config()
{
	int fd;
	char buf[1024];
	//config文件初始化
	if( init_ini(CONFIG_FILE_NAME, &fd, buf, 1024) ){
		return -1;
	}
	//获取sql数据库 配置名
	if( get_ini(buf, "sql_name", cgv_sql_name) ){
		return -1;
	}
	//获取sql数据库 登陆用户名
	if( get_ini(buf, "sql_user", cgv_sql_user) ){
		return -1;
	}
	//获取sql数据库 登陆密码
	if( get_ini(buf, "sql_pass", cgv_sql_pass) ){
		return -1;
	}
	//char temp[16];
	//获取 认证服务器连接端口
	//if( get_ini(buf, "report_port", temp) ){
	//	return -1;
	//}
	//cgv_report_port = atoi(temp);
	//获取 认证服务器状态报告间隔时间
	//关闭config文件
	destroy_ini( fd );

	//打印配置信息
	xyprintf(0, "** O(∩ _∩ )O ~~ Get config success!!\n\tsql_name = %s\n\tsql_user = %s\n\tsql_pass = %s",
			cgv_sql_name, cgv_sql_user, cgv_sql_pass);

	return 0;
}

/** 
 *@brief  程序主体函数 由子进程运行
 */
void run()
{
#if IS_DAEMON_EXIST 
	//关闭父进程创建的log文件
	logs_destroy();
#endif
	//创建子进程log文件
	if( logs_init("logs") ){
		printf("logs_init error!!!!\n");
		exit(-1);
	}
	
	//打印程序开启信息
	xyprintf(0, "\n\t\t\t*************************************************************************\n\
			*************************************************************************\n\
			****                                                                 ****\n\
			****                      The cp.out is running!                      ****\n\
			****                          Pid is %d                            ****\n\
			****                                                                 ****\n\
			*************************************************************************\n\
			*************************************************************************\n",
			getpid() );
	
	
	//获取配置
	if( get_config() ){
		printf("get config error!!!!\n");
		sleep(1000);
		exit(-1);
	}

	//初始化认证服务器list信息互斥锁 和 定时任务 互斥锁
//	pthread_mutex_init(&gv_authenticate_list_lock, 0);

	/****************平台连接监视线程**************************************************/
	pthread_t pt;
	if( pthread_create(&pt, NULL, platform_conn_thread, NULL) != 0 ){
		xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
	}
	/**********************************************************************************/
	if( pthread_create(&pt, NULL, radius_conn_thread, NULL) != 0 ){
		xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
	}
	/**********************************************************************************/
	if( pthread_create(&pt, NULL, protal_test_thread, NULL) != 0 ){
		xyprintf(errno, "PTHREAD_ERROR: %s %d -- pthread_create()", __FILE__, __LINE__);
	}
	/**********************************************************************************/

	while(1){
//		guide_conn_thread();
	}

	//不会执行到的一步
	//pthread_mutex_destroy(&gv_authenticate_list_lock);
}

/** 
 *@brief  主函数 启动子进程中运行程序主体 父进程监控主进程的运行状态
 */
int main(int argc, char** argv)
{
#if IS_DAEMON_EXIST 
	if( logs_init("wizard_logs") ){
		printf("DATA_ERROR:%s %d -- logs_init error!!!!\n",__FILE__, __LINE__);
		exit(-1);
	}

	// 守护进程（父进程）
	int status;
	for(;;){
		if( 0 == fork() ){
			run();
		}
		xyprintf(0, "ELF_THREAD:The child process is running!");
	    waitpid(-1, &status, 0);
		xyprintf(0, "ELF_THREAD:The child process is dead!");
		if(WIFSIGNALED(status)) {
			xyprintf(0, "ELF_THREAD:The child process is dead! Signal is %d", WTERMSIG(status) );
        }
	}
#else
	run();
#endif
}

/*****************************************************
 *
 * 设备接入 管理 等
 *
 *****************************************************/

#include "header.h"

#define USER_MP_LIST_DEBUG	1 

// 链表结构体
typedef struct usermac_vs_wlanparameter{
	struct list_head	node;
	unsigned int		userid;
	char				usermac[20];
}user_mp_list_node;

// 链表结构体长度
static unsigned int user_mp_list_node_size = sizeof(user_mp_list_node);

// 链表头
LIST_HEAD(			user_mp_list_head);
// 链表互斥锁
static pthread_mutex_t		user_mp_list_lock;

#if USER_MP_LIST_DEBUG
// 链表计数器
static int user_mp_list_count = 0;
#endif


/** 
 *@brief  链表初始化
 *@return nothing
 */
void user_mp_list_init()
{
	pthread_mutex_init(&user_mp_list_lock, 0);
}

/** 
 *@brief  链表数据添加
 *@return nothing
 */
void user_mp_list_add(unsigned int userid, char* usermac)
{
	user_mp_list_node *node = malloc( user_mp_list_node_size );
	node->userid = userid;
	memcpy(node->usermac, usermac, 17);

	pthread_mutex_lock(&user_mp_list_lock);
	list_add(&(node->node), &user_mp_list_head);
#if USER_MP_LIST_DEBUG
	user_mp_list_count ++;
	xyprintf(0, "USER MP LIST ADD : user_mp_list_count = %d, id is %u, mac is %s", user_mp_list_count, node->userid, node->usermac);
#endif
	pthread_mutex_unlock(&user_mp_list_lock);
}

/** 
 *@brief  链表数据查找并删除
 *@return nothing
 */
int user_mp_list_find_and_del(unsigned int userid, char* usermac)
{
	int find_flag = -1;
	struct list_head* pos;
	user_mp_list_node* node;

	pthread_mutex_lock(&user_mp_list_lock);
	for( pos = user_mp_list_head.next; pos != &user_mp_list_head; pos = pos->next ){
		node = (user_mp_list_node*)pos;
		if( node->userid == userid ){
			find_flag = 0;
			list_del( pos );
#if USER_MP_LIST_DEBUG
			user_mp_list_count --;
			xyprintf(0, "USER MAC LIST FIND : user_mp_list_count = %d, id is %u, mac is %s", user_mp_list_count, node->userid, node->usermac);
#endif
			break;
		}
	}
	pthread_mutex_unlock(&user_mp_list_lock);
	
	if( !find_flag ){
		memcpy(usermac, node->usermac, 17);
		if(node){
			free( node );
		}
	}
	return find_flag;
}

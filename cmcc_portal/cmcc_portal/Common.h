#ifndef _CMCC_COMMON_H_
#define _CMCC_COMMON_H_

#include "kaitone_std.h"
#include <string.h>

#pragma pack(1)

#define PORTAL_USERNAME_LEN  128
#define PORTAL_PASSWORD_LEN  128
#define PORTAL_IP_LEN  16
#define PORTAL_MAC_LEN  18

//#define AUTHSERVER_IP	"127.0.0.1"	//��Ȩ��������ַ
//#define AUTHSERVER_PORT 10087	    //��Ȩ�������˿�

//Portal��������
const UINT8 REQ_CHALLENGE = 0x01;   //Client----->Server  Portal Server ��AC�豸���͵�����Challeng����
const UINT8 ACK_CHALLENGE = 0x02;   //Client<-----Server  Server AC�豸��Portal Server����Challeng���ĵ���Ӧ����
const UINT8 REQ_AUTH      = 0x03;   //Client----->Server  Server Portal Server��AC�豸���͵�������֤����
const UINT8 ACK_AUTH      = 0x04;   //Client<-----Server  Server AC�豸��Portal Server������֤���ĵ���Ӧ����
const UINT8 REQ_LOGOUT    = 0x05;   //Client----->Server  Server  ��ErrCode�ֶ�ֵΪ0x00����ʾ�˱�����Portal Server��AC�豸���͵������û����߱��ģ�
                                    //��ErrCode�ֶ�ֵΪ0x01����ʾ�ñ�����Portal Server���͵ĳ�ʱ���ģ���ԭ����Portal Server�����ĸ��������ڹ涨ʱ����û���յ���Ӧ���ġ�  
const UINT8 ACK_LOGOUT    = 0x06;   //Client<-----Server  Server AC�豸��Portal Server�������߱��ĵ���Ӧ����  
const UINT8 AFF_ACK_AUTH  = 0x07;   //Client----->Server  Server  Portal Server���յ�����֤�ɹ���Ӧ���ĵ�ȷ�ϱ���
const UINT8 NTF_LOGOUT    = 0x08;   //Server --> Client   Client �û���ǿ������֪ͨ���� 
const UINT8 REQ_INFO      = 0x09;   //Client --> Server   ��Ϣѯ�ʱ��� 
const UINT8 ACK_INFO      = 0x0a;   //Server --> Client   ��Ϣѯ�ʵ�Ӧ����


//��������
const UINT8 UserName        = 0x01;        //<=253 ���ɱ䣩 �� �� ��
const UINT8 PassWord        = 0x02;        //<=16���ɱ䣩 �û��ύ����������
const UINT8 Challenge       = 0x03;        //16���̶��� Chap��ʽ���ܵ�ħ����
const UINT8 ChapPassWord    = 0x04;        //16���̶���  ����Chap��ʽ���ܺ������

//======================AC begin======================
typedef struct portal_ac_attr
{
	UINT8 type;
	UINT8 len;
	UINT8 attrVal[253];

//	portal_ac_attr()
//	{
//		memset(this, 0, sizeof(portal_ac_attr));
//	}
}ST_PORTAL_AC_ATTR;

typedef struct portal_ac
{
	UINT8 ver;			// Э��İ汾�ţ�����Ϊ 1 �ֽڣ�Ŀǰ�����ֵΪ 0x01
	UINT8 type;			// ���ĵ����ͣ�����Ϊ 1 �ֽ�
	UINT8 pap_chap;		// ��֤��ʽ������Ϊ 1 �ֽ�
	UINT8 rsv;			// �����ֶΣ�����Ϊ 1 �ֽڣ������б�����ֵΪ 0
	UINT16 serialNo;	// ���ĵ����кţ�����Ϊ 2 �ֽ�
	UINT16 reqID;		// 2���ֽڣ���AC�豸�������
	UINT32 userIP;		// Portal�û���IP��ַ������Ϊ 4 �ֽڣ���ֵ��Portal Server�������õ�IP��ַ��д
	UINT16 userPort;	// UserPort�ֶ�Ŀǰû���õ�������Ϊ 2 �ֽڣ������б�������ֵΪ0
	UINT8 errCode;		// ErrCode�ֶκ�Type�ֶ�һ���ʾһ�������壬����Ϊ 1�ֽ�
	UINT8 attrNum;		// ��ʾ���߿ɱ䳤�ȵ������ֶ����Եĸ���������Ϊ 1 �ֽ�
//	ST_PORTAL_AC_ATTR ac_attr[5];
	char ac_attr[512];

	portal_ac()
	{
		bzero(this,sizeof(portal_ac));
	}
}ST_PORTAL_AC;
//======================AC end======================

typedef struct req_auth
{
    char userip[PORTAL_IP_LEN];
    char name[PORTAL_USERNAME_LEN];
    char password[PORTAL_PASSWORD_LEN];
    
    req_auth()
    {
        bzero(this,sizeof(req_auth));
    }
}ST_REQ_AUTH;


typedef struct req_mac_query
{
	int serial;
	int stat;//=1:��ѯ���Ѵ���  =0:û�в�ѯ��  =-1:ִ�в�ѯʧ��
	char userName[PORTAL_USERNAME_LEN];//��ѯ���ĵ�¼�û���
	char password[PORTAL_PASSWORD_LEN];//��ѯ�����û���¼����
    char acip[PORTAL_IP_LEN]; 
    char userip[PORTAL_IP_LEN];
    char usermac[PORTAL_MAC_LEN];
    req_mac_query()
    {
    	bzero(this,sizeof(req_mac_query));
    }
}ST_REQ_MAC_QUERY;

#pragma pack()

#endif

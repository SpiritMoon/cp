/*****************************************************************
��Ȩ����:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

�ļ���  :	UtilFunc.h

����    :	��˧   �汾: v1.0    ��������: 2013-05-28

��������:	���ߺ���

����˵��:

�޸ļ�¼:
*****************************************************************/
#ifndef _UTILFUNC_H
#define _UTILFUNC_H

#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <set>
#include <string>
#include <iostream>
using namespace std;

#define msleep(ms) usleep(ms*1000)//�����ӳٺ���
#define BIT_ISSET(s,n) ((s)&=(0x01<<n))


string intIp_to_strIp(unsigned int pIp);
unsigned int strIp_to_intIp(string pIp);

#endif

/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	UtilFunc.h

作者    :	张帅   版本: v1.0    创建日期: 2013-05-28

功能描述:	工具函数

其他说明:

修改记录:
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

#define msleep(ms) usleep(ms*1000)//毫秒延迟函数
#define BIT_ISSET(s,n) ((s)&=(0x01<<n))


string intIp_to_strIp(unsigned int pIp);
unsigned int strIp_to_intIp(string pIp);

#endif

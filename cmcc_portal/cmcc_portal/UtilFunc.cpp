/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	UtilFunc.h

作者    :	张帅   版本: v1.0    创建日期: 2013-05-28

功能描述:	工具函数

其他说明:

修改记录:
*****************************************************************/
#include "UtilFunc.h"

string intIp_to_strIp(unsigned int pIp)
{
	string strIp;
	char arrIp[32]={'\0'};
/*	sprintf(arrIp,"%d.",(pIp >> 24));
	strIp= arrIp;
	sprintf(arrIp,"%d.",(pIp & 0x00FFFFFF)>>16);
	strIp += arrIp;
	sprintf(arrIp,"%d.",(pIp & 0x0000FFFF)>>8);
	strIp += arrIp;
	sprintf(arrIp,"%d",(pIp & 0x000000FF));
	strIp += arrIp;	*/

	sprintf(arrIp, "%d.", (pIp & 0x000000FF));
	strIp = arrIp;
	sprintf(arrIp, "%d.", (pIp & 0x0000FFFF) >> 8);
	strIp += arrIp;
	sprintf(arrIp, "%d.", (pIp & 0x00FFFFFF) >> 16);
	strIp += arrIp;
	sprintf(arrIp, "%d", (pIp >> 24));
	strIp += arrIp;
	return strIp; 
}

unsigned int strIp_to_intIp(string pIp)
{
	unsigned int intIp = 0;
	unsigned int tempInt = 0;
	string pTemp;
	int pos;
	int tempPos;
	pos = pIp.find(".",0);
	pTemp = pIp.substr(0,pos);	
	intIp = strtoul(pTemp.c_str(),0,10);
	intIp = intIp << 24;

	tempPos = pIp.find(".",pos+1);
	pTemp = pIp.substr(pos+1,(tempPos-pos-1));	
	tempInt = strtoul(pTemp.c_str(),0,10);
	intIp |= (tempInt << 16);
	pos = tempPos;

	tempPos = pIp.find(".",pos+1);
	pTemp = pIp.substr(pos+1,(tempPos-pos-1));	
	tempInt = strtoul(pTemp.c_str(),0,10);
	intIp |= tempInt << 8;

	pTemp = pIp.substr(tempPos+1,(pIp.length()-tempPos-1));
	tempInt = strtoul(pTemp.c_str(),0,10);
	intIp |= tempInt;	
	return intIp;
}


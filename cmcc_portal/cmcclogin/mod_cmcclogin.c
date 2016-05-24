/*****************************************************************
版权声明:	(C), 2012-2013, Kaitone Tech. Co., Ltd.

文件名  :	mod_login.c

作者    :	张帅   版本: v1.0    创建日期: 2012-11-05

功能描述:	login模块实现.处理登陆的HTTP请求.

其他说明:

修改记录:
*****************************************************************/

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "kaitone_std.h"
#include "common.h"
#include "ap_check_param.c"

#include <vector>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

#ifdef DEBUG
	static CKTLog	g_Log;
	static bool	g_bInit = false;
#endif


//处理登陆包HTTP请求回调函数
static int cmcclogin_handler(request_rec *r)
{
	if (strcmp(r->handler, "cmcclogin")) 
	{
		return DECLINED;
	}
	r->content_type = "text/html";

	apr_table_setn(r->headers_out, "Cache-control", "no-cache");

#ifdef DEBUG
	if (!g_bInit)
	{
		g_bInit = true;
		pid_t pid = getpid();
		char szLogName[128];
		snprintf(szLogName,sizeof(szLogName),"cmcclogin_%d",pid);	//根据pid设置日志文件名
		g_Log.SetProcName(szLogName);
		g_Log.Add("login create");
	}
	char logBuf[2048];
#endif

	//获取URL参数
	ST_CMCCLOGIN_REQUEST login_request;	//登陆请求参数结构
	memset(&login_request, 0, sizeof(login_request));
	ST_LOGIN_RESPONSE login_response;	//登陆响应
	memset(&login_response, 0, sizeof(login_response));
	CKTString	gw_address = "";
	CKTString	gw_port	= "";
	CKTString	gw_id = "";
	CKTString	mac = "";
	CKTString	url = "";

	if (NULL != r->args)
	{
		CKTToken	urlToken;
		urlToken.SetString(r->args,"&");
		CKTString	urlPara = "";

		vector<CKTString> vecPara;
		vecPara.clear();
		//循环获取URL中所有参数
		while(urlToken.GetToken(urlPara))
		{
			vecPara.push_back(urlPara);
		}

		for (int i=0; i<vecPara.size(); i++)
		{
			CKTToken subToken;
			subToken.SetString(vecPara[i].GetBuf(),"=");
			CKTString	key = "";
			CKTString	value = "";
			bool key_ret = subToken.GetToken(key);
			bool value_ret = subToken.GetToken(value);
			if (key_ret && value_ret)
			{
				if(key == "apmac")
				{
                    gw_id = value;
                    gw_id.ToUpper();	//gw_id转大写
				}
				else if(key == "usermac")
				{
					mac = value;
					mac.ToLower();	//mac转小写
				}
				else if(key == "url")
				{
					url = value;					
				} else if (key == "ssid") {
					strncpy(login_request.ssid, value.GetBuf(), sizeof(login_request.ssid) - 1);
				} else if (key == "userip") {
					strncpy(login_request.userip, value.GetBuf(), sizeof(login_request.userip) - 1);
				} else if (key == "userurl") {
					strncpy(login_request.url, value.GetBuf(), sizeof(login_request.url) - 1);
				}
			}
		}

		strncpy(login_request.gw_id, gw_id.GetBuf(), sizeof(login_request.gw_id) - 1);
		strncpy(login_request.mac, mac.GetBuf(), sizeof(login_request.mac) - 1);
//		strncpy(login_request.url, url.GetBuf(), sizeof(login_request.url) - 1);
		strncpy(login_request.gw_address, "172.0.0.1", sizeof(login_request.gw_address) - 1);
		strncpy(login_request.gw_port, "80", sizeof(login_request.gw_port) - 1);
#ifdef DEBUG
        g_Log.Add("before apr_table_get\n");
#endif

        //获取浏览器UA
        const char* uaStr = apr_table_get(r->headers_in, "User-Agent");
        if (uaStr != NULL)
        {
		    strncpy(login_request.useragent, uaStr, sizeof(login_request.useragent) - 1);
		    useragent_filter(login_request.useragent);
        }
        else
        {
            strcpy(login_request.useragent, "");
        }
#ifdef DEBUG
        sprintf(logBuf, "finish apr_table_get:%s\n", login_request.useragent);
        g_Log.Add(logBuf);
#endif
	}

	if ((gw_id != "") && (mac != ""))
	{
		CTCPClient tcpClient;
		if (!tcpClient.Connect(AUTHSERVER_IP, AUTHSERVER_PORT))
		{
#ifdef DEBUG
			sprintf(logBuf, "connect error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
			g_Log.Add(logBuf);
#endif
			tcpClient.closeSocket();

			return DECLINED;
		}

		//向鉴权服务器发送参数
		unsigned char requestType = CMCC_LOGIN;
		if (-1 == tcpClient.tcpSend((unsigned char *)&requestType, sizeof(requestType)))
		{
#ifdef DEBUG
			sprintf(logBuf, "tcpSend error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
			g_Log.Add(logBuf);
#endif
			tcpClient.closeSocket();
		}
		if (-1 == tcpClient.tcpSend((unsigned char *)&login_request, sizeof(login_request)))
		{
#ifdef DEBUG
			sprintf(logBuf, "tcpSend error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
			g_Log.Add(logBuf);
#endif
			tcpClient.closeSocket();

			return DECLINED;
		}
		//接收鉴权服务器的响应
		//读取登录类型
		char loginresponse = -1;
		if (-1 == tcpClient.tcpRead((unsigned char *)&loginresponse, sizeof(loginresponse)))
		{
#ifdef DEBUG
			sprintf(logBuf, "tcpRead error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
			g_Log.Add(logBuf);
#endif
			tcpClient.closeSocket();

			return DECLINED;
		}

		switch (loginresponse)
		{
		case AUTO_LOGIN:
			{
				ST_AUTO_LOGIN_RESPONSE stAutoLoginRes;
				memset(&stAutoLoginRes,0,sizeof(stAutoLoginRes));
				if (-1 == tcpClient.tcpRead((unsigned char *)&stAutoLoginRes, sizeof(stAutoLoginRes)))
				{
#ifdef DEBUG
					sprintf(logBuf, "tcpRead error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
					g_Log.Add(logBuf);
#endif
					tcpClient.closeSocket();

					return DECLINED;
				}

				tcpClient.closeSocket();

				CKTString js_location = "";
				CKTString js_string = "";

				if (!stAutoLoginRes.bCheckCanSuffer)
				{
					js_location = "";
					js_location = "http://";
					js_location += WEBSERVER_IP;
					js_location += "/gw_message?message=overflow&mac=";
					js_location += mac;
					js_location += "&gw_id=";
					js_location += gw_id;
					js_string = "";
					js_string = "<SCRIPT LANGUAGE='javascript'>";
					js_string += "window.location.href='";
					js_string += js_location;
					js_string += "'</SCRIPT>";

					ap_rprintf(r, "%s\n",js_string.GetBuf());
				}
				else
				{
					js_location = "";
					js_location = "http://";
					js_location += gw_address;
					js_location += ":";
					js_location += gw_port;
					js_location += "/wifidog/auth?token=";
					js_location += stAutoLoginRes.szToken;
					js_string = "";
					js_string = "<SCRIPT LANGUAGE='javascript'>";
					js_string += "window.location.href='";
					js_string += js_location;
					js_string += "'</SCRIPT>";
					ap_rprintf(r, "%s\n",js_string.GetBuf());
				}
			}
			break;
		case PORTAL_LOGIN:
			{
				ST_LOGIN_PORTAL_RESPONSE stPortalLoginRes;
				memset(&stPortalLoginRes,0,sizeof(stPortalLoginRes));
				if (-1 == tcpClient.tcpRead((unsigned char *)&stPortalLoginRes, sizeof(stPortalLoginRes)))
				{
					tcpClient.closeSocket();
					return DECLINED;
				}

				tcpClient.closeSocket();
				CKTString js_string = "";
				CKTString js_location = "";
				
				if (!stPortalLoginRes.bCheckCanSuffer)
				{
					js_location = "";
					js_location = "http://";
					js_location += WEBSERVER_IP;
					js_location += "/gw_message?message=overflow&mac=";
					js_location += mac;
					js_location += "&gw_id=";
					js_location += gw_id;
                    // 改为 http 302 跳转
                    apr_table_setn(r->headers_out, "Http", "302");
                    apr_table_setn(r->headers_out, "Location", js_location.GetBuf());
                    r->status = HTTP_MOVED_TEMPORARILY;	//302
                    ap_send_error_response(r, 0);
				}
				else
				{
                    if (!stPortalLoginRes.bSignCheckPass)
                    {
                        ap_rprintf(r, "							\
                                      <html>								\
                                      <head>								\
                                      <title>密码错误</title>				\
                                      </head>								\
                                      <script>							\
                                      function passwordalert()			\
                                      {									\
                                      alert(\"密码错误,请重新输入！\");	\
                                      history.go(-1)		\
                                      }									\
                                      </script>							\
                                      <body onload=\"passwordalert()\">	\
                                      </body>								\
                                      </html>");		
                    }
                    else
                    {
						if (stPortalLoginRes.loginway == 33) {
							apr_table_setn(r->headers_out, "Http", "302");
							apr_table_setn(r->headers_out, "Location", stPortalLoginRes.url);
							r->status = HTTP_MOVED_TEMPORARILY;	//302
							ap_send_error_response(r, 0);							
						} else {
							CKTString jumpUrl = stPortalLoginRes.url;

							js_string = "<SCRIPT LANGUAGE='javascript'>";
							js_string += "window.location.href='";
							js_string += jumpUrl.GetBuf();                        
							js_string += "'</SCRIPT>";
							ap_rprintf(r, "%s\n",js_string.GetBuf());
						}
                    }
				}
			}
			break;
		case USER_LOGIN:
			{
				ST_USER_LOGIN_RESPONSE stUserLoginRes;
				memset(&stUserLoginRes,0,sizeof(stUserLoginRes));
				if (-1 == tcpClient.tcpRead((unsigned char *)&stUserLoginRes, sizeof(stUserLoginRes)))
				{
#ifdef DEBUG
					sprintf(logBuf, "tcpRead error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
					g_Log.Add(logBuf);
#endif
					tcpClient.closeSocket();

					return DECLINED;
				}

				tcpClient.closeSocket();

				if (stUserLoginRes.bNeedLogin)
				{
					CKTString js_location = "";
					CKTString js_string = "";

					js_location = "http://";					
					switch (stUserLoginRes.loginway)
					{
					case EN_USER_LOGIN_STATIC_PASSWORD:                        
#ifdef AUTH_TEST_NEWWEIXIN_VER
                        js_location += WEBNEWWEIXIN_IP;
                        js_location += "/weixin/common?";
#elif defined (AUTH_TENCENT_VER)
                        js_location += WEBNEWWEIXIN_IP;
                        js_location += "/weixin/common?";
#elif defined (AUTH_ALI_VER)
                        js_location += WEBNEWWEIXIN_IP;
                        js_location += "/weixin/common?";
#else
                        js_location += WEBSERVER_IP;                        
                        if (stUserLoginRes.isUseWeixinLoginHtml)
                            js_location += "/weblogin.html?";
                        else                        
                            js_location += "/weblogin_old.html?";
                            //js_location += WEBSERVER_IP_WEIXIN;
#endif
						break;
					case EN_USER_LOGIN_DYNAMIC_PASSWORD:
                        js_location += WEBSERVER_IP;
						js_location += "/weblogin2.html?";
						break;
					case EN_USER_LOGIN_NEW_AUTO_LOGIN:
                        js_location += WEBSERVER_IP;
						js_location += "/userlogin?loginrequesttype=3&";	//新自动登录,直接到userlogin
					default:
						break;
					}

					js_location += "gw_address=";
					js_location += gw_address.GetBuf();
					js_location += "&gw_port=";
					js_location += gw_port.GetBuf();
					js_location += "&gw_id=";
					js_location += gw_id.GetBuf();
					js_location += "&mac=";
					js_location += mac.GetBuf();
					js_location += "&url=";
                    js_location += url.GetBuf();
                    js_location += "&com_id=";
                    js_location += stUserLoginRes.szCompanyID;

                    /*
					js_string = "<SCRIPT LANGUAGE='javascript'>";
					js_string += "window.location.href='";
					js_string += js_location;
					js_string += "'</SCRIPT>";
					ap_rprintf(r, "%s\n",js_string.GetBuf());
                    */
                    // 改为 http 302 跳转
                    apr_table_setn(r->headers_out, "Http", "302");
                    apr_table_setn(r->headers_out, "Location", js_location.GetBuf());
                    r->status = HTTP_MOVED_TEMPORARILY;	//302
                    ap_send_error_response(r, 0);

				}
				else
				{
					CKTString js_location = "";
					CKTString js_string = "";

					if (!stUserLoginRes.bCheckCanSuffer)
					{
						js_location = "";
						js_location = "http://";
						js_location += WEBSERVER_IP;
						js_location += "/gw_message?message=overflow&mac=";
						js_location += mac;
						js_location += "&gw_id=";
						js_location += gw_id;
                        /*
						js_string = "";
						js_string = "<SCRIPT LANGUAGE='javascript'>";
						js_string += "window.location.href='";
						js_string += js_location;
						js_string += "'</SCRIPT>";
						ap_rprintf(r, "%s\n",js_string.GetBuf());
                        */

                        // 改为 http 302 跳转
                        apr_table_setn(r->headers_out, "Http", "302");
                        apr_table_setn(r->headers_out, "Location", js_location.GetBuf());
                        r->status = HTTP_MOVED_TEMPORARILY;	//302
                        ap_send_error_response(r, 0);

					}
					else
					{
						js_location = "";
						js_location = "http://";
						js_location += gw_address;
						js_location += ":";
						js_location += gw_port;
						js_location += "/wifidog/auth?token=";
						js_location += stUserLoginRes.szToken;
                        /*
						js_string = "";
						js_string = "<SCRIPT LANGUAGE='javascript'>";
						js_string += "window.location.href='";
						js_string += js_location;
						js_string += "'</SCRIPT>";
						ap_rprintf(r, "%s\n",js_string.GetBuf());
                        */

                        // 改为 http 302 跳转
                        apr_table_setn(r->headers_out, "Http", "302");
                        apr_table_setn(r->headers_out, "Location", js_location.GetBuf());
                        r->status = HTTP_MOVED_TEMPORARILY;	//302
                        ap_send_error_response(r, 0);
					}
				}
			}
			break;
            // 2014-03-16 huhb add >>>>
            // 需要自动放行处理
        case LOGIN_AUTO_PASS:
            {

                ST_AUTO_LOGIN_RESPONSE stAutoLoginRes;
                memset(&stAutoLoginRes,0,sizeof(stAutoLoginRes));
                if (-1 == tcpClient.tcpRead((unsigned char *)&stAutoLoginRes, sizeof(stAutoLoginRes)))
                {
#ifdef DEBUG
                    sprintf(logBuf, "tcpRead error:%s %s:%d", strerror(errno), __FILE__, __LINE__);
                    g_Log.Add(logBuf);
#endif
                    tcpClient.closeSocket();
                    return DECLINED;
                }

                tcpClient.closeSocket();
                CKTString js_location = "";
                CKTString js_string = "";
				// 
				js_location = "http://hao123.com";
				// 改为 http 302 跳转
				apr_table_setn(r->headers_out, "Http", "302");
				apr_table_setn(r->headers_out, "Location", js_location.GetBuf());
				r->status = HTTP_MOVED_TEMPORARILY;	//302
				ap_send_error_response(r, 0);	
            }
            break;
            // 2014-03-16 huhb add <<<<
		default:
            {
			    tcpClient.closeSocket();
                char tmps[128] = {0};
                bool directJump = false;
                if (WEIXIN_COMPANY_ERROR == loginresponse)
                    sprintf(tmps, "login error: %d,wifi company is not wechat comapny!", loginresponse);
                else if (WEIXIN_PARAM_ERROR == loginresponse)
                    sprintf(tmps, "login error: %d,param error!", loginresponse);
                else if (WEIXIN_COMP_SEL_MPACCOUNT_ERR == loginresponse)
                    sprintf(tmps, "login error: %d,select mpAccount by company error!", loginresponse);
                else if (LOGIN_AP_NOT_SET == loginresponse)
                    sprintf(tmps, "login error: %d,the AP is invalid!", loginresponse);
                else
                {
                    directJump = true;
                    sprintf(tmps, "login error: %d", loginresponse);
                }
                if (directJump)
                {
                    CKTString js_string = "<SCRIPT LANGUAGE='javascript'>";
                    js_string += "window.location.href='m.hao123.com'</SCRIPT>";
                    ap_rprintf(r, "%s\n",js_string.GetBuf());
                }
                else
                {
			        apr_table_setn(r->headers_out, "Http", "400");
			        ap_rputs(tmps ,r);
                }
            }
			break;
		}
	}
	else
	{
		ap_rputs("the gw_address or gw_port or gw_id or mac is empty!", r);
	}

    return OK;
}

static void cmcclogin_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(cmcclogin_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA cmcclogin_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    cmcclogin_register_hooks  /* register hooks                      */
};

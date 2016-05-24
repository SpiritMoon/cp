向AC发送请求认证报文以及请求用户下线报文模块

### 包含接口
SendReqAuthAndRecv()	发送REQ_AUTH到AC，并且等待其应答ACK_AUTH，函数范围值为ACK_AUTH的错误码，为0表示认证成功
SendReqLogoutAndRecv()	发送REQ_LOGOUT到AC，并且等待其应答ACK_LOGOUT，函数返回值为错误码，为0表示下线成功


### 生成文件
libPortal.so

### 运行环境：
目前的测试环境在192.168.2.5的gengbin用户下的/home/gengbin/portal目录
其中send.cpp 为测试发送数据包文件，recv.cpp为模拟AC收到数据包后，给客户端回应的服务端


### 发送参数参考文件
svn://192.168.1.48/dev/yfzx/WifiAp/PortalServer/doc/中国移动WLAN业务PORTAL协议规范V2.0.0.pdf

### 认证方式
采用Pap认证
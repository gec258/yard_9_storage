#include "server.h"
#include "err.h"

#define BUFF_LEN 1024

static void handle_udp_msg(int fd)
{
    
}

/*

创建udp套接字

*/

uint32_t tk_init_udp_server(const char * ip,uint32_t port, int * socket_fd)
{

	int   ret;
	struct sockaddr_in ser_addr; 

	*socket_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET:IPV4;SOCK_DGRAM:UDP
	if(*socket_fd < 0)
	{
		ERR_DIS("create socket fail!\n");
		return RT_CREAT_UDP_FD_FAILED;
	}

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(ip);// htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
	ser_addr.sin_port = htons(port);  
	ret = bind(*socket_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if(ret < 0)
	{
		ERR_DIS("socket bind fail!\n");
		return RT_SOCKET_BIND_FAILED;
	}

	//handle_udp_msg(socket_fd);

	//close(socket_fd);
	return SUCCESS;
}

/*

关闭udp套接字

*/
uint32_t tk_uninit_udp_server(int socket_fd)
{
	
	if (socket_fd)
	{
	   close(socket_fd);
	   
	}
return SUCCESS;	
}

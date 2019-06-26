#include "server.h"
#include "err.h"
#include <unistd.h>
#include "fs.h"
#include <pthread.h>
/*


*/


#define IS_AUTO_INIT_ZONE     1
/* If the automatic initialization space is set, the number of spaces does not take effect.*/
#define MANUAL_INIT_ZONE_NUM  0


#define MODULE_PROC_NUM       3
#define MODULE_IP_LEN         20
#define BUFFER_SIZE           1024


typedef struct{

	unsigned char ip[MODULE_IP_LEN];
	uint64_t      port;
	int           socket_fd;  
	pthread_t     tid;
	volatile uint32_t      th_is_running; // 线程运行标志
	uint32_t exit_status;
	void * (*fun)(void *);
}module_t ,* p_module_t;



typedef struct
{

	module_t proc[3];


}obj_t,p_obj_t;



void * tk_thread_sniffer_pack_0(void *arg)
{

	p_module_t proc=(p_module_t) arg;
	LOG_INFO ("线程: %d START! \n",(int)pthread_self());

}



/*

   远程控制  

*/

void * tk_thread_remote_control_pack(void *arg)
{

	p_module_t proc=(p_module_t) arg;
	LOG_INFO ("线程: %d START! \n",(int)pthread_self());

}



void * tk_thread_sniffer_pack_1(void *arg)
{
	LOG_INFO ("线程: %d START! \n",(int)pthread_self());
	p_module_t proc=(p_module_t) arg;
	proc->th_is_running=1;
	char * recv_buf = (char * )malloc ( BUFFER_SIZE* sizeof(char));  //接收缓冲区，1024字节

	socklen_t clent_len;
	struct sockaddr_in clent_addr;  //clent_addr用于记录发送方的地址信息
	clent_len = sizeof(clent_addr);

	if (tk_init_udp_server(proc->ip,proc->port,&proc->socket_fd)!=SUCCESS)
	{
		ERR_DIS("udp server start failed\n");
		//	goto err_out;
	}

	while(proc->th_is_running)
	{

		if(recvfrom(proc->socket_fd, recv_buf, BUFFER_SIZE, 0, (struct sockaddr*)&clent_addr, &clent_len)==-1)  
		{
			printf("recieve data fail!\n");
			return;
		}

		//  sendto(fd, recv_buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, clent_len);  

	}

	close(proc->socket_fd);
	free(recv_buf);
}






uint32_t tk_init_proc(p_module_t deal)
{

	uint32_t i;
	/*
	   抓包线程
	 */
	deal[0].fun=tk_thread_sniffer_pack_0;
	deal[1].fun=tk_thread_sniffer_pack_1;
	deal[2].fun=tk_thread_remote_control_pack;

	for (i=0;i<MODULE_PROC_NUM;i++)
	{

		if (pthread_create(&deal[i].tid,NULL,deal[i].fun,&deal[i])!=0)
		{
			ERR_DIS("Thread creat failed  %d  \n",i);
			return TH_CREAT_FAILED;
		}


	}
	return SUCCESS;
}


uint32_t tk_uninit_proc(p_module_t deal)
{
	uint32_t i=0;
	for(i=0;i<MODULE_PROC_NUM;i++)
		pthread_join(deal[i].tid,NULL);//等待trd线程结束

	return SUCCESS;
}


uint32_t tk_cmd_get(int argc ,char * argv[])
{

	return SUCCESS;
}


/*

   配置文件       

 */
uint32_t load_cfg_info()
{

	return SUCCESS;
}


/*
0:MATE
1:SAVE
2:A PORT
3:B PORT
 */

static int __init (int argc ,char * argv[])
{

	obj_t ob;
	if (tk_init_proc(ob.proc)!=SUCCESS)
	{
		ERR_DIS("Start thread failed\n");

	}


	LOG_INFO("Start the server successfully\n");


	tk_uninit_proc(ob.proc);
	return 0;

err_out:
	return -1;
}




int main (int argc ,char * argv[])
{

	return  __init(argc,argv);
}

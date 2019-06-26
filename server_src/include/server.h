/*!
 
Author: zhangfei
Date:
  
*/

#ifndef __SERVER__
#define __SERVER__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

uint32_t tk_creat_udp_server(const char * ip,uint32_t port);
	


#endif

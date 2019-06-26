#ifndef __ERR_H_
#define __ERR_H_
#include <stdlib.h>
#include <stdio.h>
/*
 
 
*/

/*assert debug*/
#define LIB_TOSTRING(s) #s    
#define LIB_STRINGIFY(s)         LIB_TOSTRING(s)
#define TK_CHECK(cond) do {                                               \
        if (!(cond)) {                                                      \
                printf("Assertion %s failed at %s:%d\n",    \
                                LIB_STRINGIFY(cond), __FILE__, __LINE__);                 \
                abort();                                                        \
        }                                                                   \
} while (0)

#define  ERR_DIS(format,...) printf("FILE: "__FILE__", LINE: %d: "format" ", __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format,...) printf("FILE: "__FILE__", LINE: %d: "format" ", __LINE__, ##__VA_ARGS__)
#define TRACE(format,...) printf("FILE: "__FILE__", LINE: %d: "format" ", __LINE__, ##__VA_ARGS__)
  
#define SUCCESS  0
#define FAILED   1
#define RT_CREAT_UDP_FD_FAILED 100
#define RT_SOCKET_BIND_FAILED  101



#define DRIVER_SUCCESS    SUCCESS       
#define DRIVER_OPEN_FAILED    200
#define DRIVER_CLOSE_FAILED   201
#define DRIVER_NO_SPACE       202

#define FS_ALLOC_ZONE_MM_FAILED 301
#define FS_OUT_OF_ZONE_INDEX    302
#define FS_LOAD_INFO_OF_MBR_IS_FAILED  303


#define TH_CREAT_FAILED           401

#endif

#ifndef __TK_SATA_H
#define __TK_SATA_H
#include "err.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/mman.h>
#include <pthread.h>
#include "libaxidma.h"
#include <string.h>
/*
   memory 
*/
#define __MM_PATH_   "/dev/mem"                //PATH
#define __MM_SIZE_    32                       //SIZE B
#define __PHY_ADDR_   0X0000                   //PHY ADDR
#define __READ(BASE,OFFSET,VALUE)  VALUE=(*((uint32_t *) (BASE + OFFSET)))
#define __WRIT(BASE,OFFSET,VALUE)  (*((uint32_t *) (BASE + OFFSET)))=VALUE 

/*

This structure encapsulates a series of SATA function operations.

*/

typedef  struct {

	int fd;
        uint64_t  size;
	uint64_t pluse_size;
	unsigned char * base;
	uint64_t blk_size;
        axidma_dev_t axi_dev;
	unsigned char *in_buf, * out_buf;
	uint32_t  send_ch,recv_ch;
	pthread_mutex_t mutex_lock;

}sata_ops_t, * p_sata_ops_t ;




uint32_t tk_init_sata_lib(p_sata_ops_t * out_hdev);


uint32_t tk_uninit_sata_lib(p_sata_ops_t hdev);


uint32_t tk_get_sata_status(p_sata_ops_t hdev);


uint64_t tk_get_sata_size(p_sata_ops_t hdev);


uint32_t tk_write_sata_stop(p_sata_ops_t hdev);


uint32_t tk_read_sata_stop(p_sata_ops_t hdev);


uint32_t tk_have_write_sata_size(p_sata_ops_t hdev);


uint32_t tk_have_read_sata_size(p_sata_ops_t hdev);


uint32_t tk_read_disk(p_sata_ops_t hdev,unsigned char * out_buf ,uint64_t size,uint64_t addr);


uint32_t tk_write_disk(p_sata_ops_t hdev,unsigned char * in_buf ,uint64_t addr,uint64_t size);


#endif

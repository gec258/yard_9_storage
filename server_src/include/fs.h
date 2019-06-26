/*

 This is a file system used to resolve fpga disk data information. 

*/
#ifndef __FS_H_
#define __FS_H_

#include "public.h"
#include "err.h"
#include "sata.h"
#include  "md5.h"
#define GB_TO_B(SIZE) (SIZE<<30)
typedef struct 
{

uint64_t index;	
uint64_t star_addr;
uint64_t size;
uint64_t end_addr;
uint64_t now_pos;

}fs_zone_t,*p_fs_zone_t;


typedef struct 
{
//   unsigned char magic[8];
   uint64_t disk_size;
   uint64_t used_size;
   uint64_t zone_num;
   uint64_t zone_now_pos;
   unsigned char md5_check[MD5_SIZE];
   p_fs_zone_t * zone;
   p_sata_ops_t hdev;


}fs_ops_t,*p_fs_ops_t;


/*
   file mate info struct
 */

typedef struct {

	// 2
	// 1
	// 1
	// 1
	// 1
	// 1
	// 2	

}file_name_t;



typedef struct 
{

	file_name_t file_name;
	uint64_t size;   /*file size*/
	uint64_t s_addr; /*file start addr*/
	uint64_t e_addr; /*file end addr*/

}file_mate_info_t ,*p_file_mate_info_t;




static uint32_t tk_init_a_zone (p_fs_ops_t fs,uint64_t zone_index,uint64_t zone_size);

static uint32_t auto_init_4_zone(p_fs_ops_t fs);

static uint64_t tk_fs_get_sata_status(p_fs_ops_t fs);

uint64_t tk_fs_get_disk_size(p_fs_ops_t fs);

static uint32_t  tk_fs_disk_info_load(p_fs_ops_t fs);

static uint32_t tk_fs_zone_info_load(p_fs_ops_t fs);

uint32_t tk_fs_driver_init(p_fs_ops_t fs);

uint32_t tk_fs_is_have_complete_info(p_fs_ops_t fs);

uint32_t  tk_fs_creat_zone(p_fs_ops_t fs,uint32_t auto_flag,uint64_t zone_num,...);

uint32_t tk_fs_uninit_driver_and_zone(p_fs_ops_t fs);

uint32_t tk_fs_load_zone_info(p_fs_ops_t fs);

uint32_t tk_fs_format_disk(p_fs_ops_t fs);

uint32_t tk_fs_first_init(p_fs_ops_t fs);

uint32_t tk_fs_write_data(p_fs_ops_t fs,unsigned char * data,uint64_t size,uint64_t addr);

uint32_t tk_fs_read_data(p_fs_ops_t fs,unsigned char * data,uint64_t size,uint64_t addr);

uint32_t tk_get_file_meta_info(uint32_t index , uint32_t num , file_mate_info_t * data);


#endif


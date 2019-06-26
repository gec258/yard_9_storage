#include "fs.h"
#include "md5.h"

#define FS_MAGIC_IS_FORMAT   "BADBEFF"
#define FS_START_ADDR         0X00
#define FS_ZONE_ADDR_START    0X400000UL

#define FS_MBR_CACLUTE_LEN  4*8
#define MD5_STR_LEN		(MD5_SIZE * 2)

/*
   |---------------------|
   |  0  |  1  | 2  |  3 |
   |---------------------|
   |---------------------|
 */

static uint32_t tk_init_a_zone (p_fs_ops_t fs,uint64_t zone_index,uint64_t zone_size)
{
	if (zone_index >= fs->zone_num)
		return  FS_OUT_OF_ZONE_INDEX;
	fs->zone[zone_index]->index=zone_index;	   
	fs->zone[zone_index]->size=zone_size;
	fs->zone[zone_index]->star_addr=fs->zone_now_pos;
	fs->zone[zone_index]->end_addr=fs->zone_now_pos+zone_size;
	fs->zone[zone_index]->now_pos=fs->zone_now_pos;
	fs->zone_now_pos+=zone_size;

	return SUCCESS;

}

/*

   Automatic initialization of four spaces for disk partitioning

 */

static uint32_t auto_init_4_zone(p_fs_ops_t fs)
{
	uint64_t mate_size=fs->disk_size/sizeof(file_mate_info_t);
	uint64_t save_size= GB_TO_B(4);
	uint64_t surplus_cap= fs->disk_size-save_size-mate_size;
	uint64_t half_surplus=surplus_cap/2;
	tk_init_a_zone (fs,0,save_size);
	tk_init_a_zone (fs,1,mate_size);
	tk_init_a_zone (fs,2,surplus_cap/2);
	tk_init_a_zone (fs,3,surplus_cap-half_surplus);

	return SUCCESS;
}



static uint64_t tk_fs_get_sata_status(p_fs_ops_t fs)
{
	TK_CHECK(fs&&fs->hdev);
	return tk_get_sata_status(fs->hdev);
}



uint64_t tk_fs_get_disk_size(p_fs_ops_t fs)
{
	TK_CHECK(fs&&fs->hdev);
	return  tk_get_sata_size(fs->hdev);    
}


static uint32_t  tk_fs_disk_info_load(p_fs_ops_t fs)
{
	uint32_t status;
	TK_CHECK(fs);
	fs_ops_t mbr;
	fs->disk_size=tk_fs_get_disk_size(fs);
	if (status =tk_fs_read_data(fs,(unsigned char *)&mbr,sizeof(fs_ops_t),FS_START_ADDR),status!=SUCCESS)
		return status;
	fs->disk_size=mbr.disk_size;
	fs->used_size=mbr.used_size;	   
	fs->zone_num=mbr.zone_num;
	fs->zone_now_pos=mbr.zone_now_pos;


	uint64_t size=fs->zone_num* sizeof (fs_zone_t) ;

	fs->zone=(p_fs_zone_t *)malloc(size);  
	if(!fs->zone)
	{
		ERR_DIS("alloc zone of disk failed \n");
		status=FS_ALLOC_ZONE_MM_FAILED;
		return status;
	}


	return SUCCESS;
}

static uint32_t tk_fs_zone_info_load(p_fs_ops_t fs)
{
	uint32_t status;
	TK_CHECK(fs);
	uint64_t size=fs->zone_num* sizeof (fs_zone_t),i ;
	TK_CHECK(fs);

	for(i=0;i<fs->zone_num;i++)
		if (status =tk_fs_read_data(fs,(unsigned char *)(fs->zone[i]),size,FS_ZONE_ADDR_START),status!=SUCCESS)
			return status;

	return SUCCESS;
}


/*

1: 文件系统底层磁盘驱动初始化

 */
uint32_t tk_fs_driver_init(p_fs_ops_t fs)
{
	uint32_t status;
	uint64_t i;
	TK_CHECK(fs);
	fs->zone=NULL;
	if(status=tk_init_sata_lib(&fs->hdev),status!=SUCCESS)
	{
		ERR_DIS("fs init sata driver failed\n");
		goto fs_out;
	}


	return SUCCESS;
fs_out:
	return status;	

}

/*

2:检查文件系统信息是否完整

 */

uint32_t tk_fs_is_have_complete_info(p_fs_ops_t fs)
{
	unsigned char md5_value[MD5_SIZE];
	uint32_t status,i;   
	fs_ops_t mbr;
	unsigned char * data= (unsigned char *)&mbr;
	MD5_CTX md5;
	if (status =tk_fs_get_sata_status(fs),status!=SUCCESS)
		return status;
	MD5Init(&md5);
	if (status =tk_fs_read_data(fs,data,sizeof(fs_ops_t),FS_START_ADDR),status!=SUCCESS)
		return status;
	MD5Update(&md5, data, FS_MBR_CACLUTE_LEN);
	MD5Final(&md5, md5_value);
	for(i = 0; i < MD5_SIZE; i++)
	{
		if(md5_value[i]!=mbr.md5_check[i]);
		return FS_LOAD_INFO_OF_MBR_IS_FAILED; 
	}
	return SUCCESS;
}



/*

   创建文件系统zone

 */
uint32_t  tk_fs_creat_zone(p_fs_ops_t fs,uint32_t auto_flag,uint64_t zone_num,...)
{
	uint32_t status;
	uint64_t i,size=zone_num* sizeof (fs_zone_t) ;
	va_list argptr;
	TK_CHECK(fs);

	fs->zone=(p_fs_zone_t *)malloc(size);  
	if(!fs->zone)
	{
		ERR_DIS("alloc zone of disk failed \n");
		status=FS_ALLOC_ZONE_MM_FAILED;
		goto fs_zone_err;
	}

	/*init every zone size,if auto_flag is set , will auto cut space !*/

	if(!auto_flag)
	{
		fs->zone_num=zone_num;
		va_start(argptr,zone_num);   
		for ( i = 0; i < zone_num; ++i ) 
			tk_init_a_zone (fs,i,va_arg(argptr,uint64_t));
		va_end( argptr );
	}
	else
	{
		fs->zone_num=4;
		auto_init_4_zone(fs);
	}
	/*debug --*/
	for(i=0;i<zone_num;i++)
		printf("-----------------%lu:    %lu   \n-------",i,fs->zone[i]->size);


	return SUCCESS;
fs_zone_err:
	return status;
}


/*

   卸载磁盘驱动和文件系统zone

 */
uint32_t tk_fs_uninit_driver_and_zone(p_fs_ops_t fs)
{
	uint32_t status;
	TK_CHECK(fs&&fs->hdev);
	status = tk_uninit_sata_lib(fs->hdev);
	if(fs->zone)
		free(fs->zone);
	return status;
}


/*

   加载文件系统信息

 */


uint32_t tk_fs_load_zone_info(p_fs_ops_t fs)
{
	uint32_t status;
	if (status=tk_fs_disk_info_load(fs),status!=SUCCESS)
		return status;

	if (status=tk_fs_zone_info_load(fs),status!=SUCCESS)
		return status;

	return SUCCESS;
}



/*

   文件系统格式化

 */
uint32_t tk_fs_format_disk(p_fs_ops_t fs)
{

	TK_CHECK(fs&&fs->hdev);
	uint32_t i=0;
	for(i=0;i<fs->zone_num;i++)
		fs->zone[i]->now_pos=fs->zone[i]->star_addr;
	fs->used_size=0;


	return SUCCESS;
}


/*

   首次使用初始化文件系统

 */
uint32_t tk_fs_first_init(p_fs_ops_t fs)
{
	uint32_t status;
	//if (status=tk_fs_driver_init(fs),status!=SUCCESS)
	//goto out_driver;

	if (status=tk_fs_creat_zone(fs,1,0),status!=SUCCESS)
		goto err_creat_zone;

	if(tk_fs_format_disk(fs),status!=SUCCESS)
		goto err_format_disk;


	return SUCCESS;
err_format_disk:

err_creat_zone:
	tk_fs_uninit_driver_and_zone(fs);
out_driver:
	return status;
}




uint32_t tk_fs_write_data(p_fs_ops_t fs,unsigned char * data,uint64_t size,uint64_t addr)
{
	TK_CHECK(fs&&fs->hdev&&data&&size); 
	return SUCCESS;
}


uint32_t tk_fs_read_data(p_fs_ops_t fs,unsigned char * data,uint64_t size,uint64_t addr)
{
	TK_CHECK(fs&&fs->hdev&&data&&size); 
	return SUCCESS;
}



uint32_t tk_get_file_meta_info(uint32_t index , uint32_t num , file_mate_info_t * data)
{
	TK_CHECK(data);


	return SUCCESS;
}

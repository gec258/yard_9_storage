#include "sata.h"


static tk_init_dma(p_sata_ops_t hdev)
{

	hdev->axi_dev = axidma_init();
	const array_t *tx_chans, *rx_chans;
	if (!hdev->axi_dev) 
	{
		ERR_DIS("Failed to initialize the AXI DMA device.\n");
		goto out;
	}

	hdev->in_buf = axidma_malloc(hdev->axi_dev, hdev->blk_size);
	if (!hdev->in_buf)
	{
		ERR_DIS("Failed to allocate the in buffer.\n");
		goto err_inbuf;
	}

	hdev->out_buf = axidma_malloc(hdev->axi_dev, hdev->blk_size);
	if (!hdev->out_buf)
	{
		ERR_DIS("Failed to allocate the out buffer.\n");
		goto err_outbuf;
	}



	// Get the tx and rx channels if they're not already specified
	tx_chans = axidma_get_dma_tx(hdev->axi_dev);
	if (tx_chans->len < 1) {
		ERR_DIS("Error: No transmit channels were found.\n");

		goto err_get;
	}
	rx_chans = axidma_get_dma_rx(hdev->axi_dev);
	if (rx_chans->len < 1) {
		ERR_DIS("Error: No receive channels were found.\n");
		goto err_get;
	}

	hdev->recv_ch=rx_chans->data[0];
        hdev->send_ch=tx_chans->data[0];

	return SUCCESS ;
err_get:
	axidma_free(hdev->axi_dev, hdev->out_buf, hdev->blk_size);
err_outbuf:
	axidma_free(hdev->axi_dev, hdev->in_buf, hdev->blk_size);
err_inbuf:
	axidma_destroy(hdev->axi_dev);
out:
	return FAILED;
}


static tk_uninit_dma(p_sata_ops_t hdev)
{
	axidma_free(hdev->axi_dev, hdev->in_buf, hdev->blk_size);
	axidma_free(hdev->axi_dev, hdev->out_buf, hdev->blk_size);
	axidma_destroy(hdev->axi_dev);

	return SUCCESS;
}


/*


   This is a init lib function 

 */

#define ONCE_DMA_BLK_SIZE    4096

uint32_t tk_init_sata_lib(p_sata_ops_t * out_hdev)
{	

	p_sata_ops_t hdev;

	hdev = (p_sata_ops_t)malloc (sizeof(sata_ops_t));
	if (!hdev)
	{   
		ERR_DIS("Malloc memory failed\n");
		goto err_out;
	}


	if (hdev->fd = open(__MM_PATH_,O_RDWR),hdev->fd<0);   
	{
		ERR_DIS("Open device failed\n");
		hdev = NULL;
		goto err_open;
	}


	hdev->base = (unsigned char *)mmap( 0,__MM_SIZE_, PROT_READ | PROT_WRITE, MAP_SHARED,hdev->fd, __PHY_ADDR_ );  
	hdev->size= __MM_SIZE_ ;
	if(!hdev->base)
	{   
		ERR_DIS(" Base mmap is failed\n");   
		goto err_mmap;
	}   
	/* Set dma once block size */
	hdev->blk_size =ONCE_DMA_BLK_SIZE;


	if (tk_init_dma(hdev)!=SUCCESS)
		goto err_axi;

	pthread_mutex_init(&hdev->mutex_lock,NULL);

	*out_hdev=hdev;

	return  DRIVER_SUCCESS; 
err_axi:
	munmap(hdev->base,hdev->size);
err_mmap:
	close (hdev->fd);
err_open:
	free(hdev);
err_out:
	return DRIVER_OPEN_FAILED;
}






/*



 */

uint32_t tk_uninit_sata_lib(p_sata_ops_t hdev)
{
	if (!hdev || !hdev->fd)
		return DRIVER_CLOSE_FAILED;
	tk_uninit_dma(hdev);
	munmap(hdev->base,hdev->size);
	close(hdev->fd);
	pthread_mutex_destroy(&hdev->mutex_lock);
	free(hdev);
	hdev=NULL;
	return DRIVER_SUCCESS;
}





/*

   get disk status 

 */
uint32_t tk_get_sata_status(p_sata_ops_t hdev)
{

	uint32_t status ;
	__READ(hdev->base, 0x00 ,status);
	return status & 0x1;
}


/*

   get  sata size

 */
uint64_t tk_get_sata_size(p_sata_ops_t hdev)
{

	uint32_t low_32,hig_16; 
	uint64_t size=0;
	__READ(hdev->base, 0x04 ,low_32);//32 low          
	__READ(hdev->base, 0x08 ,hig_16);//16 hig
	size=  hig_16;
	size = size<<32|low_32;

	return size  ;
}




/*

   start sata write

 */
static uint32_t tk_write_sata_start(p_sata_ops_t hdev)
{
	__WRIT(hdev->base,0x00,0X1);
	__WRIT(hdev->base,0x00,0X0);

	return 0;
}

/*

   stop sata write

 */

uint32_t tk_write_sata_stop(p_sata_ops_t hdev)
{
	__WRIT(hdev->base,0x04,0X1);
	return 0;
}	



/*

 */
static uint32_t tk_read_sata_start(p_sata_ops_t hdev)
{
	__WRIT(hdev->base,0x08,0X1);
	__WRIT(hdev->base,0x08,0X0);
	return 0;
}	



uint32_t tk_read_sata_stop(p_sata_ops_t hdev)
{
	__WRIT(hdev->base,0x0c,0X1);
	return 0;
}




//设置当前读/写数据大小,

static uint32_t _tk_have_read_write_sata_size(p_sata_ops_t hdev,uint64_t size,uint64_t addr)
{
	uint32_t low_32,hig_16; 
	low_32= size&(uint32_t)0xffffffff;
	hig_16=(size>>32)&0xffff;
	__WRIT(hdev->base, 0x10 ,low_32);//32 low          
	__WRIT(hdev->base, 0x14 ,hig_16);//16 hig
	low_32= addr&(uint32_t)0xffffffff;
	hig_16=(addr>>32)&0xffff;
	__WRIT(hdev->base, 0x18 ,low_32);//32 low          
	__WRIT(hdev->base, 0x1c ,hig_16);//16 hig

	return 0  ;
}




//已写入数据大小
uint32_t tk_have_write_sata_size(p_sata_ops_t hdev)
{
	uint32_t low_32,hig_16; 
	uint64_t size; 
	__READ(hdev->base, 0x18 ,low_32);//32 low          
	__READ(hdev->base, 0x1c ,hig_16);//16 hig
	size=  hig_16;
	size = size<<32|low_32;
	return size  ;
}
//已读出数据大小
uint32_t tk_have_read_sata_size(p_sata_ops_t hdev)
{
	uint32_t low_32,hig_16; 
	uint64_t size=0;
	__READ(hdev->base, 0x10 ,low_32);//32 low          
	__READ(hdev->base, 0x14 ,hig_16);//16 hig
	size=  hig_16;
	size = size<<32|low_32;
	return size  ;
}



uint32_t tk_write_disk(p_sata_ops_t hdev,unsigned char * in_buf ,uint64_t size,uint64_t addr)
{

	if(!size)
		return SUCCESS;

	uint64_t blk_size = hdev->blk_size;
	uint64_t wr_cnt = size/ blk_size;
	uint64_t i,surplus;

	if(surplus=size%blk_size)
		wr_cnt+=1;
	if(wr_cnt*blk_size > hdev->pluse_size)	
		return DRIVER_NO_SPACE;	       


	for(i=0;i<wr_cnt;i++,in_buf+=blk_size)
	{
		pthread_mutex_lock(&hdev->mutex_lock);
		_tk_have_read_write_sata_size(hdev,blk_size,addr);
		pthread_mutex_unlock(&hdev->mutex_lock);
		if((i==wr_cnt-1) && surplus)
			memcpy(hdev->in_buf,in_buf,surplus);
		else
			memcpy(hdev->in_buf,in_buf,blk_size);
		if (axidma_oneway_transfer(hdev->axi_dev, hdev->send_ch, hdev->in_buf,blk_size, true)<0)
			return FAILED;
		/* check is write ok */

	}

	return SUCCESS ;
}

uint32_t tk_read_disk(p_sata_ops_t hdev, unsigned char * out_buf ,uint64_t size , uint64_t addr)
{

	if(!size)
		return SUCCESS;

	uint64_t blk_size = hdev->blk_size;
	uint64_t rd_cnt = size/ blk_size;
	uint64_t i,surplus;

	if(surplus=size%blk_size,surplus)
	{
		rd_cnt+=1;
	}
	for(i=0;i<rd_cnt;i++,out_buf+=blk_size)
	{
		pthread_mutex_lock(&hdev->mutex_lock);
		_tk_have_read_write_sata_size(hdev,size,addr);
		pthread_mutex_unlock(&hdev->mutex_lock);
		if (axidma_oneway_transfer(hdev->axi_dev,hdev->recv_ch, hdev->out_buf,blk_size, true)<0)
			return FAILED;
		if((i==rd_cnt-1) && surplus)
			memcpy(out_buf,hdev->out_buf,surplus);
		else
			memcpy(out_buf,hdev->out_buf,blk_size);
	}

	return SUCCESS  ;
}




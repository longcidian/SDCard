/*******************************************************************************
* Name		: SD card
* Brief		: access an SD card
* Author	: longcd
* LastEdit	: 2017/8/8
*******************************************************************************/

#ifndef SD_CARD_H
#define SD_CARD_H

//SD card return
typedef enum{
	SD_RET_OK,				//OK
	SD_ENTER_IDLE_ERR,		//can not enter IDLE
	SD_TYPE_ERR,			//not V2.0 SD card
	SD_INIT_ERR,
	SD_CMD17_ERR,			//CMD17 not replied
	SD_CMD24_ERR,
}SDCard_ret_t;


SDCard_ret_t SDCard_init(void);
SDCard_ret_t SDCard_readCSD(void);
SDCard_ret_t SDCard_readBlock(u32 block,u8 *buf);
SDCard_ret_t SDCard_writeBlock(u32 block, u8 *buf);

//for FatFs
u8 MMC_disk_status(void);
SDCard_ret_t MMC_disk_initialize(void);
SDCard_ret_t MMC_disk_read(u8 *buff,u32 sector, u32 count);
SDCard_ret_t MMC_disk_write(const u8 *buff, u32 sector, u32 count);

#endif

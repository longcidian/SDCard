/*******************************************************************************
* Name		: SD card
* Brief		: access an SD card
* Author	: longcd
* LastEdit	: 2017/8/8
********************************************************************************
*/

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#include "SPI.h"

#include "SDCard.h"

////////////////////////////////////////////////////////////////////////////////
// bus define

//SPI define
#define SD_SPI_writeByte(byte)		SPI_writeByte(byte)
#define SD_SPI_readByte(byte)		SPI_readByte(byte)
#define SD_SPI_CS_low()				SPI_CS_enable()
#define SD_SPI_CS_high()			SPI_CS_disable()

////////////////////////////////////////////////////////////////////////////////
// cmd define
#define CMD0		(0X40 + 0)	//reset SD card
#define CMD8		(0X40 + 8)
#define CMD9		(0X40 + 9)	//read CSD register
#define CMD17		(0X40 + 17)
#define CMD24		(0X40 + 24)
#define CMD41		(0X40 + 41)
#define CMD55		(0X40 + 55)
#define CMD58		(0X40 + 58)

////////////////////////////////////////////////////////////////////////////////
//

#define SD_IDLE_OK		0X01
#define DUMMY_BYTE		0XFF

////////////////////////////////////////////////////////////////////////////////

typedef struct{
	u32		byteVolume;
}SDCard_t;

////////////////////////////////////////////////////////////////////////////////

SDCard_t SDCard;

////////////////////////////////////////////////////////////////////////////////

/*	send cmd to SD card
 */
static void SDCard_sendCmd(u8 cmd,u32 arg,u8 crc)
{
	SD_SPI_writeByte(cmd);
	SD_SPI_writeByte(arg >> 24);
	SD_SPI_writeByte(arg >> 16);
	SD_SPI_writeByte(arg >> 8);
	SD_SPI_writeByte(arg);
	SD_SPI_writeByte(crc);
}

/*	init SD card to SPI mode
 */
SDCard_ret_t SDCard_init(void)
{
	u8 i;
	u8 retry;
	u8 ret;
	u8 buf[8];
	
	//send at least 74 clocks
	SD_SPI_CS_low();
	for(i = 0; i < 10; i++){
		SD_SPI_writeByte(0xff);
	}

	//enter IDLE
	retry = 0;
	do{
		SDCard_sendCmd(CMD0,0,0x95);
		for(i = 0; i < 10; i++){
			ret = SD_SPI_readByte();
			if(ret == SD_IDLE_OK) break;
		}

		retry++;
	}while((ret != SD_IDLE_OK) && (retry < 200));
	
	if(ret != SD_IDLE_OK){		//fail to enter IDLE
		return SD_ENTER_IDLE_ERR;
	}
	
	//query card type
	SDCard_sendCmd(CMD8,0X000001AA,0X87);
	for(i = 0; i < 8; i++){
		buf[i] = SD_SPI_readByte();
	}
	
	if(buf[1] == 0x01){			//v2.00 or later card
		if(buf[4] == 0x01 &&	//support 2.7-3.6V
		   buf[5] == 0xaa)		//check pattern match
		{
			
		}
	} else {	//v1.x or not SD card
		return SD_TYPE_ERR;
	}
	
	//wait card ready
	retry = 0;
	do{
		SDCard_sendCmd(CMD55,0,0X01);
		buf[0] = SD_SPI_readByte();
		buf[1] = SD_SPI_readByte();
		SDCard_sendCmd(CMD41,0X40000000,0x01);
		for(i = 0; i < 8; i++){
			buf[i] = SD_SPI_readByte();
		}
		
		retry++;
	}while((retry < 200) && (buf[1] != 0));
	
	if(buf[1] != 0){
		return SD_INIT_ERR;
	}
	
	//get CCS
	SDCard_sendCmd(CMD58,0,0x01);
	for(i = 0; i < 8; i++){
		buf[i] = SD_SPI_readByte();
	}
	
	if(buf[2] == 0xc0){	//high capacity
	
	} else {	//standard capacity
	
	}
	
	SD_SPI_CS_high();
	return SD_RET_OK;
}

/*	¶ÁÈ¡CSD¼Ä´æÆ÷
 */
SDCard_ret_t SDCard_readCSD(void)
{
	u8 buf[32];
	u8 i;

	SD_SPI_CS_low();
	SDCard_sendCmd(CMD9,0,0xff);
	for(i = 0; i < 32; i++){
		buf[i] = SD_SPI_readByte();
	}
	
	if( (buf[4] >> 6) == 1){	//CSD V2.0/high capacity

	} else if( (buf[4] >> 6) == 0){	//CSD V1.01-1.10
	
	} else {	//reserved
	
	}
	
	SDCard.byteVolume = ( (u32)(buf[11] & 0x3f) << 16) + ((u32)buf[12] << 8) + buf[13];
	SDCard.byteVolume = (SDCard.byteVolume + 1) * 512;	//K byte
	
	SD_SPI_CS_high();
	return SD_RET_OK;
}

/*	read a block(512 bytes)
 */
SDCard_ret_t SDCard_readBlock(u32 block,u8 *buf)
{
	u16 i;
	u8 ret;
	
	SD_SPI_CS_low();
	
	//send cmd
	SDCard_sendCmd(CMD17,block,0xff);
	for(i = 0; i < 8; i++){
		ret = SD_SPI_readByte();
		if(ret == 0x00) break;
	}

	if(ret != 0x00){	//not replied?
		return SD_CMD17_ERR;
	}
	
	//wait start block token
	for(i = 0; i < 100; i++){
		ret = SD_SPI_readByte();
		if(ret == 0xfe) break;
	}
	
	if(ret != 0xfe){	//not receive start block token?
		return SD_CMD17_ERR;
	}
	
	//get block data
	for(i = 0; i < 512; i++){
		*buf++ = SD_SPI_readByte();
	}

	//get CRC
	for(i = 0; i < 2; i++){
		SD_SPI_readByte();
	}
	
	SD_SPI_CS_high();
	return SD_RET_OK;
}

/*	write a block(512 bytes)
 */
SDCard_ret_t SDCard_writeBlock(u32 block, u8 *buf)
{
	u16 i;
	u8 ret;
	
	SD_SPI_CS_low();
	
	//send cmd
	SDCard_sendCmd(CMD24,block,0xff);
	for(i = 0; i < 8; i++){
		ret = SD_SPI_readByte();
		if(ret == 0x00) break;
	}

	if(ret != 0x00){	//not replied?
		return SD_CMD24_ERR;
	}
	
	//send start block token
	for(i = 0; i < 50; i++){
		SD_SPI_writeByte(DUMMY_BYTE);
	}
	SD_SPI_writeByte(0xfe);
	
	//send block data
	for(i = 0; i < 512; i++){
		SD_SPI_writeByte(*buf++);
	}
	
	//send CRC
	SD_SPI_writeByte(DUMMY_BYTE);
	SD_SPI_writeByte(DUMMY_BYTE);
	
	//get response
	if((SD_SPI_readByte() & 0x1f) != 0x05){	//data not accepted?
		return SD_CMD24_ERR;
	}

	//wait write complete
	while(SD_SPI_readByte() != 0xff);

	SD_SPI_CS_high();
	return SD_RET_OK;
}

/*	used for FatFs
 */
u8 MMC_disk_status(void)
{
	//return 0 means OK for testing
	return 0;
}

/*	used for FatFs
 */
SDCard_ret_t MMC_disk_initialize(void)
{
	return(SDCard_init());
}

/*	used for FatFs
 */
SDCard_ret_t MMC_disk_read(u8 *buff,u32 sector, u32 count)
{
	SDCard_ret_t ret;
	
	while(count--){
		ret = SDCard_readBlock(sector, buff);
		sector += 512;
		if(ret != SD_RET_OK) break;
	}
	
	return ret;
}

/*	used for FatFs
 */
SDCard_ret_t MMC_disk_write(const u8 *buff, u32 sector, u32 count)
{
	SDCard_ret_t ret;
	
	while(count--){
		ret = SDCard_writeBlock(sector, (u8 *)buff);
		sector += 512;
		if(ret != SD_RET_OK) break;
	}

	return ret;
}

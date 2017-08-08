/*******************************************************************************
* Name		: SPI
* Brief		: GPIO simulate SPI bus
* Author	: longcd
* LastEdit	: 2017/8/8
********************************************************************************
*/

//MCU
#include "stm32f10x.h"

#include "SPI.h"

////////////////////////////////////////////////////////////////////////////////
// pin define

#define SPI_CS_low()		GPIO_ResetBits(GPIOA,GPIO_Pin_6)
#define SPI_CS_high()		GPIO_SetBits(GPIOA,GPIO_Pin_6)
#define SPI_CLK_low()		GPIO_ResetBits(GPIOA,GPIO_Pin_2)
#define SPI_CLK_high()		GPIO_SetBits(GPIOA,GPIO_Pin_2)
#define SPI_MOSI_low()		GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define SPI_MOSI_high()		GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define SPI_MISO_inHigh()	(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != RESET)

////////////////////////////////////////////////////////////////////////////////

static void SPI_delay(void)
{
	//adjust SPI speed
}

void SPI_init(void)
{
	GPIO_InitTypeDef	GPIO_init;
	
	//enable SPI port clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	//SPI CS
	GPIO_init.GPIO_Pin = GPIO_Pin_6;
	GPIO_init.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_init);

	//SPI CLK
	GPIO_init.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA,&GPIO_init);

	//SPI MOSI
	GPIO_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA,&GPIO_init);
	
	//SPI MISO
	GPIO_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_init);
	
	SPI_CS_high();
	SPI_CLK_high();
}

void SPI_writeByte(u8 byte)
{
	u8 i;
	
	for(i = 0; i < 8; i++){
		SPI_CLK_low();
		if(byte & 0x80){
			SPI_MOSI_high();
		} else {
			SPI_MOSI_low();
		}
		
		SPI_delay();
		SPI_CLK_high();
		SPI_delay();
		byte <<= 1;
	}
}

u8 SPI_readByte(void)
{
	u8 i;
	u8 dat;
	
	for(i = 0; i < 8; i++){
		SPI_CLK_low();
		SPI_delay();
		SPI_CLK_high();
		SPI_delay();
		dat <<= 1;
		if( SPI_MISO_inHigh() ){
			dat++;
		}
	}

	return dat;
}

void SPI_CS_enable(void)
{
	SPI_CS_low();
}

void SPI_CS_disable(void)
{
	SPI_CS_high();
}

/*******************************************************************************
* Name		: SPI
* Brief		: GPIO simulate SPI bus
* Author	: longcd
* LastEdit	: 2017/8/8
********************************************************************************
*/

#ifndef SPI_H
#define SPI_H

void SPI_init(void);
void SPI_writeByte(u8 byte);
u8 SPI_readByte(void);
void SPI_CS_enable(void);
void SPI_CS_disable(void);

#endif

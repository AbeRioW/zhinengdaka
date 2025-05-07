#include <string.h>
#include "main.h"
#include "AS608.h"
#include "usart.h"
uint32_t AS608Addr = 0XFFFFFFFF; //é»˜è®¤
extern uint8_t aRxBuffer[RXBUFFERSIZE];

//ä¸²å£å‘é€ä¸€ä¸ªå­—èŠ‚
static void Com_SendData(uint8_t data)
{
	HAL_UART_Transmit(&huart1,&data, 1,50);
}

//å‘é€åŒ…å¤´
static void SendHead(void)
{
	Com_SendData(0xEF);
	Com_SendData(0x01);
}

//å‘é€åœ°å€
static void SendAddr(void)
{
	Com_SendData(AS608Addr>>24);
	Com_SendData(AS608Addr>>16);
	Com_SendData(AS608Addr>>8);
	Com_SendData(AS608Addr);
}
//å‘é€åŒ…æ ‡è¯†,
static void SendFlag(uint8_t flag)
{
	Com_SendData(flag);
}
//å‘é€åŒ…é•¿åº¦
static void SendLength(int length)
{
	Com_SendData(length>>8);
	Com_SendData(length);
}
//·¢ËÍÖ¸ÁîÂë
static void Sendcmd(uint8_t cmd)
{
	Com_SendData(cmd);
}
//·¢ËÍĞ£ÑéºÍ
static void SendCheck(uint16_t check)
{
	Com_SendData(check>>8);
	Com_SendData(check);
}
//ÅĞ¶ÏÖĞ¶Ï½ÓÊÕµÄÊı×éÓĞÃ»ÓĞÓ¦´ğ°ü
//waittimeÎªµÈ´ıÖĞ¶Ï½ÓÊÕÊı¾İµÄÊ±¼ä£¨µ¥Î»1ms£©
//·µ»ØÖµ£ºÊı¾İ°üÊ×µØÖ·
extern uint8_t RX_len;//½ÓÊÕ×Ö½Ú¼ÆÊı
static uint8_t *JudgeStr(uint16_t waittime)
{
	char *data;
	uint8_t str[8];
	str[0]=0xef;str[1]=0x01;str[2]=AS608Addr>>24;
	str[3]=AS608Addr>>16;str[4]=AS608Addr>>8;
	str[5]=AS608Addr;str[6]=0x07;str[7]='\0';
	
	while(--waittime)
	{
		HAL_Delay(1);
			if(RX_len)//½ÓÊÕµ½Ò»´ÎÊı¾İ
		{
			RX_len=0;
			data=strstr((const char*)&aRxBuffer,(const char*)str);
			if(data)
				return (uint8_t*)data;	
		}
	}
	return 0;
}
//Â¼ÈëÍ¼Ïñ GZ_GetImage
//¹¦ÄÜ:Ì½²âÊÖÖ¸£¬Ì½²âµ½ºóÂ¼ÈëÖ¸ÎÆÍ¼Ïñ´æÓÚImageBuffer¡£ 
//Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_GetImage(void)
{
  uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x01);
  temp =  0x01+0x03+0x01;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//Éú³ÉÌØÕ÷ GZ_GenChar
//¹¦ÄÜ:½«ImageBufferÖĞµÄÔ­Ê¼Í¼ÏñÉú³ÉÖ¸ÎÆÌØÕ÷ÎÄ¼ş´æÓÚCharBuffer1»òCharBuffer2			 
//²ÎÊı:BufferID --> charBuffer1:0x01	charBuffer1:0x02												
//Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_GenChar(uint8_t BufferID)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x04);
	Sendcmd(0x02);
	Com_SendData(BufferID);
	temp = 0x01+0x04+0x02+BufferID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//¾«È·±È¶ÔÁ½Ã¶Ö¸ÎÆÌØÕ÷ GZ_Match
//¹¦ÄÜ:¾«È·±È¶ÔCharBuffer1 ÓëCharBuffer2 ÖĞµÄÌØÕ÷ÎÄ¼ş 
//Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_Match(void)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x03);
	temp = 0x01+0x03+0x03;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//ËÑË÷Ö¸ÎÆ GZ_Search
//¹¦ÄÜ:ÒÔCharBuffer1»òCharBuffer2ÖĞµÄÌØÕ÷ÎÄ¼şËÑË÷Õû¸ö»ò²¿·ÖÖ¸ÎÆ¿â.ÈôËÑË÷µ½£¬Ôò·µ»ØÒ³Âë¡£			
//²ÎÊı:  BufferID @ref CharBuffer1	CharBuffer2
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö£¬Ò³Âë£¨ÏàÅäÖ¸ÎÆÄ£°å£©
uint8_t GZ_Search(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x08);
	Sendcmd(0x04);
	Com_SendData(BufferID);
	Com_SendData(StartPage>>8);
	Com_SendData(StartPage);
	Com_SendData(PageNum>>8);
	Com_SendData(PageNum);
	temp = 0x01+0x08+0x04+BufferID
	+(StartPage>>8)+(uint8_t)StartPage
	+(PageNum>>8)+(uint8_t)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID   =(data[10]<<8)+data[11];
		p->mathscore=(data[12]<<8)+data[13];	
	}
	else
		ensure = 0xff;
	return ensure;	
}
//ºÏ²¢ÌØÕ÷£¨Éú³ÉÄ£°å£©GZ_RegModel
//¹¦ÄÜ:½«CharBuffer1ÓëCharBuffer2ÖĞµÄÌØÕ÷ÎÄ¼şºÏ²¢Éú³É Ä£°å,½á¹û´æÓÚCharBuffer1ÓëCharBuffer2	
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_RegModel(void)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x05);
	temp = 0x01+0x03+0x05;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;		
}
//´¢´æÄ£°å GZ_StoreChar
//¹¦ÄÜ:½« CharBuffer1 »ò CharBuffer2 ÖĞµÄÄ£°åÎÄ¼ş´æµ½ PageID ºÅflashÊı¾İ¿âÎ»ÖÃ¡£			
//²ÎÊı:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID£¨Ö¸ÎÆ¿âÎ»ÖÃºÅ£©
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_StoreChar(uint8_t BufferID,uint16_t PageID)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x06);
	Sendcmd(0x06);
	Com_SendData(BufferID);
	Com_SendData(PageID>>8);
	Com_SendData(PageID);
	temp = 0x01+0x06+0x06+BufferID
	+(PageID>>8)+(uint8_t)PageID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}
//É¾³ıÄ£°å GZ_DeletChar
//¹¦ÄÜ:  É¾³ıflashÊı¾İ¿âÖĞÖ¸¶¨IDºÅ¿ªÊ¼µÄN¸öÖ¸ÎÆÄ£°å
//²ÎÊı:  PageID(Ö¸ÎÆ¿âÄ£°åºÅ)£¬NÉ¾³ıµÄÄ£°å¸öÊı¡£
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_DeletChar(uint16_t PageID,uint16_t N)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x07);
	Sendcmd(0x0C);
	Com_SendData(PageID>>8);
	Com_SendData(PageID);
	Com_SendData(N>>8);
	Com_SendData(N);
	temp = 0x01+0x07+0x0C+(PageID>>8)+(uint8_t)PageID+(N>>8)+(uint8_t)N;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//Çå¿ÕÖ¸ÎÆ¿â GZ_Empty
//¹¦ÄÜ:  É¾³ıflashÊı¾İ¿âÖĞËùÓĞÖ¸ÎÆÄ£°å
//²ÎÊı:  ÎŞ
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_Empty(void)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x0D);
	temp = 0x01+0x03+0x0D;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//Ğ´ÏµÍ³¼Ä´æÆ÷ GZ_WriteReg
//¹¦ÄÜ:  Ğ´Ä£¿é¼Ä´æÆ÷
//²ÎÊı:  ¼Ä´æÆ÷ĞòºÅRegNum:4\5\6
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_WriteReg(uint8_t RegNum,uint8_t DATA)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x05);
	Sendcmd(0x0E);
	Com_SendData(RegNum);
	Com_SendData(DATA);
	temp = RegNum+DATA+0x01+0x05+0x0E;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//¶ÁÏµÍ³»ù±¾²ÎÊı GZ_ReadSysPara
//¹¦ÄÜ:  ¶ÁÈ¡Ä£¿éµÄ»ù±¾²ÎÊı£¨²¨ÌØÂÊ£¬°ü´óĞ¡µÈ)
//²ÎÊı:  ÎŞ
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö + »ù±¾²ÎÊı£¨16bytes£©
uint8_t GZ_ReadSysPara(SysPara *p)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x0F);
	temp = 0x01+0x03+0x0F;
	SendCheck(temp);
	data=JudgeStr(1000);
	if(data)
	{
		ensure = data[9];
		p->GZ_max = (data[14]<<8)+data[15];
		p->GZ_level = data[17];
		p->GZ_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
		p->GZ_size = data[23];
		p->GZ_N = data[25];
	}		
	else
		ensure=0xff;
	return ensure;
}
//ÉèÖÃÄ£¿éµØÖ· GZ_SetAddr
//¹¦ÄÜ:  ÉèÖÃÄ£¿éµØÖ·
//²ÎÊı:  GZ_addr
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_SetAddr(uint32_t GZ_addr)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x07);
	Sendcmd(0x15);
	Com_SendData(GZ_addr>>24);
	Com_SendData(GZ_addr>>16);
	Com_SendData(GZ_addr>>8);
	Com_SendData(GZ_addr);
	temp = 0x01+0x07+0x15
	+(uint8_t)(GZ_addr>>24)+(uint8_t)(GZ_addr>>16)
	+(uint8_t)(GZ_addr>>8) +(uint8_t)GZ_addr;				
	SendCheck(temp);
	AS608Addr=GZ_addr;//·¢ËÍÍêÖ¸Áî£¬¸ü»»µØÖ·
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
		AS608Addr = GZ_addr;
	if(ensure==0x00)//ÉèÖÃµØÖ·³É¹¦
	{
		
	}
	return ensure;
}
//¹¦ÄÜ£º Ä£¿éÄÚ²¿ÎªÓÃ»§¿ª±ÙÁË256bytesµÄFLASH¿Õ¼äÓÃÓÚ´æÓÃ»§¼ÇÊÂ±¾,
//	¸Ã¼ÇÊÂ±¾Âß¼­ÉÏ±»·Ö³É 16 ¸öÒ³¡£
//²ÎÊı:  NotePageNum(0~15),Byte32(ÒªĞ´ÈëÄÚÈİ£¬32¸ö×Ö½Ú)
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö
uint8_t GZ_WriteNotepad(uint8_t NotePageNum,uint8_t *Byte32)
{
	uint16_t temp;
  uint8_t  ensure,i;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(36);
	Sendcmd(0x18);
	Com_SendData(NotePageNum);
	for(i=0;i<32;i++)
	 {
		 Com_SendData(Byte32[i]);
		 temp += Byte32[i];
	 }
  temp =0x01+36+0x18+NotePageNum+temp;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//¶Á¼ÇÊÂGZ_ReadNotepad
//¹¦ÄÜ£º  ¶ÁÈ¡FLASHÓÃ»§ÇøµÄ128bytesÊı¾İ
//²ÎÊı:  NotePageNum(0~15)
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö+ÓÃ»§ĞÅÏ¢
uint8_t GZ_ReadNotepad(uint8_t NotePageNum,uint8_t *Byte32)
{
	uint16_t temp;
  uint8_t  ensure,i;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x04);
	Sendcmd(0x19);
	Com_SendData(NotePageNum);
	temp = 0x01+0x04+0x19+NotePageNum;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		for(i=0;i<32;i++)
		{
			Byte32[i]=data[10+i];
		}
	}
	else
		ensure=0xff;
	return ensure;
}
//¸ßËÙËÑË÷GZ_HighSpeedSearch
//¹¦ÄÜ£ºÒÔ CharBuffer1»òCharBuffer2ÖĞµÄÌØÕ÷ÎÄ¼ş¸ßËÙËÑË÷Õû¸ö»ò²¿·ÖÖ¸ÎÆ¿â¡£
//		  ÈôËÑË÷µ½£¬Ôò·µ»ØÒ³Âë,¸ÃÖ¸Áî¶ÔÓÚµÄÈ·´æÔÚÓÚÖ¸ÎÆ¿âÖĞ £¬ÇÒµÇÂ¼Ê±ÖÊÁ¿
//		  ºÜºÃµÄÖ¸ÎÆ£¬»áºÜ¿ì¸ø³öËÑË÷½á¹û¡£
//²ÎÊı:  BufferID£¬ StartPage(ÆğÊ¼Ò³)£¬PageNum£¨Ò³Êı£©
//ËµÃ÷:  Ä£¿é·µ»ØÈ·ÈÏ×Ö+Ò³Âë£¨ÏàÅäÖ¸ÎÆÄ£°å£©
uint8_t GZ_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x08);
	Sendcmd(0x1b);
	Com_SendData(BufferID);
	Com_SendData(StartPage>>8);
	Com_SendData(StartPage);
	Com_SendData(PageNum>>8);
	Com_SendData(PageNum);
	temp = 0x01+0x08+0x1b+BufferID
	+(StartPage>>8)+(uint8_t)StartPage
	+(PageNum>>8)+(uint8_t)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
 	if(data)
	{
		ensure=data[9];
		p->pageID 	=(data[10]<<8) +data[11];
		p->mathscore=(data[12]<<8) +data[13];
	}
	else
		ensure=0xff;
	return ensure;
}
//¶ÁÓĞĞ§Ä£°å¸öÊı GZ_ValidTempleteNum
//¹¦ÄÜ£º¶ÁÓĞĞ§Ä£°å¸öÊı
//²ÎÊı: ÎŞ
//ËµÃ÷: Ä£¿é·µ»ØÈ·ÈÏ×Ö+ÓĞĞ§Ä£°å¸öÊıValidN
uint8_t GZ_ValidTempleteNum(uint16_t *ValidN)
{
	uint16_t temp;
  uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//ÃüÁî°ü±êÊ¶
	SendLength(0x03);
	Sendcmd(0x1d);
	temp = 0x01+0x03+0x1d;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		*ValidN = (data[10]<<8) +data[11];
	}		
	else
		ensure=0xff;
	
	return ensure;
}

//ä¸AS608æ¡æ‰‹ GZ_HandShake
//å‚æ•°: GZ_Addråœ°å€æŒ‡é’ˆ
//è¯´æ˜: æ¨¡å—è¿”æ–°åœ°å€ï¼ˆæ­£ç¡®åœ°å€ï¼‰	
uint8_t GZ_HandShake(uint32_t *GZ_Addr)
{
	SendHead();
	SendAddr();
	Com_SendData(0X01);
	Com_SendData(0X00);
	Com_SendData(0X00);	
	HAL_Delay(200);
	if(RX_len)
	{
		printf("%d\r\n",RX_len);
		RX_len=0;
		printf("%02x %02x %02x\r\n",aRxBuffer[0],aRxBuffer[1],aRxBuffer[6]);
		if(//åˆ¤æ–­æ˜¯ä¸æ˜¯æ¨¡å—è¿”å›çš„åº”ç­”åŒ…					
					aRxBuffer[0]==0XEF
				&&aRxBuffer[1]==0X01
				&&aRxBuffer[6]==0X07
			)
			{
				*GZ_Addr=(aRxBuffer[2]<<24) + (aRxBuffer[3]<<16)
								+(aRxBuffer[4]<<8) + (aRxBuffer[5]);
				return 0;
			}

	}
	return 1;		
}
//Ä£¿éÓ¦´ğ°üÈ·ÈÏÂëĞÅÏ¢½âÎö
//¹¦ÄÜ£º½âÎöÈ·ÈÏÂë´íÎóĞÅÏ¢·µ»ØĞÅÏ¢
//²ÎÊı: ensure
const char *EnsureMessage(uint8_t ensure) 
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;		
		case  0x01:
			p="Êı¾İ°ü½ÓÊÕ´íÎó";break;
		case  0x02:
			p="´«¸ĞÆ÷ÉÏÃ»ÓĞÊÖÖ¸";break;
		case  0x03:
			p="Â¼ÈëÖ¸ÎÆÍ¼ÏñÊ§°Ü";break;
		case  0x04:
			p="Ö¸ÎÆÍ¼ÏñÌ«¸É¡¢Ì«µ­¶øÉú²»³ÉÌØÕ÷";break;
		case  0x05:
			p="Ö¸ÎÆÍ¼ÏñÌ«Êª¡¢Ì«ºı¶øÉú²»³ÉÌØÕ÷";break;
		case  0x06:
			p="Ö¸ÎÆÍ¼ÏñÌ«ÂÒ¶øÉú²»³ÉÌØÕ÷";break;
		case  0x07:
			p="Ö¸ÎÆÍ¼ÏñÕı³££¬µ«ÌØÕ÷µãÌ«ÉÙ£¨»òÃæ»ıÌ«Ğ¡£©¶øÉú²»³ÉÌØÕ÷";break;
		case  0x08:
			p="Ö¸ÎÆ²»Æ¥Åä";break;
		case  0x09:
			p="Ã»ËÑË÷µ½Ö¸ÎÆ";break;
		case  0x0a:
			p="ÌØÕ÷ºÏ²¢Ê§°Ü";break;
		case  0x0b:
			p="·ÃÎÊÖ¸ÎÆ¿âÊ±µØÖ·ĞòºÅ³¬³öÖ¸ÎÆ¿â·¶Î§";
		case  0x10:
			p="É¾³ıÄ£°åÊ§°Ü";break;
		case  0x11:
			p="Çå¿ÕÖ¸ÎÆ¿âÊ§°Ü";break;	
		case  0x15:
			p="»º³åÇøÄÚÃ»ÓĞÓĞĞ§Ô­Ê¼Í¼¶øÉú²»³ÉÍ¼Ïñ";break;
		case  0x18:
			p="¶ÁĞ´ FLASH ³ö´í";break;
		case  0x19:
			p="Î´¶¨Òå´íÎó";break;
		case  0x1a:
			p="ÎŞĞ§¼Ä´æÆ÷ºÅ";break;
		case  0x1b:
			p="¼Ä´æÆ÷Éè¶¨ÄÚÈİ´íÎó";break;
		case  0x1c:
			p="¼ÇÊÂ±¾Ò³ÂëÖ¸¶¨´íÎó";break;
		case  0x1f:
			p="Ö¸ÎÆ¿âÂú";break;
		case  0x20:
			p="µØÖ·´íÎó";break;
		default :
			p="Ä£¿é·µ»ØÈ·ÈÏÂëÓĞÎó";break;
	}
 return p;	
}






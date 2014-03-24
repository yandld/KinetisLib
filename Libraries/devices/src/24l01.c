/**
  ******************************************************************************
  * @file    24l01.c
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   CH KinetisLib: http://github.com/yandld   http://upcmcu.taobao.com 
  ******************************************************************************
  */
  
#include "24l01.h"
#include "spi_abstraction.h"
#include "board.h"
#include "gpio.h"


#define CE_HIGH()     GPIO_WriteBit(BOARD_NRF2401_CE_PORT, BOARD_NRF2401_CE_PIN, 1)
#define CE_LOW()      GPIO_WriteBit(BOARD_NRF2401_CE_PORT, BOARD_NRF2401_CE_PIN, 0)

static uint8_t NRF2401_WriteBuffer(uint8_t reg, uint8_t *pData, uint8_t len)
{
    uint8_t status;
    status = SPI_ABS_ReadWriteByte(reg, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_KeepAsserted); // Select register to write to and read status byte
    len--;
    while(len--)
    {
        SPI_ABS_ReadWriteByte(*pData++, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_KeepAsserted);
    }
    SPI_ABS_ReadWriteByte(*pData++, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_ReturnInactive);
	return(status);          // return nRF24L01 status byte
}

static uint8_t NRF2401_ReadWriteReg(uint8_t reg,uint8_t value)
{
    uint8_t send_buf[2];
    uint8_t rec_buf[2];
    send_buf[0] = reg;
    send_buf[1] = value;
    SPI_ABS_xfer(send_buf, rec_buf, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_KeepAsserted, sizeof(send_buf));
    return rec_buf[0];
}

const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���͵�ַ
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���յ�ַ
unsigned char Bank0_Reg[][2]=
{
    {0,0x0F},//congig
    {1,0x03},//en_aa
    {2,0x03},//en_rxaddr
    {3,0x03},//setup_aw
    {4,0x15},//setup_retr
    {5,40},//rf_ch
    {6,0X07},//rf_setup	//REG6,120517,0x0F or 0x2F:2Mbps; 0x07:1Mbps ; 0x27:250Kbps
    {7,0x07},//status
    {8,0x00},//observe_tx
    {9,0x00},//cd
    {12,0x11},//rx_addr_p2
    {13,0x12},//rx_addr_p3
    {14,0x13},//rx_addr_p4
    {15,0x14},//rx_addr_p5
    {17,0x20},//RX_PW_P0
    {18,0x20},//RX_PW_P1
    {19,0x20},//RX_PW_P2
    {20,0x20},//RX_PW_P3
    {21,0x20},//RX_PW_P4
    {22,0x20},//RX_PW_P5
    {23,0x00},//FIFO_STATUS,
    {28,0x3F},//DYNPD
    {29,0x04}, //FEATURE
};

static uint8_t NRF2401_ConfigStatus(void)
{
    uint8_t i;
	for(i = 20; i > 0; i--)
    {
        NRF2401_ReadWriteReg(WRITE_REG+Bank0_Reg[i][0], Bank0_Reg[i][1]);    //ˢ�½�������
    }
	
	NRF2401_WriteBuffer(WRITE_REG+RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);//дTX�ڵ��ַ
	NRF2401_WriteBuffer(WRITE_REG+TX_ADDR, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);//дRX�ڵ��ַ
	i = NRF2401_ReadWriteReg(29,0xFF); //��ȡ�Ĵ�����״̬ 
	if(i == 0) // i!=0 showed that chip has been actived.so do not active again.
    {
        NRF2401_ReadWriteReg(0X50,0x73);// Active
    }
	for(i = 22; i >= 21; i--)
    {
        NRF2401_ReadWriteReg((WRITE_REG|Bank0_Reg[i][0]), Bank0_Reg[i][1]);
    }
	return 0;
}


static uint8_t NRF2401_ReadBuffer(uint8_t reg, uint8_t *pData, uint32_t len)
{
    uint8_t status;
    status = SPI_ABS_ReadWriteByte(reg, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_KeepAsserted); // Select register to write to and read status byte
    len--;
    while(len--)
    {
        *pData++ = SPI_ABS_ReadWriteByte(0x00, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_KeepAsserted);
    }
    *pData++ = SPI_ABS_ReadWriteByte(0x00, BOARD_SPI_NRF2401_CSn, kSPI_ABS_CS_ReturnInactive);
	return(status);                    // return nRF24L01 status byte
}

static uint8_t NRF2401_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i = sizeof(buf);
	NRF2401_WriteBuffer(WRITE_REG+TX_ADDR,buf, i);//д��5���ֽڵĵ�ַ.	
	NRF2401_ReadBuffer(TX_ADDR,buf, i); //����д��ĵ�ַ 
	for(i = 0; i < 5; i++)
	{
        if(buf[i] != 0xA5) return 1;
	}
	return 0;
}	

uint8_t NRF2401_Init(void)
{
    SPI_ABS_Init(kSPI_ABS_CPOL0_CPHA0, 1000*1000);
	GPIO_QuickInit(BOARD_NRF2401_CE_PORT, BOARD_NRF2401_CE_PIN , kGPIO_Mode_OPP);
    GPIO_QuickInit(BOARD_NRF2401_IRQ_PORT, BOARD_NRF2401_IRQ_PIN , kGPIO_Mode_IPU);
    PORT_PinMuxConfig(HW_GPIOC, BOARD_SPI_CS_PIN, kPinAlt2);
    CE_LOW();
    if(NRF2401_Check())
    {
        return 1;
    }
    NRF2401_ConfigStatus();
    return 0;
}



//�ú�����ʼ��NRF24L01��TXģʽ
//����TX��ַ,дTX���ݿ���,����RX�Զ�Ӧ��ĵ�ַ,���TX��������,ѡ��RFƵ��,�����ʺ�LNA HCURR
//PWR_UP,CRCʹ��
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
//CEΪ�ߴ���10us,����������.	 
void NRF2401_SetTXMode(void)
{			
    uint8_t value;
	NRF2401_ReadWriteReg(FLUSH_TX,  0x00);    //ˢ�½������� 
    CE_LOW();// chip enable
    value=NRF2401_ReadWriteReg(NCONFIG, 0xFF); //��ȡ�Ĵ�����״̬ 
    value |= 0xFE;//set bit 1
    NRF2401_ReadWriteReg(WRITE_REG+NCONFIG, value);// Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
    CE_HIGH(); //CEΪ��,�������ģʽ 
}

//�ú�����ʼ��NRF24L01��RXģʽ
//����RX��ַ,дRX���ݿ���,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void NRF2401_SetRXMode(void)
{
	uint8_t value;
	NRF2401_ReadWriteReg(FLUSH_RX, 0x00);    //ˢ�½�������
	value=NRF2401_ReadWriteReg(NRF2401_STATUS, 0xFF); //��ȡ�Ĵ�����״̬ 
	NRF2401_ReadWriteReg(WRITE_REG+NRF2401_STATUS, value);  //���״̬��־
	CE_LOW();
	value = NRF2401_ReadWriteReg(NCONFIG,0xFF); //��ȡ�Ĵ�����״̬ 
	value |= 0x01;//set bit 1
	NRF2401_ReadWriteReg(WRITE_REG+NCONFIG, value);// Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
	CE_HIGH(); //CEΪ��,�������ģʽ 
}	

//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
uint8_t NRF2401_SendPacket(uint8_t *txbuf, uint8_t len)
{
    NRF2401_WriteBuffer(WR_TX_PLOAD,txbuf, len);//д���ݵ�TX BUF  32���ֽ�
    return 0;
}

//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:0��������ɣ��������������
uint8_t NRF2401_RecPacket(uint8_t *rxbuf)
{
	uint8_t sta,len;		    							   
	sta = NRF2401_ReadWriteReg(NRF2401_STATUS,0xFF);  //��ȡ״̬�Ĵ�����ֵ  
  
	if(sta & RX_OK)//���յ�����
	{
        len = NRF2401_ReadWriteReg(0x60,0xFF);	// read len
        NRF2401_ReadBuffer(RD_RX_PLOAD,rxbuf,len);// read receive payload from RX_FIFO buffer	
        NRF2401_ReadWriteReg(WRITE_REG+NRF2401_STATUS,sta); //���TX_DS��MAX_RT�жϱ�־		
        return len;			
	}	
	NRF2401_ReadWriteReg(WRITE_REG+NRF2401_STATUS,sta); //���TX_DS��MAX_RT�жϱ�־		
	return 0;//û�յ��κ�����
}	



 












/**
  ******************************************************************************
  * @file    i2c.h
  * @author  YANDLD
  * @version V2.4
  * @date    2013.5.23
  * @brief   ����K60�̼��� IIC ͷ�ļ�
  ******************************************************************************
  */
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "sys.h"

#define HW_I2C0   (0x00U)
#define HW_I2C1   (0x01U)


//I2C ��ʼ���ṹ 
typedef struct
{
    uint8_t instance;
    uint32_t baudrate;
    
    //uint32_t I2C_ClockSpeed;        
}I2C_InitTypeDef;
/*??48  ??76    ??96   ??376 
*/



//!< I2C QuickInit macro
#define I2C1_SCL_PE01_SDA_PE00       (0x000081a1U)
#define I2C0_SCL_PB00_SDA_PB01       (0x00008088U)
#define I2C0_SCL_PB02_SDA_PB03       (0x00008488U)
#define I2C1_SCL_PC10_SDA_PC11       (0x00009491U)


//���������														
#define IS_I2C_ALL_PERIPH(PERIPH) ((PERIPH) == I2C0 || (PERIPH) == I2C1)

//I2C �ٶȶ���
#define I2C_CLOCK_SPEED_50KHZ                   ( 50*1000)
#define I2C_CLOCK_SPEED_100KHZ                  (100*1000)
#define I2C_CLOCK_SPEED_150KHZ                  (150*1000)
#define I2C_CLOCK_SPEED_200KHZ                  (200*1000)
#define I2C_CLOCK_SPEED_250KHZ                  (250*1000)
#define I2C_CLOCK_SPEED_300KHZ                  (300*1000)
//���������
#define IS_I2C_CLOCK_SPEED(SPEED)  (((SPEED) == I2C_CLOCK_SPEED_50KHZ)  || \
                                    ((SPEED) == I2C_CLOCK_SPEED_100KHZ) || \
                                    ((SPEED) == I2C_CLOCK_SPEED_150KHZ) || \
                                    ((SPEED) == I2C_CLOCK_SPEED_200KHZ) || \
                                    ((SPEED) == I2C_CLOCK_SPEED_250KHZ) || \
                                    ((SPEED) == I2C_CLOCK_SPEED_300KHZ))

//I2C ���趨��
typedef struct
{
    uint32_t I2C_Index:4;
    uint32_t I2C_GPIO_Index:4;
    uint32_t I2C_Alt_Index:4;
    uint32_t I2C_SCL_Pin_Index:6;
    uint32_t I2C_SDA_Pin_Index:6;
    uint32_t I2C_Reserved:8;
}I2C_MapTypeDef;


//�������
#define IS_I2C_DATA_CHL(CHL)     (((CHL) == I2C1_SCL_PE1_SDA_PE0) || \
                                  ((CHL) == I2C0_SCL_PB0_SDA_PB1) || \
                                  ((CHL) == I2C0_SCL_PB2_SDA_PB3) || \
                                  ((CHL) == I2C1_SCL_PC10_SDA_PC11))

//�ж�����
#define I2C_IT_TCF             (uint16_t)(0)
#define I2C_IT_IAAS            (uint16_t)(1)
#define I2C_IT_ARBL            (uint16_t)(2)
#define I2C_IT_SLTF            (uint16_t)(3)
#define I2C_IT_SHTF2           (uint16_t)(4)
//���������


//DMA����
#define I2C_DMAReq_TCF             ((uint16_t)0)
#define IS_I2C_DMAREQ(REQ)   ((REQ) == I2C_DMAReq_TCF)


//I2C������д����
#define I2C_MASTER_WRITE   (0)
#define I2C_MASTER_READ    (1)
//�������
#define IS_I2C_MASTER_DIRECTION(DIR)    (((DIR) == I2C_MASTER_WRITE) || \
                                        ((DIR) == I2C_MASTER_READ))
typedef enum
{
    kI2C_Read,
    kI2C_Write,
    kI2C_DirectionNameCount,
}I2C_Direction_Type; 

typedef enum
{
    kI2C_ITDMA_Disable,
    kI2C_IT_BTC,	//!< Byte Transfer Complete Interrupt
    kI2C_DMA_BTC,   //!< DMA Trigger On Byte Transfer Complete
}I2C_ITDMAConfig_Type;


//!< API functions
void I2C_Init(I2C_InitTypeDef* I2C_InitStruct);

//������ʵ�ֵĽӿں���
/*
void I2C_Init(I2C_InitTypeDef* I2C_InitStruct);
void I2C_GenerateSTART(I2C_Type *I2Cx);
void I2C_GenerateRESTART(I2C_Type *I2Cx);
void I2C_GenerateSTOP(I2C_Type *I2Cx);
void I2C_SendData(I2C_Type *I2Cx,uint8_t data8);
uint8_t I2C_ReadData(I2C_Type *I2Cx);
void I2C_Send7bitAddress(I2C_Type* I2Cx, uint8_t Address, uint8_t I2C_Direction);
uint8_t I2C_WaitAck(I2C_Type *I2Cx);
void I2C_SetMasterMode(I2C_Type* I2Cx,uint8_t I2C_Direction);
void I2C_GenerateAck(I2C_Type *I2Cx);
void I2C_GenerateNAck(I2C_Type *I2Cx);
void I2C_ITConfig(I2C_Type* I2Cx, uint16_t I2C_IT, FunctionalState NewState);
ITStatus I2C_GetITStatus(I2C_Type* I2Cx, uint16_t I2C_IT);
void I2C_DMACmd(I2C_Type* I2Cx, uint16_t I2C_DMAReq, FunctionalState NewState);
void I2C_ClearITPendingBit(I2C_Type* I2Cx, uint16_t I2C_IT);
uint8_t I2C_IsLineBusy(I2C_Type* I2Cx);
uint8_t I2C_WriteSingleRegister(I2C_Type* I2Cx, uint8_t DeviceAddress, uint8_t RegisterAddress, uint8_t Data);
uint8_t I2C_ReadSingleRegister(I2C_Type* I2Cx, uint8_t DeviceAddress, uint8_t RegisterAddress, uint8_t* pData);
uint8_t I2C_Write(I2C_Type *I2Cx ,uint8_t DeviceAddress, uint8_t *pBuffer, uint32_t len);
*/
#ifdef __cplusplus
}
#endif


#endif


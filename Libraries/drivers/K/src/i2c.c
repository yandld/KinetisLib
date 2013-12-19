/**
  ******************************************************************************
  * @file    i2c.c
  * @author  YANDLD
  * @version V2.4
  * @date    2013.5.23
  * @brief   超核K60固件库 IIC 驱动文件
  ******************************************************************************
  */
#include "i2c.h"
#include "gpio.h"
#include "clock.h"

//!< Leagacy Support for Kineis Z Version(Inital Version)
#if (!defined(I2C_BASES))

    #if (defined(MK60DZ10))
        #define I2C_BASES {I2C0, I2C1}
    #endif

#endif

//!< Gloabl Const Table Defination
static IRQn_Type   const I2C_IRQBase = I2C0_IRQn;
static I2C_Type * const I2C_InstanceTable[] = I2C_BASES;
static const uint32_t SIM_I2CClockGateTable[] =
{
    SIM_SCGC4_I2C0_MASK,
    SIM_SCGC4_I2C1_MASK,
};

typedef struct 
{
    uint8_t icr;            /*!< F register ICR value.*/
    uint16_t sclDivider;    /*!< SCL clock divider.*/
}_I2C_Divider_Type;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief I2C divider values.*/
/*!*/
/*! This table is taken from the I2C Divider and Hold values section of the*/
/*! reference manual. In the original table there are, in some cases, multiple*/
/*! entries with the same divider but different hold values. This table*/
/*! includes only one entry for every divider, selecting the lowest hold value.*/
const _I2C_Divider_Type I2C_DiverTable[] =
{
    /* ICR  Divider*/
    { 0x00, 20 },
    { 0x01, 22 },
    { 0x02, 24 },
    { 0x03, 26 },
    { 0x04, 28 },
    { 0x05, 30 },
    { 0x09, 32 },
    { 0x06, 34 },
    { 0x0a, 36 },
    { 0x07, 40 },
    { 0x0c, 44 },
    { 0x0d, 48 },
    { 0x0e, 56 },
    { 0x12, 64 },
    { 0x0f, 68 },
    { 0x13, 72 },
    { 0x14, 80 },
    { 0x15, 88 },
    { 0x19, 96 },
    { 0x16, 104 },
    { 0x1a, 112 },
    { 0x17, 128 },
    { 0x1c, 144 },
    { 0x1d, 160 },
    { 0x1e, 192 },
    { 0x22, 224 },
    { 0x1f, 240 },
    { 0x23, 256 },
    { 0x24, 288 },
    { 0x25, 320 },
    { 0x26, 384 },
    { 0x2a, 448 },
    { 0x27, 480 },
    { 0x2b, 512 },
    { 0x2c, 576 },
    { 0x2d, 640 },
    { 0x2e, 768 },
    { 0x32, 896 },
    { 0x2f, 960 },
    { 0x33, 1024 },
    { 0x34, 1152 },
    { 0x35, 1280 },
    { 0x36, 1536 },
    { 0x3a, 1792 },
    { 0x37, 1920 },
    { 0x3b, 2048 },
    { 0x3c, 2304 },
    { 0x3d, 2560 },
    { 0x3e, 3072 },
    { 0x3f, 3840 }
};


/* Documentation for this function is in fsl_i2c_hal.h.*/
void I2C_SetBaudrate(uint8_t instance, uint32_t sourceClockInHz, uint32_t baudrate)
{
    /* Check if the requested frequency is greater than the max supported baud.*/
    if (baudrate > (sourceClockInHz / (1U * 20U)))
    {
        return;
    }
    uint32_t mult;
    uint32_t hz = baudrate;
    uint32_t bestError = 0xffffffffu;
    uint32_t bestMult = 0u;
    uint32_t bestIcr = 0u;
    /* Search for the settings with the lowest error.*/
    /* mult is the MULT field of the I2C_F register, and ranges from 0-2. It selects the*/
    /* multiplier factor for the divider.*/
    for (mult = 0u; (mult <= 2u) && (bestError != 0); ++mult)
    {
        uint32_t multiplier = 1u << mult;
        /* Scan table to find best match.*/
        uint32_t i;
        for (i = 0u; i < ARRAY_SIZE(I2C_DiverTable); ++i)
        {
            uint32_t computedRate = sourceClockInHz / (multiplier * I2C_DiverTable[i].sclDivider);
            uint32_t absError = hz > computedRate ? hz - computedRate : computedRate - hz;
            if (absError < bestError)
            {
                bestMult = mult;
                bestIcr = I2C_DiverTable[i].icr;
                bestError = absError;
                /* If the error is 0, then we can stop searching because we won't find a*/
                /* better match.*/
                if (absError == 0)
                {
                    break;
                }
            }
        }
    }
    I2C_InstanceTable[instance]->F = (I2C_F_ICR(bestIcr)|I2C_F_MULT(bestMult));
}


static const QuickInit_Type I2C_QuickInitTable[] =
{
    { 1, 4, 6, 0, 2, 0}, //I2C1_SCL_PE01_SDA_PE00
    { 0, 1, 2, 0, 2, 0}, //I2C0_SCL_PB00_SDA_PB01
    { 0, 1, 2, 2, 2, 0}, //I2C0_SCL_PB02_SDA_PB03
    { 1, 2, 2,10, 2, 0}, //I2C1_SCL_PC10_SDA_PC11
};

void CalConst(const QuickInit_Type * table, uint32_t size)
{
	uint8_t i =0;
	uint32_t value = 0;
	for(i = 0; i < size; i++)
	{
		value = table[i].ip_instance<<0;
		value|= table[i].io_instance<<3;
		value|= table[i].mux<<6;
		value|= table[i].io_base<<9;
		value|= table[i].io_offset<<14;
		value|= table[i].channel<<19;
		UART_printf("(0x%08xU)\r\n",value);
	}
}

void I2C_QuickInit(uint32_t I2CxMAP, uint32_t baudrate)
{
    uint8_t i;
    I2C_InitTypeDef I2C_InitStruct1;
    QuickInit_Type *pI2CxMap = (QuickInit_Type*)&(I2CxMAP);
    I2C_InitStruct1.baudrate = baudrate;
    I2C_InitStruct1.instance = pI2CxMap->ip_instance;
    I2C_Init(&I2C_InitStruct1);
    // init pinmux and  open drain and pull up
    for(i = 0; i < pI2CxMap->io_offset; i++)
    {
        PORT_PinMuxConfig(pI2CxMap->io_instance, pI2CxMap->io_base + i, pI2CxMap->mux);
        PORT_PinConfig(pI2CxMap->io_instance, pI2CxMap->io_base + i, kPullUp, ENABLE); 
    }

    //UART_printf("pI2CxMap->io_base:%d\r\n", pI2CxMap->io_base);
}

void I2C_Init(I2C_InitTypeDef* I2C_InitStruct)
{
    uint32_t freq;
    SIM->SCGC4 |= SIM_I2CClockGateTable[I2C_InitStruct->instance];
    // disable first
    I2C_InstanceTable[I2C_InitStruct->instance]->C1 &= ~I2C_C1_IICEN_MASK;
    // set baudrate
    CLOCK_GetClockFrequency(kBusClock, &freq);
    I2C_SetBaudrate(I2C_InitStruct->instance, freq, I2C_InitStruct->baudrate);
    // enable i2c
    I2C_InstanceTable[I2C_InitStruct->instance]->C1 |= I2C_C1_IICEN_MASK;
}



/***********************************************************************************************
 功能：I2C 发送开始信号
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：0
 详解： I2C 总线当SCL为高电平 时 SDA下降沿跳变 识别为开始信号
************************************************************************************************/
void I2C_GenerateSTART(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->C1 |= I2C_C1_TX_MASK;
	I2Cx->C1 |= I2C_C1_MST_MASK;
}
/***********************************************************************************************
 功能：I2C 发送重新开始信号
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：0
 详解： I2C 总线当SCL为高电平 时 SDA下降沿跳变 识别为开始信号
************************************************************************************************/
void I2C_GenerateRESTART(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->C1 |= I2C_C1_RSTA_MASK;
}
/***********************************************************************************************
 功能：I2C 发送停止信号
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：0
 详解： I2C 总线当SCL为高电平 时 SDA上升沿跳变 识别为结束信号
************************************************************************************************/
void I2C_GenerateSTOP(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->C1 &= ~I2C_C1_MST_MASK;
	I2Cx->C1 &= ~I2C_C1_TX_MASK;
}
/***********************************************************************************************
 功能：I2C 发送8bit数据
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
			 data8: 8bit数据
 返回：0
 详解：0
************************************************************************************************/
void I2C_SendData(I2C_Type *I2Cx,uint8_t data8)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->D = data8;
}
/***********************************************************************************************
 功能：I2C 读取8bit数据
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：读取到的8bit 数据
 详解：0
************************************************************************************************/
uint8_t I2C_ReadData(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	return (I2Cx->D);
}
/***********************************************************************************************
 功能：I2C 发送7位地址码
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
			 Address: 7bit地址码
       I2C_Direction：
       @arg I2C_MASTER_WRITE: 主机写
       @arg I2C_MASTER_READ:  主机读
 返回：0
 详解：封装了SendData
************************************************************************************************/
void I2C_Send7bitAddress(I2C_Type* I2Cx, uint8_t Address, uint8_t I2C_Direction)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	//assert_param(IS_I2C_MASTER_DIRECTION(I2C_Direction));
	
	(I2C_Direction == I2C_MASTER_WRITE)?(Address &= 0xFE):(Address |= 0x01);
	I2Cx->D = Address;
}
/***********************************************************************************************
 功能：I2C 等待应答信号完成
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块

 返回：0   :成功收到应答 
       1   :未收到应答
 详解：主机每传送一个字节都需要调用I2C_WaitAck一次 WaitAck 会等待这一个字节传输结束 包括最后ACK位的结束后返回
************************************************************************************************/
uint8_t I2C_WaitAck(I2C_Type *I2Cx)
{

	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
    //wait for transfer complete
    while ((I2Cx->S & I2C_S_TCF_MASK) == 0);
    //both TCF and IICIF indicate one byte trasnfer complete
    while ((I2Cx->S & I2C_S_IICIF_MASK) == 0);
	  //IICIF is a W1C Reg, so clear it!
	  I2Cx->S |= I2C_S_IICIF_MASK;
    //see if we receive the ACK signal
    if (I2Cx->S & I2C_S_RXAK_MASK)
		{
        return 1;
		}
		else
		{
        return 0;
		}
}

/***********************************************************************************************
 功能：I2C 设置主机读写模式
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
			 I2C_Direction : 主机读写方向
			 @arg  I2C_MASTER_WRITE : 主机写
       @arg  I2C_MASTER_READ  : 主机读
 返回：0
 详解：0 
************************************************************************************************/
void I2C_SetMasterMode(I2C_Type* I2Cx,uint8_t I2C_Direction)
{
	//参数检查
	//assert_param(IS_I2C_MASTER_DIRECTION(I2C_Direction));
	
	(I2C_Direction == I2C_MASTER_WRITE)?(I2Cx->C1 |= I2C_C1_TX_MASK):(I2Cx->C1 &= ~I2C_C1_TX_MASK);
}
/***********************************************************************************************
 功能：I2C 设置为读取一个字节后返回NACK
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：0
 详解：0 
************************************************************************************************/
void I2C_GenerateNAck(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->C1 |= I2C_C1_TXAK_MASK;
}
/***********************************************************************************************
 功能：I2C 设置为读取一个字节后返回ACK
 形参：I2Cx: I2C模块号
       @arg I2C0: I2C0 模块
       @arg I2C1: I2C1 模块
 返回：0
 详解：0 
************************************************************************************************/
void I2C_GenerateAck(I2C_Type *I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	I2Cx->C1 &= ~I2C_C1_TXAK_MASK;
}

/***********************************************************************************************
 功能：I2C 中断配置
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
			 I2C_IT : 中断源
       @arg I2C_IT_TCF : I2C中断传输完成
       NewState : 使能或者禁止
       @arg ENABLE : 使能
       @arg DISABLE: 禁止 
 返回：0
 详解：0
************************************************************************************************/
void I2C_ITConfig(I2C_Type* I2Cx, uint16_t I2C_IT, FunctionalState NewState)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	//assert_param(IS_I2C_IT(I2C_IT));
	////assert_param(IS_FUNCTIONAL_STATE(NewState));
	
	switch(I2C_IT)
	{
		case I2C_IT_TCF:
			(ENABLE == NewState)?(I2Cx->C1 |= I2C_C1_IICIE_MASK):(I2Cx->C1 &= ~I2C_C1_IICIE_MASK);
			break;
		case I2C_IT_IAAS:
			(ENABLE == NewState)?(I2Cx->C1 |= I2C_C1_IICIE_MASK):(I2Cx->C1 &= ~I2C_C1_IICIE_MASK);
		  (ENABLE == NewState)?(I2Cx->C1 |= I2C_C1_WUEN_MASK):(I2Cx->C1 &= ~I2C_C1_WUEN_MASK);
			break;
		case I2C_IT_SLTF:
			(ENABLE == NewState)?(I2Cx->C1 |= I2C_C1_IICIE_MASK):(I2Cx->C1 &= ~I2C_C1_IICIE_MASK);
			break;
		case I2C_IT_SHTF2:
			(ENABLE == NewState)?(I2Cx->C1 |= I2C_C1_IICIE_MASK):(I2Cx->C1 &= ~I2C_C1_IICIE_MASK);
		  (ENABLE == NewState)?(I2Cx->SMB |= I2C_SMB_SHTF2IE_MASK):(I2Cx->SMB &= ~I2C_SMB_SHTF2IE_MASK);
			break;
		default:break;
	}
}
/***********************************************************************************************
 功能：I2C 获取中断状态
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
			 I2C_IT : 中断源
       @arg I2C_IT_TCF : I2C中断传输完成
 返回：0
 详解：0
************************************************************************************************/
ITStatus I2C_GetITStatus(I2C_Type* I2Cx, uint16_t I2C_IT)
{
	ITStatus retval = RESET;
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	//assert_param(IS_I2C_IT(I2C_IT));
	switch(I2C_IT)
	{
		case I2C_IT_TCF:
			(I2Cx->S & I2C_S_TCF_MASK)?(retval = SET):(retval = RESET);
			break;
		case I2C_IT_IAAS:
			(I2Cx->S & I2C_S_IAAS_MASK)?(retval = SET):(retval = RESET);
			break;
		case I2C_IT_SLTF:
			(I2Cx->SMB & I2C_SMB_SLTF_MASK)?(retval = SET):(retval = RESET);
			break;
		case I2C_IT_ARBL:
			(I2Cx->S & I2C_S_ARBL_MASK)?(retval = SET):(retval = RESET);
			break;  
		case I2C_IT_SHTF2:
			(I2Cx->SMB & I2C_SMB_SHTF2_MASK)?(retval = SET):(retval = RESET);
			break;
		default:break;
	}
	return retval;
}

/***********************************************************************************************
 功能：I2C DMA使能
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
			 I2C_DMAReq : 中断源
       @arg I2C_DMAReq_TCF : I2C传输完成
       NewState : 使能或者禁止
       @arg ENABLE : 使能
       @arg DISABLE: 禁止 
 返回：0
 详解：0
************************************************************************************************/
void I2C_DMACmd(I2C_Type* I2Cx, uint16_t I2C_DMAReq, FunctionalState NewState)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	//assert_param(IS_I2C_DMAREQ(I2C_DMAReq));
	//assert_param(IS_FUNCTIONAL_STATE(NewState));
	
	switch(I2C_DMAReq)
	{
		case I2C_DMAReq_TCF:
			(NewState == ENABLE)?(I2Cx->C1 |= I2C_C1_DMAEN_MASK):(I2Cx->C1 &= ~I2C_C1_DMAEN_MASK);
			break;
		default:break;
	}
}

/***********************************************************************************************
 功能：I2C 清除中断标志
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
			 I2C_IT : 中断源
       @arg I2C_IT_TCF : I2C中断传输完成
 返回：0
 详解：0
************************************************************************************************/
void I2C_ClearITPendingBit(I2C_Type* I2Cx, uint16_t I2C_IT)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	//assert_param(IS_I2C_IT(I2C_IT));
	
	//清中断标志位
	I2Cx->C1 |= I2C_C1_IICEN_MASK;
	switch(I2C_IT)
	{
		case I2C_IT_TCF:
			break;
		case I2C_IT_IAAS:
			break;
		case I2C_IT_SLTF:
			I2Cx->SMB |= I2C_SMB_SLTF_MASK;
			break; 
		case I2C_IT_ARBL:
	  	I2Cx->S |= I2C_S_ARBL_MASK;
			break;
		case I2C_IT_SHTF2:
			I2Cx->SMB |= I2C_SMB_SHTF2_MASK;
			break;
	}
}
/***********************************************************************************************
 功能：判断I2C 线上是否空闲 高电平
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
 返回：TRUE: 空闲 FALSE :忙
 详解：0
************************************************************************************************/
uint8_t I2C_IsLineBusy(I2C_Type* I2Cx)
{
	//参数检查
	//assert_param(IS_I2C_ALL_PERIPH(I2Cx));
	
	if(I2Cx->S & I2C_S_BUSY_MASK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

uint8_t I2C_Write(I2C_Type *I2Cx ,uint8_t DeviceAddress, uint8_t *pBuffer, uint32_t len)
{
    //Generate START signal
    I2C_GenerateSTART(I2Cx);
    //Send 7bit Data with WRITE operation
    I2C_Send7bitAddress(I2Cx, DeviceAddress, I2C_MASTER_WRITE);
    if(I2C_WaitAck(I2Cx))
    {
			  I2C_GenerateSTOP(I2Cx);
			  while((I2Cx->S & I2C_S_BUSY_MASK) == 1) {};
        return 1;
    }
		//Send All Data
		while(len--)
		{
        I2C_SendData(I2Cx, *(pBuffer++));
        if(I2C_WaitAck(I2Cx))
        {
            I2C_GenerateSTOP(I2Cx);
            while((I2Cx->S & I2C_S_BUSY_MASK) == 1) {};
            return 2;
        }
		}
		//Generate stop and wait for line idle
		I2C_GenerateSTOP(I2Cx);
    while((I2Cx->S & I2C_S_BUSY_MASK) == 1) {};
		return 0;
}


/***********************************************************************************************
 功能：通用 I2C 读写一个从机的一个寄存器 适用于大多数期间 MMA84系列传感器等等
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
       DeviceAddress: 设备地址
       RegisterAddress:寄存器在设备中的地址
       Data: 需要写入的数据
 返回：0:成功 else: 错误代码
 详解：实质顺序为 START->ADDRESS->RegADR->Data->STOP->Wait until all stop
************************************************************************************************/
uint8_t I2C_WriteSingleRegister(I2C_Type* I2Cx, uint8_t DeviceAddress, uint8_t RegisterAddress, uint8_t Data)
{
    uint8_t ret;
    uint8_t buf[2];
    buf[0] = RegisterAddress;
    buf[1] = Data;
    ret = I2C_Write(I2Cx, DeviceAddress, buf, sizeof(buf));
    return ret;
}



/***********************************************************************************************
 功能：通用 I2C 读写一个从机的一个寄存器 适用于大多数期间 MMA84系列传感器等等
 形参：I2Cx: I2C模块号
       @arg I2C0 : I2C0模块
       @arg I2C1 : I2C1模块
       DeviceAddress: 设备地址
       RegisterAddress:寄存器在设备中的地址
       Data: 需要写入的数据
 返回：0:成功 else: 错误代码
 详解：实质顺序为 START->ADDRESS->RESTART->ADDRESS_With_READ->ReadData->SEND_NACK->STOP->Wait until all stop
************************************************************************************************/
uint8_t I2C_ReadSingleRegister(I2C_Type* I2Cx, uint8_t DeviceAddress, uint8_t RegisterAddress, uint8_t* pData)
{
    uint8_t data;
    //Generate START signal
    I2C_GenerateSTART(I2Cx);
    //Send 7bit Data with WRITE operation
    I2C_Send7bitAddress(I2Cx, DeviceAddress, I2C_MASTER_WRITE);
    if(I2C_WaitAck(I2Cx))
    {
			  I2C_GenerateSTOP(I2Cx);
			  while((I2Cx->S & I2C_S_BUSY_MASK) == 1);
        return 1;
    }
    //Send Reg Address
		I2C_SendData(I2Cx, RegisterAddress);
    if(I2C_WaitAck(I2Cx))
    {
			  I2C_GenerateSTOP(I2Cx);
			  while((I2Cx->S & I2C_S_BUSY_MASK) == 1);
        return 2;
    }
		//Generate RESTART Signal
    I2C_GenerateRESTART(I2Cx);
		//Resend 7bit Address, This time we use READ Command
    I2C_Send7bitAddress(I2Cx, DeviceAddress, I2C_MASTER_READ);
    if(I2C_WaitAck(I2Cx))
    {
			  I2C_GenerateSTOP(I2Cx);
        while((I2Cx->S & I2C_S_BUSY_MASK) == 1);
        return 3;
    }
    //Set Master in slave mode
    I2C_SetMasterMode(I2Cx,I2C_MASTER_READ);
		//Dummy Read in order to generate SCL clock
    data = I2Cx->D;
		I2C_GenerateNAck(I2Cx);
		//This time, We just wait for masters receive byte transfer complete
    I2C_WaitAck(I2Cx);
		//Generate stop and wait for line idle
		I2C_GenerateSTOP(I2Cx);
    while((I2Cx->S & I2C_S_BUSY_MASK) == 1);
		//actual read
		data = I2Cx->D;
		*pData = data;
		return 0;
}




#if 0
/***********************************************************************************************
 功能：I2C 初始化
 形参：I2C_InitStruct: I2C初始化结构
 返回：0
 详解：0
************************************************************************************************/
void I2C_Init(I2C_InitTypeDef* I2C_InitStruct)
{
	I2C_Type *I2Cx = NULL;
	PORT_Type *I2C_PORT = NULL;
	uint32_t prescaler = 0;
	I2C_MapTypeDef *pI2C_Map = (I2C_MapTypeDef*)&(I2C_InitStruct->I2CxMAP);
	//参数检查
	//assert_param(IS_I2C_DATA_CHL(I2C_InitStruct->I2CxMAP));
	//assert_param(IS_I2C_CLOCK_SPEED(I2C_InitStruct->I2C_ClockSpeed));
	//使能I2C时钟
	switch(pI2C_Map->I2C_Index)
	{
		case 0:
			SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK; 
			I2Cx = I2C0;
			break;
		case 1:
			SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK;
			I2Cx = I2C1;		
			break;
		default:break;
	}
	//使能对应的PORT
	switch(pI2C_Map->I2C_GPIO_Index)
	{
		case 0:
			SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
			I2C_PORT = PORTA;
			break;
		case 1:
			SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
			I2C_PORT = PORTB;
			break;
		case 2:
			SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
			I2C_PORT = PORTC;
			break;
		case 3:
			SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
			I2C_PORT = PORTD;
			break;
		case 4:
			SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
			I2C_PORT = PORTE;
			break;
		default:break;
	}
	//开启对应引脚为I2C功能
	I2C_PORT->PCR[pI2C_Map->I2C_SCL_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	I2C_PORT->PCR[pI2C_Map->I2C_SDA_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	I2C_PORT->PCR[pI2C_Map->I2C_SCL_Pin_Index] |= PORT_PCR_MUX(pI2C_Map->I2C_Alt_Index)|PORT_PCR_ODE_MASK;
	I2C_PORT->PCR[pI2C_Map->I2C_SDA_Pin_Index] |= PORT_PCR_MUX(pI2C_Map->I2C_Alt_Index)|PORT_PCR_ODE_MASK;
	//将引脚设置为漏极输出
	//设置I2C分频数
//	prescaler = (((CPUInfo.BusClock /(I2C_InitStruct->I2C_ClockSpeed))-160))/32 +  0x20;
	I2Cx->F	= prescaler;
	//使能I2C模块
	I2Cx->C1 = I2C_C1_IICEN_MASK ;
}




#endif


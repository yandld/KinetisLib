#include "gpio.h"
#include "common.h"
#include "uart.h"
#include "dma.h"
#include "sram.h"
#include "ftm.h"
#include "dma.h"
#include "pit.h"

static const uint32_t DMA_PORT_TriggerSourceTable[] = 
{
    PORTA_DMAREQ,
    PORTB_DMAREQ,
    PORTC_DMAREQ,
    PORTD_DMAREQ,
    PORTE_DMAREQ,
};

/**
 * @brief  DMA 用作脉冲计数初始化     
 * @param  dmaChl :DMA通道号
 * @param  instance :端口号 比如HW_GPIOA
 * @param  pinIndex :引脚号
 * @retval None
 */
static void DMA_PulseCountInit(uint32_t dmaChl, uint32_t instance, uint32_t pinIndex)
{
    /* 开启2路引脚 配置为DMA触发 */
    GPIO_QuickInit(instance, pinIndex, kGPIO_Mode_IPU);
    /* 配置为DMA上升沿触发 */
    GPIO_ITDMAConfig(instance, pinIndex, kGPIO_DMA_RisingEdge);
    /* 配置DMA */
    uint8_t dummy1, dummy2;
    DMA_InitTypeDef DMA_InitStruct1;  
    DMA_InitStruct1.chl = dmaChl;  
    DMA_InitStruct1.chlTriggerSource = DMA_PORT_TriggerSourceTable[instance];
    DMA_InitStruct1.triggerSourceMode = kDMA_TriggerSource_Normal; 
    DMA_InitStruct1.minorByteTransferCount = 1;
    DMA_InitStruct1.majorTransferCount = DMA_CITER_ELINKNO_CITER_MASK;
    
    DMA_InitStruct1.sourceAddress = (uint32_t)&dummy1;
    DMA_InitStruct1.sourceAddressMajorAdj = 0; 
    DMA_InitStruct1.sourceAddressMinorAdj = 0;
    DMA_InitStruct1.sourceDataWidth = kDMA_DataWidthBit_8;
    
    DMA_InitStruct1.destAddress = (uint32_t)&dummy2; 
    DMA_InitStruct1.destAddressMajorAdj = 0;
    DMA_InitStruct1.destAddressMinorAdj = 0; 
    DMA_InitStruct1.destDataWidth = kDMA_DataWidthBit_8;
    DMA_Init(&DMA_InitStruct1);
    /* 启动传输 */
    DMA_StartTransfer(dmaChl);
}


static void PIT_ISR(void)
{
    /* 由于DMA 是倒计数的 所需需要用最大值减一下 */
    uint32_t ch_value[2];
    /* CH0 */
    ch_value[0] = DMA_CITER_ELINKNO_CITER_MASK - DMA_GetMajorLoopCount(HW_DMA_CH0);
    /* CH1 */
    ch_value[1] = DMA_CITER_ELINKNO_CITER_MASK - DMA_GetMajorLoopCount(HW_DMA_CH1);
    /* 清零计数 */
    DMA_CancelTransfer();
    DMA_SetMajorLoopCount(HW_DMA_CH0, DMA_CITER_ELINKNO_CITER_MASK);
    DMA_SetMajorLoopCount(HW_DMA_CH1, DMA_CITER_ELINKNO_CITER_MASK);
    /* 开始下一次传输 */
    DMA_StartTransfer(HW_DMA_CH0);
    DMA_StartTransfer(HW_DMA_CH1);
    printf("[CH%d]:%4dHz [CH%d]:%4dHz\r\n", 0, ch_value[0], 1, ch_value[1]); 
    
    
}

int main(void)
{
    DelayInit();
    GPIO_QuickInit(HW_GPIOE, 6, kGPIO_Mode_OPP);
    UART_QuickInit(UART0_RX_PD06_TX_PD07, 115200);
    
    printf("DMA pulse count test\r\n");
    printf("connect A05&C01, B00&B23 \r\n");
    
    /* 开启2路PWM通道 产生不同频率的PWM波 */
    FTM_PWM_QuickInit(FTM0_CH0_PC01, 10000);
    FTM_PWM_QuickInit(FTM1_CH0_PB00, 20000);

    
    /* 开启DMA捕捉引脚脉冲信号 使用DMA0通道 触发源选择A 端口 5引脚 (每个端口只能测量一路DMA 也就是说DMA脉冲最多只能测量5路(PTA,PTB,PTC,PTD,PTE))*/
    DMA_PulseCountInit(HW_DMA_CH0, HW_GPIOA, 5);
    DMA_PulseCountInit(HW_DMA_CH1, HW_GPIOB, 23);
    
    /* 开启一个PIT中断用于显示收到的计数 */
    PIT_QuickInit(HW_PIT_CH0, 1000 * 1000);
    PIT_CallbackInstall(HW_PIT_CH0, PIT_ISR);
    PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF);

   
    while(1)
    {
        GPIO_ToggleBit(HW_GPIOE, 6);
        DelayMs(500);
    }
}



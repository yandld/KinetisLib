/**
  ******************************************************************************
  * @file    dma.c
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   CH KinetisLib: http://github.com/yandld   http://upcmcu.taobao.com 
  ******************************************************************************
  */

#include "dma.h"

static DMA_CallBackType DMA_CallBackTable[16] = {NULL};

static const IRQn_Type DMA_IRQnTable[] = 
{
    DMA0_IRQn,
    DMA1_IRQn,
    DMA2_IRQn,
    DMA3_IRQn,
    DMA4_IRQn,
    DMA5_IRQn,
    DMA6_IRQn,
    DMA7_IRQn,
    DMA8_IRQn,
    DMA9_IRQn,
    DMA10_IRQn,
    DMA11_IRQn,
    DMA12_IRQn,
    DMA13_IRQn,
    DMA14_IRQn,
    DMA15_IRQn,
};


void DMA_Init(DMA_InitTypeDef *DMA_InitStruct)
{
	// enable DMA and DMAMUX clock
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;    
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
    // dma chl source config
    DMAMUX->CHCFG[DMA_InitStruct->chl] = DMAMUX_CHCFG_SOURCE(DMA_InitStruct->chlTriggerSource);
    // trigger mode
    switch(DMA_InitStruct->triggerSourceMode)
    {
        case kDMA_TriggerSource_Normal:
            DMAMUX->CHCFG[DMA_InitStruct->chl] &= ~DMAMUX_CHCFG_TRIG_MASK;
            break;
        case kDMA_TriggerSource_Periodic:
            DMAMUX->CHCFG[DMA_InitStruct->chl] |= DMAMUX_CHCFG_TRIG_MASK;
            break;
        default:
            break;
    }
    //
    DMA0->TCD[DMA_InitStruct->chl].ATTR  = 0;
    // minor loop cnt
    DMA0->TCD[DMA_InitStruct->chl].NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(DMA_InitStruct->minorByteTransferCount);
    // major loop cnt
	DMA0->TCD[DMA_InitStruct->chl].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(DMA_InitStruct->majorTransferCount);
	DMA0->TCD[DMA_InitStruct->chl].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(DMA_InitStruct->majorTransferCount);
    // source config
    DMA0->TCD[DMA_InitStruct->chl].SADDR = DMA_InitStruct->sourceAddress;
    DMA0->TCD[DMA_InitStruct->chl].SOFF = DMA_InitStruct->sourceAddressMinorAdj;
    DMA0->TCD[DMA_InitStruct->chl].ATTR |= DMA_ATTR_SSIZE(DMA_InitStruct->sourceDataWidth);
    DMA0->TCD[DMA_InitStruct->chl].SLAST = DMA_SLAST_SLAST(DMA_InitStruct->sourceAddressMajorAdj);
    // destation config
    DMA0->TCD[DMA_InitStruct->chl].DADDR = DMA_InitStruct->destAddress;
    DMA0->TCD[DMA_InitStruct->chl].DOFF = DMA_InitStruct->destAddressMinorAdj;
    DMA0->TCD[DMA_InitStruct->chl].ATTR |= DMA_ATTR_DSIZE(DMA_InitStruct->destDataWidth);
    DMA0->TCD[DMA_InitStruct->chl].DLAST_SGA = DMA_DLAST_SGA_DLASTSGA(DMA_InitStruct->destAddressMajorAdj);
    // auto close enable
    DMA0->TCD[DMA_InitStruct->chl].CSR |= DMA_CSR_DREQ_MASK; 
	// enable DMAMUX
	DMAMUX->CHCFG[DMA_InitStruct->chl] |= DMAMUX_CHCFG_ENBL_MASK;
}

//!< Start transfer
void DMA_StartTransfer(uint8_t chl)
{
    DMA0->ERQ |= (1<<chl);
}


void DMA_ITConfig(uint8_t chl, DMA_ITConfig_Type config)
{
    switch(config)
    {
        case kDMA_IT_Half_Disable:
            DMA0->TCD[chl].CSR &= ~DMA_CSR_INTHALF_MASK;
            break;
        case kDMA_IT_Major_Disable:
            DMA0->TCD[chl].CSR &= ~DMA_CSR_INTMAJOR_MASK;
            break;
        case kDMA_IT_Half:
            NVIC_EnableIRQ(DMA_IRQnTable[chl]);
            DMA0->TCD[chl].CSR |= DMA_CSR_INTHALF_MASK;
            break;
        case kDMA_IT_Major:
            NVIC_EnableIRQ(DMA_IRQnTable[chl]);
            DMA0->TCD[chl].CSR |= DMA_CSR_INTMAJOR_MASK;
            break; 
        default:
            break;
    }
}

void DMA_CallbackInstall(uint8_t chl, DMA_CallBackType AppCBFun)
{
    if(AppCBFun != NULL)
    {
        DMA_CallBackTable[chl] = AppCBFun;
    }
}

uint8_t DMA_IsTransferComplete(uint8_t chl)
{
    if(DMA0->TCD[chl].CSR & DMA_CSR_DONE_MASK)
    {
        // clear this bit
        DMA0->TCD[chl].CSR &= ~DMA_CSR_DONE_MASK;
        return 0;
    }
    else
    {
        return 1;
    }
}

void DMA0_IRQHandler(void)
{
    DMA0->INT |= (1<<0);
    if(DMA_CallBackTable[0]) DMA_CallBackTable[0]();
}
void DMA1_IRQHandler(void)
{
    DMA0->INT |= (1<<1);
    if(DMA_CallBackTable[1]) DMA_CallBackTable[1]();
}
void DMA2_IRQHandler(void)
{
    DMA0->INT |= (1<<2);
    if(DMA_CallBackTable[2]) DMA_CallBackTable[2]();
}
void DMA3_IRQHandler(void)
{
    DMA0->INT |= (1<<3);
    if(DMA_CallBackTable[3]) DMA_CallBackTable[3]();
}
void DMA4_IRQHandler(void)
{
    DMA0->INT |= (1<<4);
    if(DMA_CallBackTable[4]) DMA_CallBackTable[4]();
}
void DMA5_IRQHandler(void)
{
    DMA0->INT |= (1<<5);
    if(DMA_CallBackTable[5]) DMA_CallBackTable[5]();
}
void DMA6_IRQHandler(void)
{
    DMA0->INT |= (1<<6);
    if(DMA_CallBackTable[6]) DMA_CallBackTable[6]();
}
void DMA7_IRQHandler(void)
{
    DMA0->INT |= (1<<7);
    if(DMA_CallBackTable[7]) DMA_CallBackTable[7]();
}
void DMA8_IRQHandler(void)
{
    DMA0->INT |= (1<<8);
    if(DMA_CallBackTable[8]) DMA_CallBackTable[8]();
}
void DMA9_IRQHandler(void)
{
    DMA0->INT |= (1<<9);
    if(DMA_CallBackTable[9]) DMA_CallBackTable[9]();
}
void DMA10_IRQHandler(void)
{
    DMA0->INT |= (1<<10);
    if(DMA_CallBackTable[10]) DMA_CallBackTable[10]();
}
void DMA11_IRQHandler(void)
{
    DMA0->INT |= (1<<11);
    if(DMA_CallBackTable[11]) DMA_CallBackTable[11]();
}
void DMA12_IRQHandler(void)
{
    DMA0->INT |= (1<<12);
    if(DMA_CallBackTable[12]) DMA_CallBackTable[12]();
}
void DMA13_IRQHandler(void)
{
    DMA0->INT |= (1<<13);
    if(DMA_CallBackTable[13]) DMA_CallBackTable[13]();
}
void DMA14_IRQHandler(void)
{
    DMA0->INT |= (1<<14);
    if(DMA_CallBackTable[14]) DMA_CallBackTable[14]();
}
void DMA15_IRQHandler(void)
{
    DMA0->INT |= (1<<15);
    if(DMA_CallBackTable[15]) DMA_CallBackTable[15]();
}






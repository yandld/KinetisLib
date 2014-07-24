/**
  ******************************************************************************
  * @file    pdb.h
  * @author  YANDLD
  * @version V2.5
  * @date    2014.3.24
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com 
  ******************************************************************************
  */
#ifndef __CH_LIB_PDB_H__
#define __CH_LIB_PDB_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "common.h"
#include <stdint.h>

//!< PIT CallBack Type
typedef void (*PDB_CallBackType)(void);

typedef struct
{
    uint32_t                srcClock;           // ʱ��ԴƵ��
    uint32_t                inputTrigSource;    // �������봥��Դ
    bool                    isContinuesMode;    //�Ƿ�����������ģʽ
    uint32_t                clkDiv;             //��ʱʱ��
    uint32_t                mod;                //�������ֵ
}PDB_InitTypeDef;

/*!< �жϼ�DMA���� */
typedef enum
{
    kPDB_IT_CF,
    kPDB_DMA_CF,
}PDB_ITDMAConfig_Type;


//!< API functions
void PDB_Init(PDB_InitTypeDef * PDB_InitStruct);
void PDB_SoftwareTrigger(void);
void PDB_ITDMAConfig(PDB_ITDMAConfig_Type config, bool status);
void PDB_CallbackInstall(PDB_CallBackType AppCBFun);
uint32_t PDB_GetModValue(void);

#ifdef __cplusplus
}
#endif

#endif

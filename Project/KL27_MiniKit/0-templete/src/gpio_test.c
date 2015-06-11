#include "gpio.h"

#define LED0_PORT   HW_GPIOC
#define LED0_PIN    3
#define LED1_PORT   HW_GPIOA
#define LED1_PIN    1
#define LED2_PORT   HW_GPIOE
#define LED2_PIN    0
#define LED3_PORT   HW_GPIOA
#define LED3_PIN    2

int cmd_gpio(int argc, char * const argv[])
{
    int i;
    /* LED */
    GPIO_QuickInit(LED0_PORT, LED0_PIN, kGPIO_Mode_OPP);
    GPIO_QuickInit(LED1_PORT, LED1_PIN, kGPIO_Mode_OPP);
    GPIO_QuickInit(LED2_PORT, LED2_PIN, kGPIO_Mode_OPP);
    GPIO_QuickInit(LED3_PORT, LED3_PIN, kGPIO_Mode_OPP);
    /* KEY */
    GPIO_QuickInit(HW_GPIOB, 0, kGPIO_Mode_IPU);

    for(i=0;i<10;i++)
    {
        GPIO_ToggleBit(LED0_PORT, LED0_PIN);
        GPIO_ToggleBit(LED2_PORT, LED2_PIN);
        GPIO_ToggleBit(LED3_PORT, LED3_PIN);
        DelayMs(50);  
    }
    return 0;
}


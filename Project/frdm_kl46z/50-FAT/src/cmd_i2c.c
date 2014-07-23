#include "shell.h"
#include "gpio.h"
#include "i2c.h"

void I2C_ISR(void)
{
    printf("I2C ISR enter\r\n");
}

/* i2c bus scan */
static void I2C_Scan(uint32_t instance)
{
    uint8_t i;
    uint8_t ret;
    for(i = 1; i < 127; i++)
    {
        ret = I2C_BurstWrite(instance , i, 0, 0, NULL, 0);
        if(!ret)
        {
            printf("ADDR:0x%2X(7BIT) | 0x%2X(8BIT) found!\r\n", i, i<<1);
        }
    }
}

static int DoI2C(int argc, char * const argv[])
{
    uint32_t instance;
    printf("i2c bus test\r\n");
    /* init i2c */
    instance = I2C_QuickInit(I2C0_SCL_PB02_SDA_PB03, 100*1000);
    PORT_PinMuxConfig(HW_GPIOB, 2, kPinAlt0);
    PORT_PinMuxConfig(HW_GPIOB, 3, kPinAlt0);
    PORT_PinMuxConfig(HW_GPIOC, 6, kPinAlt7);
    PORT_PinMuxConfig(HW_GPIOC, 7, kPinAlt7);
    I2C_Scan(instance);
}

SHELL_EXPORT_CMD(DoI2C, i2c , i2c test)
    


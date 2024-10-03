#ifndef _SW_I2C_H_
#define _SW_I2C_H_

#include <stdint.h>

#define SW_I2C_WAIT_TIME    10 // 10us 100kHz



#define I2C_READ            0x01
#define READ_CMD            1
#define WRITE_CMD           0

typedef enum
{
    HAL_IO_OPT_SET_SDA_LOW = 0,     // ��������� SDA � 0
    HAL_IO_OPT_SET_SDA_HIGH,        // ��������� SDA � 1

    HAL_IO_OPT_SET_SCL_LOW,         // ��������� SCL � 0
    HAL_IO_OPT_SET_SCL_HIGH,        // ��������� SCL � 1

    HAL_IO_OPT_SET_SDA_INPUT,       // ���������������� SDA � INPUT
    HAL_IO_OPT_SET_SDA_OUTPUT,      // ���������������� SDA � OUTPUT

    HAL_IO_OPT_SET_SCL_INPUT,       // ���������������� SCL � INPUT
    HAL_IO_OPT_SET_SCL_OUTPUT,      // ���������������� SCL � OUTPUT

    HAL_IO_OPT_GET_SDA_LEVEL,       // ������� ������� �� SDA
    HAL_IO_OPT_GET_SCL_LEVEL        // ������� ������� �� SCL
} hal_io_opt_e;

typedef struct sw_i2c_s
{
    int (*hal_init)(void);
    int (*hal_io_ctl)(hal_io_opt_e opt);
    void (*hal_delay_us)(uint32_t us);
} sw_i2c_t;


/* functions */
void SW_I2C_initial(sw_i2c_t *d);
uint8_t SW_I2C_Read_8addr(sw_i2c_t *d, uint8_t IICID, uint8_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Read_16addr(sw_i2c_t *d, uint8_t IICID, uint16_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Write_8addr(sw_i2c_t *d, uint8_t IICID, uint8_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Write_16addr(sw_i2c_t *d, uint8_t IICID, uint16_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Check_SlaveAddr(sw_i2c_t *d, uint8_t IICID);


#endif  /* __I2C_SW_H */

/**
 ****************************************************************************************************
 * @file        atk_spl06.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-10-17
 * @brief       SPL06��ѹ�� ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20241017
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "atk_spl06.h"
//#include "./BSP/IIC/myiic.h"
//#include "./SYSTEM/usart/usart.h"
#include "math.h"
#include "stm32f1xx_it.h"

spl06_calibcoeff_t spl06calib;

/* Kp��Kt�ĳ߶����� */
const uint32_t scale_factor[8] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

static int32_t kp = 0;
static int32_t kt = 0;

/**
 * @brief   дһ���ֽڵ�SPL06�ļĴ���
 * @param   reg: �Ĵ�����ַ
 * @param   data: �Ĵ�������
 * @retval  д����
 * @arg     0: �ɹ�
 * @arg     1: ʧ��
 */
uint8_t atk_spl06_write_byte(uint8_t reg, uint8_t data)
{
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(data);        /* ����һ�ֽ� */
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_stop();                 /* ����һ��ֹͣ���� */
    
    return 0;
}

/**
 * @brief   ��SPL06ָ���Ĵ�����ַ����һ������
 * @param   reg:        �Ĵ�����ַ
 * @retval  ����������
 */
uint8_t atk_spl06_read_byte(uint8_t reg)
{
    uint8_t temp = 0;
    
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x01);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    temp = iic_read_byte(0);
    iic_stop();
    
    return temp;
}

/**
 * @brief   ��SPL06��ȡN�ֽ�����
 * @param   reg:        �Ĵ�����ַ
 * @param   date:       ���ݴ洢buf
 * @param   len:        ���ݳ���
 * @retval  �������
 * @retval  0, �����ɹ�
 *          ����, ����ʧ��
 */
int atk_spl06_read_nbytes(uint8_t reg, uint8_t *date, uint8_t len)
{
    uint8_t i;
    
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x01);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    for (i = 0; i < len; i++)
    {
        date[i] = iic_read_byte((i == (len - 1)) ? 0 : 1);
    }
    iic_stop();
    
    return 0;
}

/**
 * @brief   SPL06д��N�ֽ�����
 * @param   reg:        �Ĵ�����ַ
 * @param   data:       д������
 * @param   len:        ���ݳ���
 * @retval  д����
 * @arg     0:          �ɹ�
 * @arg     1:          ʧ��
 */
uint8_t atk_spl06_write_nbytes(uint8_t reg, uint8_t* data, uint8_t len)
{
    uint8_t i;
    
    iic_start();
    iic_send_byte((SPL06_I2C_ADDR << 1) | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    for (i = 0; i < len; i++)
    {
        iic_send_byte(data[i]);
        if (iic_wait_ack() != 0)
        {
            iic_stop();
            return 1;
        }
    }
    iic_stop();
    
    return 0;
}


/**
 * @brief   ��ȡУ׼���ݣ����ڼ�����ѹֵ���¶�ֵ��
 * @param   ��
 * @retval  ��
 */
void atk_spl06_get_calib_param(void)
{
    uint8_t buffer[SPL06_CALIB_COEFFICIENT_LENGTH] = {0};

    atk_spl06_read_nbytes(SPL06_COEFFICIENT_CALIB_REG, buffer, SPL06_CALIB_COEFFICIENT_LENGTH);
    
    spl06calib.c0 = (int16_t)buffer[0]<<4 | buffer[1]>>4;
    spl06calib.c0 = (spl06calib.c0 & 0x0800) ? (spl06calib.c0 | 0xF000) : spl06calib.c0;

    spl06calib.c1 = (int16_t)(buffer[1] & 0x0F)<<8 | buffer[2];
    spl06calib.c1 = (spl06calib.c1 & 0x0800) ? (spl06calib.c1 | 0xF000) : spl06calib.c1;

    spl06calib.c00 = (int32_t)buffer[3]<<12 | (int32_t)buffer[4]<<4 | (int32_t)buffer[5]>>4;
    spl06calib.c00 = (spl06calib.c00 & 0x080000) ? (spl06calib.c00 | 0xFFF00000) : spl06calib.c00;

    spl06calib.c10 = (int32_t)(buffer[5] & 0x0F)<<16 | (int32_t)buffer[6]<<8 | (int32_t)buffer[7];
    spl06calib.c10 = (spl06calib.c10 & 0x080000) ? (spl06calib.c10 | 0xFFF00000) : spl06calib.c10;

    spl06calib.c01 = (int16_t)buffer[8]<<8 | buffer[9];
    spl06calib.c11 = (int16_t)buffer[10]<<8 | buffer[11];
    spl06calib.c20 = (int16_t)buffer[12]<<8 | buffer[13];
    spl06calib.c21 = (int16_t)buffer[14]<<8 | buffer[15];
    spl06calib.c30 = (int16_t)buffer[16]<<8 | buffer[17];
}

/**
 * @brief   ���ö�Ӧ�������Ĳ��������Լ���������
 * @param   sensor:         ����������   
 * @param   measureRate:    ��������   
 * @param   oversamplRate:  ��������   
 * @retval  ��
 */
void atk_spl06_rateset(spl06Sensor_e sensor, uint8_t measureRate, uint8_t oversamplRate)
{
    uint8_t reg;
    
    if (sensor == PRESURE_SENSOR)           /* ѹ�������� */
    {
        kp = scale_factor[oversamplRate];
        atk_spl06_write_byte(SPL06_PRESSURE_CFG_REG, measureRate<<4 | oversamplRate);
        
        if (oversamplRate > SPL06_OVERSAMP_8)
        {
            atk_spl06_read_nbytes(SPL06_INT_FIFO_CFG_REG, &reg, 1);
            atk_spl06_write_byte(SPL06_INT_FIFO_CFG_REG, reg | 0x04);
        }
    }
    else if (sensor == TEMPERATURE_SENSOR)  /* �¶ȴ����� */
    {
        kt = scale_factor[oversamplRate];
        atk_spl06_write_byte(SPL06_TEMPERATURE_CFG_REG, measureRate<<4 | oversamplRate | 0x80); /* Using mems temperature */
        if (oversamplRate > SPL06_OVERSAMP_8)
        {
            atk_spl06_read_nbytes(SPL06_INT_FIFO_CFG_REG, &reg, 1);
            atk_spl06_write_byte(SPL06_INT_FIFO_CFG_REG, reg | 0x08);
        }
    }
}

/**
 * @brief   ��ȡ�¶�ֵ���棩
 * @param   temperature:    ԭʼ���¶�����        
 * @retval  ����� ���յĲ����¶�ֵ
 */
float atk_spl06_get_temperature(int32_t temperature)
{
    float fTCompensate;
    float fTsc;

    fTsc = temperature / (float)kt;
    fTCompensate =  spl06calib.c0 * 0.5 + spl06calib.c1 * fTsc;
    return fTCompensate;
}

/**
 * @brief   ��ȡ��ѹֵ��Pa��
 * @param   pressure:       ԭʼѹ������ 
 * @param   temperature:    ԭʼ���¶�����        
 * @retval  ����� ���յĲ�����ѹֵ
 */
float atk_spl06_get_pressure(int32_t pressure, int32_t temperature)
{
    float fTsc, fPsc;
    float qua2, qua3;
    float fPCompensate;

    fTsc = temperature / (float)kt;
    fPsc = pressure / (float)kp;
    qua2 = spl06calib.c10 + fPsc * (spl06calib.c20 + fPsc* spl06calib.c30);
    qua3 = fTsc * fPsc * (spl06calib.c11 + fPsc * spl06calib.c21);
    /* qua3 = 0.9f *fTsc * fPsc * (spl06Calib.c11 + fPsc * spl06Calib.c21); */

    fPCompensate = spl06calib.c00 + fPsc * qua2 + fTsc * spl06calib.c01 + qua3;
    /* fPCompensate = spl06Calib.c00 + fPsc * qua2 + 0.9f *fTsc  * spl06Calib.c01 + qua3; */
    return fPCompensate;
}

/**
 * @brief   ��mΪ��λ,��ѹ��ת��Ϊ��ƽ�����ϸ߶�(ASL)
 * @param   pressure:   ����������ѹ������,ע����ѹֵ�ĵ�λ��Ϊhpa 
 * @retval  ���θ߶ȣ�m��
 */
float atk_spl06_pressure_to_asl(float pressure)
{   
    if(pressure)
    {
        return 44330.f * (powf((1015.7f / pressure), 0.190295f) - 1.0f);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   ��ȡspl06����
 * @param   spl06_result_t *p_res: �ýṹ�����ڴ�Ż�ȡ�����¶ȡ�ѹ�������θ߶�����
 * @retval  ��
 */
void atk_spl06_get_data(spl06_result_t *p_res)
{
    uint8_t data[SPL06_DATA_FRAME_SIZE];
    
    /* ��ȡ�Ĵ����е���ѹֵ���¶�ֵ */
    atk_spl06_read_nbytes(SPL06_PRESSURE_MSB_REG, data, SPL06_DATA_FRAME_SIZE);
    /* �õ���ѹ���� */
    p_res->praw = (int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2];
    p_res->praw = (p_res->praw & 0x800000) ? (0xFF000000 | p_res->praw) : p_res->praw;
    /* �õ��¶����� */
    p_res->traw = (int32_t)data[3] << 16 | (int32_t)data[4] << 8 | (int32_t)data[5];
    p_res->traw = (p_res->traw & 0x800000) ? (0xFF000000 | p_res->traw) : p_res->traw;
    
    /* ��ȡ�����Ĳ����¶�ֵ����ѹֵ */
    p_res->tcomp = atk_spl06_get_temperature(p_res->traw);                      /* ��λ�� */
    p_res->pcomp = atk_spl06_get_pressure(p_res->praw, p_res->traw) / 100;      /* ��λhPa */ 
    p_res->asl = atk_spl06_pressure_to_asl(p_res->pcomp);                       /* ת���ɺ��θ߶� */   
}


/**
 * @brief       ��ʼ��SPL06
 * @param       ��
 * @retval      �����
 *              0: ���ɹ�
 *              1: ���ʧ��
 */
uint8_t atk_spl06_init(void)
{
    uint8_t spl06_id = 0, res = 0;
    
    /* ��ʼ��IIC�ӿ� */
    iic_init();                               
    delay_ms(40);                                   /* ��ʱ40ms����Ӳ���ȶ� */
    
    spl06_id = atk_spl06_read_byte(SPL06_CHIP_ID);
    if(spl06_id == SPL06_DEFAULT_CHIP_ID)
    {
        printf("SPL06 ID: 0x%X\r\n", spl06_id);
        res = 0;
    }
    else
    {
        return 1;
    }
    /* ��ȡУ׼���� */
    atk_spl06_get_calib_param();
    
    /* ����ѹ�����¶ȴ������Ĳ��������Լ��������� */
    atk_spl06_rateset(PRESURE_SENSOR, SPL06_MWASURE_16, SPL06_OVERSAMP_64);
    atk_spl06_rateset(TEMPERATURE_SENSOR, SPL06_MWASURE_16, SPL06_OVERSAMP_64);
    
    /* ���������ɼ�ģʽ�������ɼ��¶Ⱥ�ѹ�� */
    res = atk_spl06_write_byte(SPL06_MODE_CFG_REG, SPL06_CONTINUOUS_MODE);
    return res;
}


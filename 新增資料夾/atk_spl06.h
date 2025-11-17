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

#ifndef _SPL06_H
#define _SPL06_H
#include "stm32f1xx_it.h"
//#include "./SYSTEM/sys/sys.h"
//#include "./SYSTEM/delay/delay.h"

/******************************************************************************************/
/* ������ַ */
#define SPL06_I2C_ADDR                  (0x76)  /* SDO = 0 ��ַΪ��0X76, SDO = 1 ��ַΪ��0X77 */

/* �Ĵ�����ַ */
#define SPL06_DEFAULT_CHIP_ID           (0x10)

#define SPL06_PRESSURE_MSB_REG          (0x00)  /* Pressure MSB Register */
#define SPL06_PRESSURE_LSB_REG          (0x01)  /* Pressure LSB Register */
#define SPL06_PRESSURE_XLSB_REG         (0x02)  /* Pressure XLSB Register */
#define SPL06_TEMPERATURE_MSB_REG       (0x03)  /* Temperature MSB Reg */
#define SPL06_TEMPERATURE_LSB_REG       (0x04)  /* Temperature LSB Reg */
#define SPL06_TEMPERATURE_XLSB_REG      (0x05)  /* Temperature XLSB Reg */
#define SPL06_PRESSURE_CFG_REG          (0x06)  /* Pressure configuration Reg */
#define SPL06_TEMPERATURE_CFG_REG       (0x07)  /* Temperature configuration Reg */
#define SPL06_MODE_CFG_REG              (0x08)  /* Mode and Status Configuration */
#define SPL06_INT_FIFO_CFG_REG          (0x09)  /* Interrupt and FIFO Configuration */
#define SPL06_INT_STATUS_REG            (0x0A)  /* Interrupt Status Reg */
#define SPL06_FIFO_STATUS_REG           (0x0B)  /* FIFO Status Reg */
#define SPL06_RST_REG                   (0x0C)  /* Softreset Register */
#define SPL06_CHIP_ID                   (0x0D)  /* Chip ID Register */
#define SPL06_COEFFICIENT_CALIB_REG     (0x10)  /* Coeffcient calibraion Register */

#define SPL06_CALIB_COEFFICIENT_LENGTH  (18)
#define SPL06_DATA_FRAME_SIZE           (6)

#define SPL06_CONTINUOUS_MODE           (0x07)

#define TEMPERATURE_INTERNAL_SENSOR     (0)
#define TEMPERATURE_EXTERNAL_SENSOR     (1)

/* �������� times / S */
#define SPL06_MWASURE_1                 (0x00)
#define SPL06_MWASURE_2                 (0x01)
#define SPL06_MWASURE_4                 (0x02)
#define SPL06_MWASURE_8                 (0x03)
#define SPL06_MWASURE_16                (0x04)
#define SPL06_MWASURE_32                (0x05)
#define SPL06_MWASURE_64                (0x06)
#define SPL06_MWASURE_128               (0x07)

/* �������� */
#define SPL06_OVERSAMP_1                (0x00)
#define SPL06_OVERSAMP_2                (0x01)
#define SPL06_OVERSAMP_4                (0x02)
#define SPL06_OVERSAMP_8                (0x03)
#define SPL06_OVERSAMP_16               (0x04)
#define SPL06_OVERSAMP_32               (0x05)
#define SPL06_OVERSAMP_64               (0x06)
#define SPL06_OVERSAMP_128              (0x07)

/******************************************************************************************/
/* ���������� */
typedef enum 
{
    PRESURE_SENSOR, 
    TEMPERATURE_SENSOR
}spl06Sensor_e;

/* ���ڴ洢У׼���� */
typedef struct 
{
    uint16_t c0;
    uint16_t c1;
    uint32_t c00;
    uint32_t c10;
    uint16_t c01;
    uint16_t c11;
    uint16_t c20;
    uint16_t c21;
    uint16_t c30;
} spl06_calibcoeff_t;

/* ���ڴ洢�¶ȡ���ѹֵ�����θ߶� */
typedef struct 
{
    uint32_t praw;   /* ��ѹԭʼֵ */
    uint32_t traw;   /* �¶�ԭʼֵ */
    float   pcomp;  /* ��������ѹֵ hpa */
    float   tcomp;  /* �������¶�ֵ �� */
    float   asl;    /* ͨ����ѹֵת����ĺ��θ߶� m */
} spl06_result_t;

/******************************************************************************************/
/* �ⲿ�ӿں���*/
uint8_t atk_spl06_init(void);                                       /* ��ʼ��SPL06 */
void atk_spl06_get_data(spl06_result_t *p_res);                     /* ��ȡSPL06���� */

#endif

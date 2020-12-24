/*
 * DS1302.h
 *
 *  Created on: 2020��12��23��
 *      Author: yang
 */

#ifndef APP_INCLUDE_DRIVER_DS1302_H_
#define APP_INCLUDE_DRIVER_DS1302_H_


#ifndef DS1302_C_
#define DS1302_Cx_ extern
#else
#define DS1302_Cx_
#endif

#include "c_types.h"

//��ֲ������Ƭ��,���޸�����ĳ���
/*GPIO14 -- RST*/
#define DS1302_RST_MUX PERIPHS_IO_MUX_MTMS_U
#define DS1302_RST_GPIO 14
#define DS1302_RST_FUNC FUNC_GPIO14
/*GPIO12 -- DTA*/
#define DS1302_DAT_MUX PERIPHS_IO_MUX_MTDI_U
#define DS1302_DAT_GPIO 12
#define DS1302_DAT_FUNC FUNC_GPIO12
/*GPIO13 -- CLK*/
#define DS1302_CLK_MUX PERIPHS_IO_MUX_MTCK_U
#define DS1302_CLK_GPIO 13
#define DS1302_CLK_FUNC FUNC_GPIO13


//����, ��ַ
#define ds1302_sec_add			0x80		//�����ݵ�ַ
#define ds1302_min_add			0x82		//�����ݵ�ַ
#define ds1302_hr_add			0x84		//ʱ���ݵ�ַ
#define ds1302_date_add			0x86		//�����ݵ�ַ
#define ds1302_month_add		0x88		//�����ݵ�ַ
#define ds1302_day_add			0x8a		//�������ݵ�ַ
#define ds1302_year_add			0x8c		//�����ݵ�ַ
#define ds1302_control_add		0x8e		//�������ݵ�ַ
#define ds1302_charger_add		0x90		//���Ĵ���
#define ds1302_burstwr_add		0xbe		//ͻ��ģʽд����
#define ds1302_burstre_add		0xbf		//ͻ��ģʽ������
#define ds1302_ram_add			0xc0		//RAM��ַ
//����
#define ds1302_clkoff		0x80		//��ͣʱ��
#define ds1302_lock			0x80		//��д����
#define ds1302_unlock		0x00		//�ر�д����
#define ds1302_lv6			0xa5		//0.7vѹ��,2K��ֵ,2.15ma
#define ds1302_lv5			0xa9		//1.4vѹ��,2K��ֵ,1.80ma
#define ds1302_lv4			0xa6		//0.7vѹ��,4K��ֵ,1.07ma
#define ds1302_lv3			0xaa		//1.7vѹ��,4K��ֵ,0.90ma
#define ds1302_lv2			0xa7		//0.7vѹ��,8K��ֵ,0.50ma

struct ds1302struct
{
  int	tm_sec;//��
  int	tm_min;//��
  int	tm_hour;//ʱ
  int	tm_mday;//��
  int	tm_mon;//��
  int	tm_year;//��
  int	tm_wday;//����
};


//��ʼ������,��ֲ������Ƭ�����滻�ڲ�����
void ds1302_gpio_init(void);
//��ʱus,��ֲ������Ƭ�����滻�ڲ�����
void ds1302_delay_us(char us);
//����ʱ����������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ds1302_clk_set(char HL);
//����RST��������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ds1302_rst_set(char HL);
//����������������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ds1302_dat_set(char HL);
//��ȡ�������ŵĸߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
char ds1302_dat_get();


/**
* @brief  дһ���ֽ����ݵ�оƬ
* @param  data:Ҫд�������
* @param  None
* @param  None
* @retval None
* @example
**/
void ds1302_write_byte(char data);


/**
* @brief  ��ȡһ���ֽ�����
* @param  None
* @param  None
* @param  None
* @retval None
* @example
**/
char ds1302_read_byte();


/**
* @brief  д���ݵ�оƬ
* @param  addr:Ҫд��ĵ�ַ
* @param  data:Ҫд�������
* @param  rc:1-�����ڲ�RAM
* @retval None
* @example
**/
void ds1302_write_data(char addr,char data,char rc);


/**
* @brief  ��ȡ����
* @param  addr:��ȡ�ĵ�ַ
* @param  rc:1-�����ڲ�RAM
* @param  None
* @retval None
* @example
**/
char ds1302_read_data(char addr,char rc);


/**
* @brief  ����оƬ��ʱ��
* @param  tm
* @param  model:0-��ͨģʽ  1-ͻ��ģʽ
* @param  None
* @retval None
* @example
* struct ds1302struct lcTime;
* lcTime.tm_year = 2020;
* lcTime.tm_mon = 1;
* lcTime.tm_mday = 1;
* lcTime.tm_hour = 12;
* lcTime.tm_min = 10;
* lcTime.tm_sec = 30;
* ds1302_set_time(&lcTime,1);
**/
void ds1302_set_time(struct ds1302struct *lcTime,char model);


/**
* @brief  ��ȡоƬ��ʱ��
* @param  tm
* @param  model:0-��ͨģʽ  1-ͻ��ģʽ
* @param  None
* @retval None
* @example
* struct ds1302struct lcTime;
* ds1302_read_time(&lcTime,1);
* int year = lcTime.tm_year;
* int mon = lcTime.tm_mon;
* int mday = lcTime.tm_mday;
* char hour = lcTime.tm_hour;
* char min = lcTime.tm_min;
* char sec = lcTime.tm_sec;
**/
void ds1302_read_time(struct ds1302struct *lcTime,char model);


#endif /* APP_INCLUDE_DRIVER_DS1302_H_ */

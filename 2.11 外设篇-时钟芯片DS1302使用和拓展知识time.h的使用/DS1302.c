/*
 * DS1302.C
 *
 *  Created on: 2020��12��23��
 *      Author: yang
 *      ʹ��:����ʱ��
		struct ds1302struct lcTime;
		lcTime.tm_year = 20;//��(оƬֻ�ܴ洢��λ 00-99,���忴�ֲ�)
		lcTime.tm_mon = 12;//��
		lcTime.tm_mday = 23;//��
		lcTime.tm_hour = 22;//ʱ
		lcTime.tm_min = 55;//��
		lcTime.tm_sec = 5;//��
		ds1302_set_time(&lcTime,1);

		ʹ��:��ȡʱ��
		struct ds1302struct lcTime;
		ds1302_read_time(&lcTime,1);//��ȡʱ�ӵ�ʱ��
		//��ӡ �� �� �� ʱ �� ��
		os_printf("\nyear=%d,mon=%d,mday=%d,hour=%d,min=%d,sec=%d\n",
				lcTime.tm_year,
				lcTime.tm_mon,
				lcTime.tm_mday,
				lcTime.tm_hour,
				lcTime.tm_min,
				lcTime.tm_sec
				);
 */
#define DS1302_C_
#include <time.h>
#include "driver/DS1302.h"
#include "eagle_soc.h"
#include "osapi.h"
#include "gpio.h"

//��ʼ������,��ֲ������Ƭ�����滻�ڲ�����
void ICACHE_FLASH_ATTR
ds1302_gpio_init(void) {
	ETS_GPIO_INTR_DISABLE();

	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);

    PIN_FUNC_SELECT(DS1302_RST_MUX, DS1302_RST_FUNC);
	PIN_FUNC_SELECT(DS1302_DAT_MUX, DS1302_DAT_FUNC);
	PIN_FUNC_SELECT(DS1302_CLK_MUX, DS1302_CLK_FUNC);

	GPIO_OUTPUT_SET(DS1302_RST_GPIO, 0);
	GPIO_OUTPUT_SET(DS1302_DAT_GPIO, 0);
	GPIO_OUTPUT_SET(DS1302_CLK_GPIO, 0);

    ETS_GPIO_INTR_ENABLE() ;
}

//��ʱus,��ֲ������Ƭ�����滻�ڲ�����
void ICACHE_FLASH_ATTR
ds1302_delay_us(char us){
	os_delay_us(us);
}
//����ʱ����������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ICACHE_FLASH_ATTR
ds1302_clk_set(char HL){
	if(HL==1)
		GPIO_OUTPUT_SET(DS1302_CLK_GPIO, 1);
	else
		GPIO_OUTPUT_SET(DS1302_CLK_GPIO, 0);
}
//����RST��������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ICACHE_FLASH_ATTR
ds1302_rst_set(char HL){
	if(HL==1)
		GPIO_OUTPUT_SET(DS1302_RST_GPIO, 1);
	else
		GPIO_OUTPUT_SET(DS1302_RST_GPIO, 0);
}
//����������������ߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
void ICACHE_FLASH_ATTR
ds1302_dat_set(char HL){
	if(HL==1)
		GPIO_OUTPUT_SET(DS1302_DAT_GPIO, 1);
	else
		GPIO_OUTPUT_SET(DS1302_DAT_GPIO, 0);
}

//��ȡ�������ŵĸߵ͵�ƽ,��ֲ������Ƭ�����滻�ڲ�����
char ICACHE_FLASH_ATTR
ds1302_dat_get(){
	return GPIO_INPUT_GET(DS1302_DAT_GPIO);
}


/**
* @brief  дһ���ֽ����ݵ�оƬ
* @param  data:Ҫд�������
* @param  None
* @param  None
* @retval None
* @example
**/
void ICACHE_FLASH_ATTR
ds1302_write_byte(char data){
	char i=0;

	for (i = 0; i < 8; i++){
		ds1302_clk_set(0);//�ȱ���CLKΪ�͵�ƽ
		ds1302_delay_us(1);

		//׼������
		if (data & 0x01)
			ds1302_dat_set(1);
		else
			ds1302_dat_set(0);

		//����������
		ds1302_delay_us(1);
		ds1302_clk_set(1);
		ds1302_delay_us(1);

		data = data >> 1;
	}
}


/**
* @brief  ��ȡһ���ֽ�����
* @param  None
* @param  None
* @param  None
* @retval None
* @example
**/
char ICACHE_FLASH_ATTR
ds1302_read_byte(){
	char i=0, temp=0;

	//��ȡ����
	for (i = 0; i < 8;i++) {
		ds1302_clk_set(0);//�����˵�һ���½���
		ds1302_delay_us(1);

		temp = temp >> 1;
		if (ds1302_dat_get()){//��ȡ����
			temp |= 0x80;
		}

		ds1302_delay_us(1);
		ds1302_clk_set(1);
		ds1302_delay_us(1);
	}
	return temp;
}

/**
* @brief  д���ݵ�оƬ
* @param  addr:Ҫд��ĵ�ַ
* @param  data:Ҫд�������
* @param  rc:1-�����ڲ�RAM
* @retval None
* @example
**/
void ICACHE_FLASH_ATTR
ds1302_write_data(char addr,char data,char rc){
	ds1302_clk_set(0);//�ȱ���CLKΪ�͵�ƽ
	ds1302_delay_us(1);
	ds1302_rst_set(1);//���ô���
	ds1302_delay_us(1);

	addr = addr & 0xFE;//�������λ,��0(д������)
	if(rc==1) addr = addr | 0x40; //����RAM
	ds1302_write_byte(addr);
	ds1302_write_byte(data);

	ds1302_rst_set(0);//ֹͣ����
	ds1302_delay_us(1);
}


/**
* @brief  ��ȡ����
* @param  addr:��ȡ�ĵ�ַ
* @param  rc:1-�����ڲ�RAM
* @param  None
* @retval None
* @example
**/
char ICACHE_FLASH_ATTR
ds1302_read_data(char addr,char rc){
	char temp;
	ds1302_clk_set(0);//�ȱ���CLKΪ�͵�ƽ
	ds1302_delay_us(1);
	ds1302_rst_set(1);//���ô���
	ds1302_delay_us(1);

	addr = addr | 0x01;//�������λ��1(��ȡ����)
	if(rc==1) addr = addr | 0x40; //����RAM
	ds1302_write_byte(addr);

	temp = ds1302_read_byte();

	ds1302_rst_set(0);//ֹͣ����
	ds1302_delay_us(1);
	return temp;
}

/**
* @brief  д���ݵ�оƬ(ͻ��ģʽ)
* @param  data:Ҫд���8�ֽ�����
* @param  None
* @param  None
* @retval None
* @example
**/
void ICACHE_FLASH_ATTR
ds1302_write_data_burst(char *data){
	int i=0;
	ds1302_clk_set(0);//�ȱ���CLKΪ�͵�ƽ
	ds1302_delay_us(1);
	ds1302_rst_set(1);//���ô���
	ds1302_delay_us(1);

	ds1302_write_byte(ds1302_burstwr_add);

	for (i=0; i<8; i++){ //����д�� 8 �ֽ�����
		ds1302_write_byte(data[i]);
	}

	ds1302_rst_set(0);//ֹͣ����
	ds1302_delay_us(1);
}

/**
* @brief  ��ȡ����(ͻ��ģʽ)
* @param  data:��ȡ�ĵ�ַ
* @param  None
* @param  None
* @retval None
* @example
**/
void ICACHE_FLASH_ATTR
ds1302_read_data_burst(char *data){
	int i=0;
	ds1302_clk_set(0);//�ȱ���CLKΪ�͵�ƽ
	ds1302_delay_us(1);
	ds1302_rst_set(1);//���ô���
	ds1302_delay_us(1);

	ds1302_write_byte(ds1302_burstre_add);
	for (i=0; i<8; i++){ //����д�� 8 �ֽ�����
		data[i] = ds1302_read_byte();
	}

	ds1302_rst_set(0);//ֹͣ����
	ds1302_delay_us(1);
}

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
void ICACHE_FLASH_ATTR
ds1302_set_time(struct ds1302struct *lcTime,char model){
	char temp[8];//ͻ��ģʽ�»�������
//	char value,valueH,valueL;
//��ΪоƬ����λ�洢ʮλ,����λ�洢��λ
//�����û�����12  ��ô��Ҫ��λ�� 0001  ��λ�� 0010   Ҳ����0x12  Ȼ��洢��оƬ��
//	valueH = (lcTime->tm_year/10)<<4;
//	valueL = lcTime->tm_year%10;
//	value = valueH | valueL;
	if(model){//ͻ��ģʽ
		temp[0] = (lcTime->tm_sec/10)<<4|lcTime->tm_sec%10;		//��
		temp[1] = (lcTime->tm_min/10)<<4|lcTime->tm_min%10;		//��
		temp[2] = (lcTime->tm_hour/10)<<4|lcTime->tm_hour%10;	//ʱ
		temp[3] = (lcTime->tm_mday/10)<<4|lcTime->tm_mday%10;	//��
		temp[4] = (lcTime->tm_mon/10)<<4|lcTime->tm_mon%10;		//��
		temp[5] = (lcTime->tm_wday/10)<<4|lcTime->tm_wday%10;	//����
		temp[6] = (lcTime->tm_year/10)<<4|lcTime->tm_year%10;	//��
		//���һλ��д�����Ĵ���,ȫ��0�Ϳ���,���Բ���Ҫ����
		ds1302_write_data_burst(temp);
	}
	else{
		ds1302_write_data(ds1302_control_add,ds1302_unlock,0);		//�ر�д����
		ds1302_write_data(ds1302_sec_add,ds1302_clkoff,0);			//��ͣʱ��
		//ds1302_write_data(ds1302_charger_add,ds1302_lv5);	    //������
		ds1302_write_data(ds1302_year_add,(lcTime->tm_year/10)<<4|lcTime->tm_year%10,0);//��
		ds1302_write_data(ds1302_month_add,(lcTime->tm_mon/10)<<4|lcTime->tm_mon%10,0);	//��
		ds1302_write_data(ds1302_date_add,(lcTime->tm_mday/10)<<4|lcTime->tm_mday%10,0);//��
		ds1302_write_data(ds1302_hr_add,(lcTime->tm_hour/10)<<4|lcTime->tm_hour%10,0);	//ʱ
		ds1302_write_data(ds1302_min_add,(lcTime->tm_min/10)<<4|lcTime->tm_min%10,0);	//��
		ds1302_write_data(ds1302_sec_add,(lcTime->tm_sec/10)<<4|lcTime->tm_sec%10,0);	//��
		ds1302_write_data(ds1302_day_add,(lcTime->tm_wday/10)<<4|lcTime->tm_wday%10,0);	//��
		ds1302_write_data(ds1302_control_add,ds1302_lock,0);		//��д����
	}
}


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
void ICACHE_FLASH_ATTR
ds1302_read_time(struct ds1302struct *lcTime,char model){
	char temp[8];//ͻ��ģʽ�»�������
	char value,valueH,valueL;
	//��ʾ:�������ݵĸ���λ����ʮλ,����λ�����λ
	//��Щ���ݵ�MSB���λ������������,��μ�DS1302�ֲ�

	if(model){//ͻ��ģʽ
		ds1302_read_data_burst(temp);
		valueH = (temp[6]>>4)&0x0f;
		valueL = temp[6]&0x0f;
		lcTime->tm_year = valueH*10 + valueL;

		valueH = (temp[5]>>4)&0x0f;
		valueL = temp[5]&0x0f;
		lcTime->tm_wday = valueH*10 + valueL;

		valueH = (temp[4]>>4)&0x0f;
		valueL = temp[4]&0x0f;
		lcTime->tm_mon = valueH*10 + valueL;

		valueH = (temp[3]>>4)&0x0f;
		valueL = temp[3]&0x0f;
		lcTime->tm_mday = valueH*10 + valueL;

		valueH = (temp[2]>>4)&0x0f;
		valueL = temp[2]&0x0f;
		lcTime->tm_hour = valueH*10 + valueL;

		valueH = (temp[1]>>4)&0x0f;
		valueL = temp[1]&0x0f;
		lcTime->tm_min = valueH*10 + valueL;

		valueH = (temp[0]>>4)&0x0f;
		valueL = temp[0]&0x0f;
		lcTime->tm_sec = valueH*10 + valueL;
	}else{
		value = ds1302_read_data(ds1302_year_add,0);	 //��
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_year = valueH*10 + valueL;

		value = ds1302_read_data(ds1302_month_add,0);	 //��
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_mon = valueH*10 + valueL;

		value = ds1302_read_data(ds1302_date_add,0);	 //��
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_mday = valueH*10 + valueL;

		value = ds1302_read_data(ds1302_hr_add,0);		 //ʱ
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_hour = valueH*10 + valueL;

		value = ds1302_read_data(ds1302_min_add,0);		 //��
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_min = valueH*10 + valueL;

		value = (ds1302_read_data(ds1302_sec_add,0))&0x7f;//�룬������ĵ�7λ�����ⳬ��59
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_sec = valueH*10 + valueL;

		value = ds1302_read_data(ds1302_day_add,0);		 //��
		valueH = (value>>4)&0x0f;
		valueL = value&0x0f;
		lcTime->tm_wday = valueH*10 + valueL;
	}
}



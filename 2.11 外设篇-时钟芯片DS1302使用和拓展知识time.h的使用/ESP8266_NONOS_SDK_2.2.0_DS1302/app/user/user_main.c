/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_devicefind.h"
#include "user_webserver.h"
#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif
#include "driver/uart.h" //����uart.h
#include "driver/BufferManage.h"
#include "driver/mDNS.h"

#include "user_tcpclient.h"

#include <time.h>
#include "driver/DS1302.h"


/*******���ڽ��ջ���********/
#define UartReadbuffLen 2048
#define UartManagebuffLen 60
u8  UartReadbuff[UartReadbuffLen];//���洮�ڽ��յ�ÿһ������
u32 UartManagebuff[UartManagebuffLen];//���������������

u8  UartReadbuffCopy[UartReadbuffLen];//��ȡ��������


os_timer_t os_timer_one;//���������ʱ���ṹ�����

uint32 priv_param_start_sec;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            priv_param_start_sec = 0x3C;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            priv_param_start_sec = 0x7C;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
            rf_cal_sec = 512 - 5;
            priv_param_start_sec = 0x7C;
            break;
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            priv_param_start_sec = 0xFC;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
            rf_cal_sec = 1024 - 5;
            priv_param_start_sec = 0x7C;
            break;
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            priv_param_start_sec = 0xFC;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            priv_param_start_sec = 0xFC;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            priv_param_start_sec = 0xFC;
            break;
        default:
            rf_cal_sec = 0;
            priv_param_start_sec = 0;
            break;
    }

    return rf_cal_sec;
}
void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

/**
* @brief   ��ʱ���ص�����
* @param   parg:���������os_timer_setfn�������Ĳ���
* @param   None
* @param   None
* @param   None
* @retval  None
* @warning None
* @example
**/
int timercnt=0;
int timercnt1=0;
void os_timer_one_function(void *parg)
{
	struct ds1302struct lcTime;

	timercnt++;
	if(timercnt>=1000){//1S
		timercnt=0;

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
	}

	BufferManageRead(&buff_manage,UartReadbuffCopy,&buff_manage.ReadLen);/*ȡ�����������*/
	if(buff_manage.ReadLen>0){/*����ȡ��������*/
		user_tcp_send_data(UartReadbuffCopy,buff_manage.ReadLen);
	}
}


void WifiConnectCallback(uint8_t status)
{
	if(status == STATION_GOT_IP){
		os_printf("\nConnect AP Success\n");
	}
	else {
		os_printf("\nDisConnect AP\n");
	}
}



/**
* @brief   ϵͳ��ʼ�����
* @param   None
* @retval  None
* @warning None
* @example
**/
void ICACHE_FLASH_ATTR
system_init_done(void)
{
	struct ds1302struct lcTime;

	ds1302_gpio_init();//��ʼ������

	lcTime.tm_year = 20;//��(оƬֻ�ܴ洢��λ 00-99,���忴�ֲ�)
	lcTime.tm_mon = 12;//��
	lcTime.tm_mday = 23;//��
	lcTime.tm_hour = 22;//ʱ
	lcTime.tm_min = 55;//��
	lcTime.tm_sec = 5;//��
	ds1302_set_time(&lcTime,1);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	BufferManageCreate(&buff_manage, UartReadbuff, UartReadbuffLen, UartManagebuff, UartManagebuffLen*4);//��������
	uart_init_2(BIT_RATE_115200,BIT_RATE_115200);

	//���ö�ʱ��
	os_timer_setfn(&os_timer_one,os_timer_one_function,NULL);//os_timer_one:��ʱ���ṹ�����    os_timer_one_function:�ص�����    yang:�����ص������Ĳ���
	//ʹ�ܶ�ʱ��
	os_timer_arm(&os_timer_one,1,1);//os_timer_one:��ʱ������        1:1ms��һ��    1:ѭ��

	system_init_done_cb(system_init_done);

	/*�������ӵ�·����*/
//	WIFI_Connect("QQQQQ","11223344",WifiConnectCallback);
//
//	user_tcp_init("192.168.0.100",8888);//��ʼ�����ӵķ�������Ϣ
//	user_tcp_connect();//��������
}


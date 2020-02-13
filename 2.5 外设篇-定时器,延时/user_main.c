/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#include "esp_common.h"
#include "gpio.h"
#include "esp_timer.h"
#include "hw_timer.h"

os_timer_t os_timer_one;//����һ��ȫ�ֵĶ�ʱ���ṹ�����

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
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
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
void os_timer_one_function(void *parg)
{
	printf("parg:%s\n", parg);//��ӡһ�´������Ĳ���
}

/**
* @brief   Ӳ����ʱ���жϻص�����
* @param   None
* @param   None
* @param   None
* @param   None
* @retval  None
* @warning None
* @example
**/
u32 cnt = 0;
void hw_test_timer_cb(void)
{
	cnt++;
	if(cnt>1000)//1S
	{
		cnt=0;
		printf("1111111111111\n");//��ӡ
	}
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	uart_init_new();
    printf("SDK version:%s\n", system_get_sdk_version());
	printf("Ai-Thinker Technology Co. Ltd.\r\n%s %s\r\n", __DATE__, __TIME__);
	//���ö�ʱ��
	os_timer_setfn(&os_timer_one,os_timer_one_function,"yang");//os_timer_one:��ʱ���ṹ�����	os_timer_one_function:�ص�����	yang:�����ص������Ĳ���
	//ʹ�ܶ�ʱ��
	os_timer_arm(&os_timer_one,500,1);//os_timer_one:��ʱ������		500:500ms��һ��	1:ѭ��
	os_timer_disarm(&os_timer_one);//�����ʱ��

	//��ʱ����ʼ��
	hw_timer_init(1);//1:ѭ��
	//���ö�ʱ���ص�����
	hw_timer_set_func(hw_test_timer_cb);//hw_test_timer_cb:Ӳ����ʱ���жϻص�����
	hw_timer_arm(1000);//1000:1000us��ʱ�����жϺ���
}


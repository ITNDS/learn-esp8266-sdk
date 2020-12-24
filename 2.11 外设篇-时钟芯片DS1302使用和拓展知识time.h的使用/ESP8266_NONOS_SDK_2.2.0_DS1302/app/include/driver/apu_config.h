/*
 * apu_config.h
 *
 *  Created on: 2020��4��9��
 *      Author: yang
 */

#ifndef __APU_CONFIG_H__
#define __APU_CONFIG_H__

#define  USSID "wifi_8266_bind" 		//AP��������
#define  UPWD "11223344"     			//����

typedef enum {
	APU_STATUS_GETING_DATA=0,//��ȡ��APP���͵�����
	APU_STATUS_LINK_SSID_PSWD,//��ʼ����·����
	APU_STATUS_LINKED,//��������·����
	APU_STATUS_UDPSEND,//ģ�鷵��UDP����,MAC��ַ��IP��ַ
	APU_STATUS_LINK_OVER,//����
} apuconfig_status;


typedef void (*apuconfig_callback_t)(apuconfig_status status, void *pdata);


void apuconfig_start(apuconfig_callback_t cb,uint8 time_out);
void apuconfig_stop();

#endif

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
#include "user_tcpclient.h"
#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#include "driver/uart.h" //����uart.h
#include  "espconn.h"
#include  "mem.h"
#include "driver/wifi.h"
#include "driver/mqtt.h"
#include "smartconfig.h"

#include "gpio.h"
#include "eagle_soc.h"

#include "driver/i2c_master.h"
#include "driver/oled.h"
#include "driver/dht11.h"

#include "driver/aly_hmac.h"

unsigned char MainBuffer[1460];//��������,ȫ��ͨ��
u32  MainLen=0;      //ȫ��ͨ�ñ���


//�޸�����ƽ̨��ȡ���豸��Ϣ
char ProductKey[50]="a1m7er1nJbQ";//�滻�Լ��� ProductKey
char DeviceName[50]="Mqtt";//�滻�Լ��� DeviceName
char DeviceSecret[50]="7GUrQwgDUcXWV3EIuLwdEvmRPWcl7VsU";//�滻�Լ��� DeviceSecret
char Region[50]="cn-shanghai";//����,�����Լ����޸�
char ClientID[50]="112233445566";//�޸��Լ����õ�

//����MQTT
unsigned char IP[100]="";//IP��ַ/����
unsigned int  Port = 1883;//�˿ں�
unsigned char MQTTid[100] = "";//ClientID
unsigned char MQTTUserName[100] = "";//�û���
unsigned char MQTTPassWord[100] = "";//����
unsigned char MQTTkeepAlive = 30;//������ʱ��
/*�������ṩ���Զ�����Ϣͨ�ŵ�����*/
unsigned char MQTTPublishTopicCustom[100]="";//�洢MQTT����������
unsigned char MQTTSubscribeTopicCustom[100]="";//�洢MQTT���ĵ�����
/*�������ṩ�ı�׼��ʽ��Ϣͨ�ŵ�����(��ģ��)*/
unsigned char MQTTPublishTopicModel[100]="";//�洢MQTT����������
unsigned char MQTTSubscribeTopicModel[100]="";//�洢MQTT���ĵ�����

MQTT_Client mqttClient;
MQTT_Client *client;//��ȡ���ӵ�MQTT_Client


os_timer_t os_timer_one;//���������ʱ���ṹ�����

extern u8  Usart0ReadBuff[Usart0ReadLen];//�������ݵ�����
extern u32 Usart0ReadCnt;//����1���յ������ݸ���
extern u32 Usart0IdelCnt;
extern u32 Usart0ReadCntCopy;//����1���յ������ݸ�������

char RelayState = 0;//��¼�̵���״̬
u32 RendTHCnt = 0;//�ɼ�DHT11��ʱ

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
    uint32 priv_param_start_sec;

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

void os_timer_one_function(void *parg)
{
	MQTT_Connect(&mqttClient);
}

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){

	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

//������MQTT
void mqttConnectedCb(uint32_t *args){
	client = (MQTT_Client*)args;
	os_printf("MQTT: Connected\r\n");
	MQTT_Subscribe(client, MQTTSubscribeTopicCustom, 0);//�����Զ�����Ϣͨ�ŵ�����
	MQTT_Subscribe(client, MQTTSubscribeTopicModel, 0);//��������
	os_timer_disarm(&os_timer_one);//ֹͣ��ʱ����MQTT
}

//���ӶϿ�
void mqttDisconnectedCb(uint32_t *args){
	client = (MQTT_Client*)args;
	os_printf("MQTT: Disconnected\r\n");
	os_timer_arm(&os_timer_one,3000,1);//���ö�ʱ������MQTT
}
//��������Ϣ
void mqttPublishedCb(uint32_t *args){
	client = (MQTT_Client*)args;
	os_printf("MQTT: Published\r\n");
}
//���յ�����
void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
	*dataBuf = (char*)os_zalloc(data_len+1);//���������������Ϣ

	MQTT_Client* client = (MQTT_Client*)args;
	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;
	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;


	if(os_strstr((char*)dataBuf, "\"data\":\"switch\""))//ѯ�ʿ���
	{
		if(os_strstr((char*)dataBuf, "\"bit\":\"1\""))//��һ·����
		{
			if(os_strstr((char*)dataBuf, "\"status\":\"-1\""))//ѯ��״̬
			{
				RelayState = ~RelayState;//�ı�״̬,�ü�ⴥ��
			}
			else if( os_strstr((char*)dataBuf, "\"status\":\"1\"") )
			{
				GPIO_OUTPUT_SET(5, 1);//����GPIO5����ߵ�ƽ
			}
			else if(os_strstr((char*)dataBuf, "\"status\":\"0\""))
			{
				GPIO_OUTPUT_SET(5, 0);//����GPIO5����͵�ƽ
			}
		}
	}

	os_printf("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);//��ӡ���յ���Ϣ
	os_free(topicBuf);
	os_free(dataBuf);
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
void hw_test_timer_cb(void)
{
	if(RelayState != GPIO_INPUT_GET(5))//�̵���״̬�仯,���ͼ̵���״̬
	{
		RelayState = GPIO_INPUT_GET(5);

		if(RelayState)
		{
			MainLen = os_sprintf((char*)MainBuffer,"{\"data\":\"switch\",\"bit\":\"1\",\"status\":\"1\"}");//���͵�����
		}
		else
		{
			MainLen = os_sprintf((char*)MainBuffer,"{\"data\":\"switch\",\"bit\":\"1\",\"status\":\"0\"}");//���͵�����
		}
		/*���������ṩ���Զ�����Ϣͨ�ŵ����ⷢ����Ϣ*/
		if(client != NULL){
			MQTT_Publish(client, MQTTPublishTopicCustom, MainBuffer, MainLen, 0, 0);//������Ϣ
		}
	}

    if(Usart0ReadCnt!=0){//���ڽ��յ�����
    	Usart0IdelCnt++;//����ʱ���ۼ�
        if(Usart0IdelCnt>10){//�ۼӵ�����ֵ(10ms)
        	Usart0IdelCnt=0;
            Usart0ReadCntCopy = Usart0ReadCnt;//�������յ����ݸ���
            Usart0ReadCnt=0;
            /*��������
             * ���ݻ�������:Usart0ReadBuff
             * ���ݳ���:Usart0ReadCntCopy
             * */
        }
    }

	RendTHCnt++;
	if(RendTHCnt>=2000){
		RendTHCnt=0;
		DHT11_Read_Data();
		OLED_ShowNum(55,3, DHT11Data[2],2,16);//
		OLED_ShowNum(55,5, DHT11Data[0],2,16);//

		/*���������ṩ���Զ�����Ϣͨ�ŵ����ⷢ����Ϣ*/
		MainLen = os_sprintf((char*)MainBuffer,"{\"data\":\"TH\",\"bit\":1,\"temperature\":%d,\"humidity\":%d}",DHT11Data[2],DHT11Data[0]);
		if(client != NULL)
		{
			/*���������ṩ���Զ�����Ϣͨ�ŵ����ⷢ����Ϣ*/
			MQTT_Publish(client, MQTTPublishTopicCustom, MainBuffer, MainLen, 0, 0);//������Ϣ

			/*���������ṩ�ı�׼��ʽ��Ϣͨ�ŵ�����(��ģ��)������Ϣ*/
						MainLen = os_sprintf((char*)MainBuffer,"{\"method\":\"thing.event.property.post\",\"id\":1111,\
			\"params\":{\"temp\":%d,\"humi\":%d},\"version\":\"1.0\"}",DHT11Data[2],DHT11Data[0]);
						MQTT_Publish(client, MQTTPublishTopicModel, MainBuffer, MainLen, 0, 1);//������Ϣ
		}
	}
}


/**
* @brief   ��ʼ��MQTT
* @param   None
* @retval  None
* @warning None
* @example
**/
void InitMQTT(void){
	MQTT_InitConnection(&mqttClient, IP, Port, 0);//MQTT������IP��ַ,�˿ں�,�Ƿ�SSL
	MQTT_InitClient(&mqttClient, MQTTid, MQTTUserName, MQTTPassWord, MQTTkeepAlive, 1);//ClientID,�û���,����,������ʱ��,���������Ϣ
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);//�������ӻص�
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);//���öϿ��ص�
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);//���÷�������Ϣ�ص�
	MQTT_OnData(&mqttClient, mqttDataCb);//�������ݻص�

	//���ö�ʱ������MQTT
	os_timer_setfn(&os_timer_one,os_timer_one_function,NULL);
	//ʹ�ܶ�ʱ��
	os_timer_arm(&os_timer_one,3000,1);
}

/*��ʼ��MQTT����*/
void ICACHE_FLASH_ATTR
aly_config(void){
	//���IP��ַ
	os_memset(IP,0,sizeof(IP));
	os_sprintf((char *)IP,"%s.iot-as-mqtt.%s.aliyuncs.com",ProductKey,Region);
	os_printf("\nIP:%s\n",IP);

	//����豸��ID
	os_memset(MQTTid,0,sizeof(MQTTid));
	os_sprintf(MQTTid,"%s|securemode=3,signmethod=hmacsha1|",ClientID);
	os_printf("\nMQTTid:%s\n",MQTTid);

	//����û���
	os_memset(MQTTUserName,0,sizeof(MQTTUserName));
	os_sprintf((char *)MQTTUserName,"%s&%s",DeviceName,ProductKey);
	os_printf("\nMQTTUserName:%s\n",MQTTUserName);

	//��϶��ĵ�����
	os_memset(MQTTSubscribeTopicCustom,0,sizeof(MQTTSubscribeTopicCustom));
	os_sprintf((char *)MQTTSubscribeTopicCustom,"/%s/%s/user/get",ProductKey,DeviceName);
	//��Ϸ���������
	os_memset(MQTTPublishTopicCustom,0,sizeof(MQTTPublishTopicCustom));
	os_sprintf((char *)MQTTPublishTopicCustom,"/%s/%s/user/update",ProductKey,DeviceName);


	//��϶��ĵ�����
	os_memset(MQTTSubscribeTopicModel,0,sizeof(MQTTSubscribeTopicModel));
	os_sprintf((char *)MQTTSubscribeTopicModel,"/sys/%s/%s/thing/service/property/set",ProductKey,DeviceName);
	os_printf("\nMQTTSubscribeTopic:%s\n",MQTTSubscribeTopicModel);

	//��Ϸ���������
	os_memset(MQTTPublishTopicModel,0,sizeof(MQTTPublishTopicModel));
	os_sprintf((char *)MQTTPublishTopicModel,"/sys/%s/%s/thing/event/property/post",ProductKey,DeviceName);
	os_printf("\nMQTTPublishTopic:%s\n",MQTTPublishTopicModel);


	//��ϼ�������
	os_sprintf(MainBuffer,"clientId%sdeviceName%sproductKey%s",ClientID,DeviceName,ProductKey);
	aly_hmac_sha1(MainBuffer,strlen(MainBuffer),DeviceSecret,strlen(DeviceSecret),MQTTPassWord);
	os_printf("\nMQTTPassWord:%s\n",MQTTPassWord);
}


/**
* @brief   ��ʼ��OLED
* @param   None
* @retval  None
* @warning None
* @example
**/
void ICACHE_FLASH_ATTR
InitOLED(void){
	/*<��ʼ��OLED*/
	i2c_master_gpio_init();//��ʼ������
	OLED_Init();
	OLED_Clear();
	OLED_ShowCHinese(36,0,0);//��
	OLED_ShowCHinese(54,0,1);//ʪ
	OLED_ShowCHinese(72,0,2);//��
	OLED_ShowString(25,3,"T :",16);
	OLED_ShowString(25,5,"H :",16);
	OLED_ShowCHinese(80,3,3);//��
	OLED_ShowString(80,5," %",16);
	/*��ʼ��OLED>*/
}


/**
* @brief   ��ʼ��GPIO
* @param   None
* @retval  None
* @warning None
* @example
**/
void ICACHE_FLASH_ATTR
InitGPIO(void){
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U , FUNC_GPIO0);//����GPIO0��ͨģʽ
    GPIO_OUTPUT_SET(0, 1);//����GPIO0����ߵ�ƽ

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U , FUNC_GPIO2);//����GPIO2��ͨģʽ
    GPIO_OUTPUT_SET(2, 1);//����GPIO2����ߵ�ƽ

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U , FUNC_GPIO5);//����GPIO5��ͨģʽ
    GPIO_OUTPUT_SET(5, 0);//����GPIO5����͵�ƽ
}


/**
* @brief   ϵͳ��ʼ�����
* @param   None
* @retval  None
* @warning None
* @example
**/
void system_init_done(void)
{
	InitGPIO();//��ʼ��GPIO
	InitOLED();//��ʼ��OLED
	aly_config();//��ʼ��MQTT����
	InitMQTT();//��ʼ��MQTT

    //��ʱ����ʼ��
	hw_timer_init(0,1);//1:ѭ��
	//���ö�ʱ���ص�����
	hw_timer_set_func(hw_test_timer_cb);//hw_test_timer_cb:Ӳ����ʱ���жϻص�����
	hw_timer_arm(1000);//1000:1000us��ʱ�����жϺ���

	WIFI_Connect("QQQQQ", "11223344", wifiConnectCb);//����·����
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
	uart_init_2(BIT_RATE_115200,BIT_RATE_115200);
	system_init_done_cb(system_init_done);
}


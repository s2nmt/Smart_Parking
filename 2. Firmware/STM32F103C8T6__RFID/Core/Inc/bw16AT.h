/*
 * bw16AT.h
 *
 *  Created on: Oct 5, 2023
 *      Author: Minh Tuan
 */

#ifndef INC_BW16AT_H_
#define INC_BW16AT_H_

#include "main.h"

#define _ms_    100
#define Host_Byte     30
#define ID_Byte       25
#define Username_Byte 25
#define Password_Byte 25
#define TopicSub_Byte 30
#define TopicPub_Byte 30

typedef enum { BW16_ERROR = 0, BW16_OK = !BW16_ERROR } BW16_StatusTypeDef;

typedef enum
{
	No_Flash_NULL_Mode       = 0,
	No_Flash_STA_Mode        = 1,
	No_Flash_AP_Mode         = 2,
	No_Flash_AP_and_STA_Mode = 3,
	Flash_NULL_Mode          = 4,
	Flash_STA_Mode           = 5,
	Flash_AP_Mode            = 6,
	FLASH_AP_and_STA_Mode    = 7
}BW16_Mode_HandleTypeDef;

typedef enum{
	OFF                 = 0,
	Simple_Config       = 1,
	Wifi_Config         = 2
}BW16_App_HandleTypeDef;

typedef struct
{
  uint8_t sub[TopicSub_Byte];
  uint8_t pub[TopicPub_Byte];
}Topic_HandleTypeDef;

typedef struct
{
  uint8_t host[Host_Byte];
  uint8_t port[5];
  uint8_t id[ID_Byte];
	uint8_t user[Username_Byte];
	uint8_t password[Password_Byte];
  Topic_HandleTypeDef topic;
}MQTT_StructuresTypeDef;
BW16_StatusTypeDef Bw16_Response(uint8_t *recv_buf, uint8_t *content, uint16_t timeout);
void Bw16_Echo(UART_HandleTypeDef huart, FunctionalState ctrl);
void Bw16_Configure_Mode(UART_HandleTypeDef huart, BW16_Mode_HandleTypeDef mode);
void Bw16_Check_Info_WiFi(UART_HandleTypeDef huart);
void Bw16_Set_DHCP(UART_HandleTypeDef huart,char *mode,char *ip,char *mark,char *gateway);
void Bw16_Check_Info_DHCP(UART_HandleTypeDef huart);
void Bw16_Auto_Connect_Wifi(UART_HandleTypeDef huart,FunctionalState ctrl);
void BW16_Start_SmartConfig(UART_HandleTypeDef huart, BW16_App_HandleTypeDef app);
void Bw16_RSSI(UART_HandleTypeDef huart);
void Bw16_Connect_Wifi_AP(UART_HandleTypeDef huart,uint8_t *ptrSSID, uint8_t *ptrPWD);
void Bw16_Set_Info_MQTT(UART_HandleTypeDef huart,uint8_t *key, uint8_t *data);
void Bw16_Check_Info_MQTT(UART_HandleTypeDef huart);
void Bw16_Connect_MQTT(UART_HandleTypeDef huart);
void Bw16_MQTT_Publish(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos, uint8_t *retained, uint8_t *payload);
void Bw16_MQTT_PublishRaw(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos, uint8_t *retained, uint8_t *length);
void Bw16_MQTT_Subscribe(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos);
void Bw16_MQTT_UnSubscribe(UART_HandleTypeDef huart, uint8_t *topic);
void Bw16_Reset(UART_HandleTypeDef huart);
void Bw16_Ping_MQTT(UART_HandleTypeDef huart,uint8_t *ip);
void Bw16_Setup_Default(UART_HandleTypeDef huart);
#endif /* INC_BW16AT_H_ */

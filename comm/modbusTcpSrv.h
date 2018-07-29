/*
 * modbusTcpSrv.h
 *
 *  Created on: 2018-7-22
 *      Author: Administrator
 */

#ifndef MODBUSTCPSRV_H_
#define MODBUSTCPSRV_H_
#include "type.h"
#include "port.h"
#include "para.h"

//����Ĵ�����ʼ��ַ
#define REG_INPUT_START       0x0001
//����Ĵ�������
#define REG_INPUT_NREGS       70
//���ּĴ�����ʼ��ַ
#define REG_HOLDING_START     0x0001
//���ּĴ�������
#define REG_HOLDING_NREGS     120
//��Ȧ��ʼ��ַ
#define REG_COILS_START       0x0001
//��Ȧ����
#define REG_COILS_SIZE        16
//���ؼĴ�����ʼ��ַ
#define REG_DISCRETE_START    0x0001
//���ؼĴ�������
#define REG_DISCRETE_SIZE     16


#define MB_TCP_BUF_SIZE  2048
typedef struct _TProtocolBuf
{
	uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE];   //mbЭ����ռĴ���
	uint16_t ucTCPRequestLen;  //mb���ռĴ���Э�鳤��
	uint8_t ucTCPResponseFrame[MB_TCP_BUF_SIZE];   //mbЭ�鷢�ͼĴ���
	uint16_t ucTCPResponseLen;   //mb����Э�鳤��
	uint8_t bFrameSent;   //�Ƿ���з�����Ӧ�ж�
}TProtocolBuf;

TProtocolBuf* getProtocolBuf();
uint16_t* getRegInput();
uint16_t* getRegHolding();
void* modbusServer(void* arg);
TPowerSetPara* getSetParaBak();
#endif /* MODBUSTCPSRV_H_ */

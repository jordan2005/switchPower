/*
 * para.h
 *
 *  Created on: 2018-7-15
 *      Author: Administrator
 */

#ifndef PARA_H_
#define PARA_H_

typedef unsigned short int uint16;
#pragma pack(1) // �ñ�������1�ֽڶ���
typedef struct _TGySetPara
{
	 uint16 tzdsx; //���ƶ�����
	 uint16 gdfs; //���緽ʽ
	 uint16 spsx; //��Ƶ����
	 uint16 fdbg; //���ȱȸ�
	 uint16 fdbd; //���ȱȵ�
	 uint16 zkbz; //ռ�ձ�ռ
	 uint16 zkbk; //ռ�ձȿ�
	 uint16 reserve0[5];
	 uint16 U2qy; //U2Ƿѹ
	 uint16 I1sx; //I1����
	 uint16 U2sx; //U2����
	 uint16 I2sx; //I2����
	 uint16 sfs; //������
	 uint16 xhfs; //Ϩ����ʽ
	 uint16 jyzdkg; //��ѹ����
	 uint16 jyfd; //\��ѹ����
	 uint16 I2mcfzsx; //���������ֵ����
	 uint16 I1fzsx; //I1��ֵ����
	 uint16 kgpl; //����Ƶ��
	 uint16 Udxs; //ֱ����ѹϵ��
	 uint16 I1xs; //һ�ε���ϵ��
	 uint16 U2xs; //���ε�ѹϵ��
	 uint16 I2xs; //���ε���ϵ��
	 uint16 Udxsjz; //ֱ����ѹУ��
	 uint16 I1xsjz; //һ�ε���У��
	 uint16 U2xsjz; //\���ε�ѹУ��
	 uint16 I2xsjz; //���ε���У��
	 uint16 reserve1[11];
	 uint16 Uded; //ֱ����ѹ�ֵ
	 uint16 I1ed; //һ�ε����ֵ
	 uint16 U2ed; //���ε�ѹ�ֵ
	 uint16 I2ed; //���ε����ֵ
	 uint16 ljgw; //�ٽ����
	 uint16 wxgw; //Σ�չ���
	 uint16 ljywsz; //�ٽ���������
	 uint16 wxywsz; //\Σ�չ�������
	 uint16 reserve2[10];
}TGySetPara;

#define GY_SET_PARA { 99, 1, 2, 3, 4, 5, 7, 8, 9, 10,\
					   11, 12, 14, 15, 16, 17, 18, 19, 20}

typedef struct _TDySetPara
{
	 uint16 dy1yxx; //��ѹ����1��Ч����
	 uint16 dy1gzfs; //��ѹ����1������ʽ.
	 uint16 dy1gzsj; //��ѹ����1���ʱ��
	 uint16 dy1tzsj; //��ѹ����1ֹͣʱ��
	 uint16 dy2yxx; //��ѹ����2��Ч����
	 uint16 dy2gzfs; //��ѹ����2������ʽ
	 uint16 dy2gzsj; //��ѹ����2���ʱ��
	 uint16 dy2tzsj; //��ѹ����2ֹͣʱ��
	 uint16 dy3yxx; //��ѹ����3��Ч����
	 uint16 dy3gzfs; //\��ѹ����3������ʽ
	 uint16 dy3gzsj; //; //��ѹ����3���ʱ��
	 uint16 dy3tzsj; //��ѹ����3ֹͣʱ��
	 uint16 dy4yxx; //��ѹ����4��Ч����
	 uint16 dy4gzfs; //��ѹ����4������ʽ
	 uint16 dy4gzsj; //��ѹ����4���ʱ��
	 uint16 dy4tzsj; //��ѹ����4ֹͣʱ��
	 uint16 dy5yxx; //��ѹ����5��Ч����
	 uint16 dy5gzfs; //��ѹ����5������ʽ
	 uint16 dy5gzsj; //��ѹ����6���ʱ��
	 uint16 dy5tzsj; //��ѹ����6ֹͣʱ��
	 uint16 reserve[10];
}TDySetPara;

typedef struct _TTimePara
{
	uint16 year; //��
	uint16 mon; //��
	uint16 day; //��
	uint16 hour; //ʱ
	uint16 min; //��
	uint16 sec; //��
	uint16 reserve[4];
}TTimeSetPara;

typedef struct _TGyOper
{
	 uint16 gyqt; //��ѹ��ͣ
	 uint16 reserve0;
	 uint16 gyfg; //��ѹ����
	 uint16 csqt; //cs��ͣ
	 uint16 reserve1[6];
}TGySetOper;

typedef struct _TDySetOper
{
	uint16 dy1cz; //��ѹ����1����
	uint16 dy2cz; //��ѹ����2����
	uint16 dy3cz; //��ѹ����3����
	uint16 dy4cz; //��ѹ����4����
	uint16 dy5cz; //��ѹ����5����
	uint16 reserve[5];
}TDySetOper;

typedef struct _TPowerSetPara
{
	TGySetPara tGySetPara;
	TDySetPara tDySetPara;
	TTimeSetPara tTimeSetPara;
	TGySetOper tGySetOper;
	TDySetOper tDySetOper;
}TPowerSetPara;

typedef struct _TGyRunBasePara
{
	uint16 REUd; //ֱ����ѹ
	uint16 REI1; //һ�ε���
	uint16 REU2;//���ε�ѹ
	uint16 REI2;//���ε���
	uint16 REtzd;//���ƶ� *10
	uint16 REsp;//��Ƶ
	uint16 REfdbg;//���ȱȸ�
	uint16 REfdbd; //���ȱȵ�
	uint16 REzkbz; //ռ�ձ�ռ
	uint16 REzkbk; //ռ�ձȿ�
	uint16 REgyzt;//��ѹ״̬
	uint16 REgdfs;//���緽ʽ
	uint16 reserve[10];
}TGyRunBasePara;
#define RUN_BASE_PARA {378, 123, 67, 5678, 67, 12, 2, 1, 5, 4, 1, 2}
typedef struct _TGyRunSuperPara
{
	uint16 REU2mcfz;//�����ѹ��ֵ
	uint16 REU2mcgz;//�����ѹ��ֵ
	uint16 REI2mcfz;//���������ֵ
	uint16 REI2mcjz;//���������ֵ
	uint16 REgl;//�������
	uint16 REUdxs;//ֱ����ѹϵ��
	uint16 REI1xs;//һ�ε���ϵ��
	uint16 REU2xs;//���ε�ѹϵ��
	uint16 REI2xs;//���ε���ϵ��
	uint16 REUdtdgz;//ֱ����ѹͨ������
	uint16 REI1tdgz;//I1ͨ������
	uint16 REU2tdgz;//U2ͨ������
	uint16 REI2tdgz;//I2ͨ������
	uint16 reserve0[7];//
	uint16 REdspbb;//DSP�汾 * 10
	uint16 REyw;//����
	uint16 REwdl;//IGBT�¶�L
	uint16 REwdr;//IGBT�¶�R
	uint16 REarmbb; //*100
	uint16 REzjgz;//�Լ����
	uint16 REgzdm;//��ѹ���ϴ���
	uint16 reserve1[3];
}TGyRunSuperPara;

typedef struct _TDyRunPara
{
	uint16 REdy1yx;//����״̬
	uint16 REdy1gzdm;//��ѹ����
	uint16 REdy1sysj;
	uint16 REdy2yx;//����״̬
	uint16 REdy2gzdm;//��ѹ����
	uint16 REdy2sysj;
	uint16 REdy3yx;//����״̬
	uint16 REdy3gzdm;//��ѹ����
	uint16 REdy3sysj;
	uint16 REdy4yx;//����״̬
	uint16 REdy4gzdm;//��ѹ����
	uint16 REdy4sysj;
	uint16 REdy5yx;//����״̬
	uint16 REdy5gzdm;//��ѹ����
	uint16 REdy5sysj;
}TDyRunPara;

typedef struct _TPowerRunPara
{
	TGyRunBasePara tGyRunBasePara;
	TGyRunSuperPara tGyRunSuperPara;
	TDyRunPara tDyRunPara;
}TPowerRunPara;
#pragma pack() // ȡ��1�ֽڶ��룬�ָ�ΪĬ�϶���

enum
{
	swtich_off,
	switch_on
};

enum
{
	DC_supply,
	pulse_supply,
	interval_supply
};

typedef struct _TChangeParaMsg
{
	uint16 usPos;
	uint16 u16Value;
}TChangeParaMsg;

#endif /* PARA_H_ */

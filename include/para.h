/*
 * para.h
 *
 *  Created on: 2018-7-15
 *      Author: Administrator
 */

#ifndef PARA_H_
#define PARA_H_

typedef unsigned short int uint16;
#pragma pack(1) // 让编译器做1字节对齐
typedef struct _TGySetPara
{
	 uint16 tzdsx; //调制度上限
	 uint16 gdfs; //供电方式
	 uint16 spsx; //闪频上限
	 uint16 fdbg; //幅度比高
	 uint16 fdbd; //幅度比低
	 uint16 zkbz; //占空比占
	 uint16 zkbk; //占空比空
	 uint16 reserve0[5];
	 uint16 U2qy; //U2欠压
	 uint16 I1sx; //I1上限
	 uint16 U2sx; //U2上限
	 uint16 I2sx; //I2上限
	 uint16 sfs; //闪封数
	 uint16 xhfs; //熄弧方式
	 uint16 jyzdkg; //降压开关
	 uint16 jyfd; //\降压幅度
	 uint16 I2mcfzsx; //脉冲电流峰值上限
	 uint16 I1fzsx; //I1峰值上限
	 uint16 kgpl; //开关频率
	 uint16 Udxs; //直流电压系数
	 uint16 I1xs; //一次电流系数
	 uint16 U2xs; //二次电压系数
	 uint16 I2xs; //二次电流系数
	 uint16 Udxsjz; //直流电压校正
	 uint16 I1xsjz; //一次电流校正
	 uint16 U2xsjz; //\二次电压校正
	 uint16 I2xsjz; //二次电流校正
	 uint16 reserve1[11];
	 uint16 Uded; //直流电压额定值
	 uint16 I1ed; //一次电流额定值
	 uint16 U2ed; //二次电压额定值
	 uint16 I2ed; //二次电流额定值
	 uint16 ljgw; //临界管温
	 uint16 wxgw; //危险管温
	 uint16 ljywsz; //临界油温设置
	 uint16 wxywsz; //\危险管温设置
	 uint16 reserve2[10];
}TGySetPara;

#define GY_SET_PARA { 99, 1, 2, 3, 4, 5, 7, 8, 9, 10,\
					   11, 12, 14, 15, 16, 17, 18, 19, 20}

typedef struct _TDySetPara
{
	 uint16 dy1yxx; //低压部件1有效设置
	 uint16 dy1gzfs; //低压部件1工作方式.
	 uint16 dy1gzsj; //低压部件1振打时间
	 uint16 dy1tzsj; //低压部件1停止时间
	 uint16 dy2yxx; //低压部件2有效设置
	 uint16 dy2gzfs; //低压部件2工作方式
	 uint16 dy2gzsj; //低压部件2振打时间
	 uint16 dy2tzsj; //低压部件2停止时间
	 uint16 dy3yxx; //低压部件3有效设置
	 uint16 dy3gzfs; //\低压部件3工作方式
	 uint16 dy3gzsj; //; //低压部件3振打时间
	 uint16 dy3tzsj; //低压部件3停止时间
	 uint16 dy4yxx; //低压部件4有效设置
	 uint16 dy4gzfs; //低压部件4工作方式
	 uint16 dy4gzsj; //低压部件4振打时间
	 uint16 dy4tzsj; //低压部件4停止时间
	 uint16 dy5yxx; //低压部件5有效设置
	 uint16 dy5gzfs; //低压部件5工作方式
	 uint16 dy5gzsj; //低压部件6振打时间
	 uint16 dy5tzsj; //低压部件6停止时间
	 uint16 reserve[10];
}TDySetPara;

typedef struct _TTimePara
{
	uint16 year; //年
	uint16 mon; //月
	uint16 day; //日
	uint16 hour; //时
	uint16 min; //分
	uint16 sec; //秒
	uint16 reserve[4];
}TTimeSetPara;

typedef struct _TGyOper
{
	 uint16 gyqt; //高压启停
	 uint16 reserve0;
	 uint16 gyfg; //高压复归
	 uint16 csqt; //cs启停
	 uint16 reserve1[6];
}TGySetOper;

typedef struct _TDySetOper
{
	uint16 dy1cz; //低压部件1操作
	uint16 dy2cz; //低压部件2操作
	uint16 dy3cz; //低压部件3操作
	uint16 dy4cz; //低压部件4操作
	uint16 dy5cz; //低压部件5操作
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
	uint16 REUd; //直流电压
	uint16 REI1; //一次电流
	uint16 REU2;//二次电压
	uint16 REI2;//二次电流
	uint16 REtzd;//调制度 *10
	uint16 REsp;//闪频
	uint16 REfdbg;//幅度比高
	uint16 REfdbd; //幅度比低
	uint16 REzkbz; //占空比占
	uint16 REzkbk; //占空比空
	uint16 REgyzt;//高压状态
	uint16 REgdfs;//供电方式
	uint16 reserve[10];
}TGyRunBasePara;
#define RUN_BASE_PARA {378, 123, 67, 5678, 67, 12, 2, 1, 5, 4, 1, 2}
typedef struct _TGyRunSuperPara
{
	uint16 REU2mcfz;//脉冲电压峰值
	uint16 REU2mcgz;//脉冲电压谷值
	uint16 REI2mcfz;//脉冲电流峰值
	uint16 REI2mcjz;//脉冲电流谷值
	uint16 REgl;//输出功率
	uint16 REUdxs;//直流电压系数
	uint16 REI1xs;//一次电流系数
	uint16 REU2xs;//二次电压系数
	uint16 REI2xs;//二次电流系数
	uint16 REUdtdgz;//直流电压通道故障
	uint16 REI1tdgz;//I1通道故障
	uint16 REU2tdgz;//U2通道故障
	uint16 REI2tdgz;//I2通道故障
	uint16 reserve0[7];//
	uint16 REdspbb;//DSP版本 * 10
	uint16 REyw;//油温
	uint16 REwdl;//IGBT温度L
	uint16 REwdr;//IGBT温度R
	uint16 REarmbb; //*100
	uint16 REzjgz;//自检故障
	uint16 REgzdm;//高压故障代码
	uint16 reserve1[3];
}TGyRunSuperPara;

typedef struct _TDyRunPara
{
	uint16 REdy1yx;//运行状态
	uint16 REdy1gzdm;//低压故障
	uint16 REdy1sysj;
	uint16 REdy2yx;//运行状态
	uint16 REdy2gzdm;//低压故障
	uint16 REdy2sysj;
	uint16 REdy3yx;//运行状态
	uint16 REdy3gzdm;//低压故障
	uint16 REdy3sysj;
	uint16 REdy4yx;//运行状态
	uint16 REdy4gzdm;//低压故障
	uint16 REdy4sysj;
	uint16 REdy5yx;//运行状态
	uint16 REdy5gzdm;//低压故障
	uint16 REdy5sysj;
}TDyRunPara;

typedef struct _TPowerRunPara
{
	TGyRunBasePara tGyRunBasePara;
	TGyRunSuperPara tGyRunSuperPara;
	TDyRunPara tDyRunPara;
}TPowerRunPara;
#pragma pack() // 取消1字节对齐，恢复为默认对齐

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

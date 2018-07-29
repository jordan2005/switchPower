/*
 * communicationPro.c
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "linuxType.h"
#include "para.h"
#include "communicationPro.h"
#include "queue.h"
#include "modbusTcpSrv.h"
#define POOL_COUNT 256
sem_t semtSetCmd;
pthread_mutex_t mutexSetPara;

static void writeSetCmd(TQueue* ptQ, fid* fdSend)
{
	uint16 usCnt;
	unsigned char i;
	usCnt = Queue_iCount(ptQ);
	TChangeParaMsg *pt = malloc(usCnt * sizeof(TChangeParaMsg));

	if (0 == usCnt)
		return;

	for (i=0; i<usCnt; i++)
	{
		TPowerSetPara* ptPara = (TPowerSetPara*)getRegHolding();
		(void)Queue_bFetch(ptQ, &pt[i].usPos);
		pt[i].u16Value = ((uint16*)ptPara)[pt[i].usPos];
		printf("\n pos: %d, value: %d\n", pt[i].usPos, pt[i].u16Value);
	}

	if (write(*((fid*)fdSend+1), pt, usCnt*sizeof(TChangeParaMsg)) < 0)
		printf("write pipe error.\n");

	free(pt);
}



TQueue tQueue;
static void* sendSetCmd(void* fdSend)
{
	unsigned char ucQueSize = sizeof(TPowerSetPara)/2;
	Queue_bInit(&tQueue, ucQueSize);

	while (1)
	{
		sem_wait(&semtSetCmd);
    	pthread_mutex_lock(&mutexSetPara);

		if(!Queue_bIsEmpty(&tQueue))
			writeSetCmd(&tQueue, fdSend);

		pthread_mutex_unlock(&mutexSetPara);
	}
	return NULL;
}



static void* readRunPara(void* fdRecv)
{
	while(1)
	{
		int n;
		char para[POOL_COUNT];
		n = read(*(fid*)fdRecv, para, POOL_COUNT);

		if (n == sizeof(TPowerRunPara))
		{
			memcpy((char*)getRegInput(), para, n);
		}
		else if (n == sizeof(TPowerSetPara))
		{
			memcpy((char*)getRegHolding(), para, n);
		}
		else
			printf("comm module receive data  error.\n");
	}

	return NULL;
}



int getIdAddrInHoldingReg(char* pIdName)//计算id name对应的寄存器地址
{
	const char s[REG_HOLDING_NREGS][10]= ID_NAME_BUF;
	unsigned char i = 0;

	for (i = 0; i < 120; ++i)
		if (strcmp(s[i], pIdName) == 0)
			return i;

	return -1;
}



void parseWebSetReq(char *pIdName,char *pIdValue)
{
	int addr = -1;

	addr = getIdAddrInHoldingReg(pIdName);
	if (addr >= 0)
	{
		pthread_mutex_lock(&mutexSetPara);

		getRegHolding()[addr] = atoi(pIdValue);
		Queue_bInsert(&tQueue, addr);

		pthread_mutex_unlock(&mutexSetPara);

		sem_post(&semtSetCmd);
	}
	printf("RegHolding[%d] = %d\n", addr, atoi(pIdValue));
}



void getRunParaStr(char* strBuf)
{
	int offset = 0;
	uint16* pRunPara = getRegInput();
	uint16* pSetPara = getRegHolding();
	int i = 0;

	for (i=0; i<65; ++i)
		offset += sprintf((char*)strBuf+offset,"%d,",pRunPara[i]);

	for (i=0; i<115; ++i)
		offset += sprintf((char*)strBuf+offset,"%d,",pSetPara[i]);
}



#define SERVER_PORT 4321
#define BUFF_LEN 1024
static void handleUdpMsg(int fd)
{
    char buf[BUFF_LEN] = {0};  //接收缓冲区，1024字节
    socklen_t len;
    int count;
    struct sockaddr_in clientAddr;  //clent_addr用于记录发送方的地址信息
	const char* strReq = "reqPara:";

    while(1)
    {
        memset(buf, 0, BUFF_LEN);
        len = sizeof(clientAddr);
        count = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clientAddr, &len);  //recvfrom是拥塞函数，没有数据就一直拥塞
        if(count == -1)
        {
            printf("recieve data fail!\n");
        }
        else if (strncmp(buf, strReq, strlen(strReq)) != 0) //request not be identified
        {
        	printf("request not be identified.\n");
        }
        else if (strlen(strReq) == strlen(buf)) //request refresh run data, only "reqPara:"
        {
        	memset(buf, 0, BUFF_LEN);
        	getRunParaStr(buf);
        	sendto(fd, buf, strlen(buf), 0, (struct sockaddr*)&clientAddr, len);
        }
        else //request set para,"reqPara:###=%ddd"
        {
        	char *pSetReq  = NULL;
            char *pIdName  = NULL;
        	char *pIdValue = NULL;
        	char *pEquPos  = NULL;

			pSetReq = buf + strlen(strReq);
			pEquPos = strchr(pSetReq, '=');
			if (NULL != pEquPos) //has a '=' in request string
			{
				*pEquPos = '\0';
				pIdName = pSetReq;
				pIdValue = pEquPos + 1;
				parseWebSetReq(pIdName, pIdValue);
			}
        }
    }
}



static void* webServer(void* arg)
{
	int srvFd, ret;
	struct sockaddr_in srvAddr;

	srvFd = socket(AF_INET, SOCK_DGRAM, 0); //AF_UNIX:IPV4;SOCK_DGRAM:UDP
	if(srvFd < 0)
	{
		printf("create socket fail!\n");
		return -1;
	}

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
	srvAddr.sin_port = htons(SERVER_PORT);  //端口号，需要网络序转换

	ret = bind(srvFd, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
	if(ret < 0)
	{
		printf("socket bind fail!\n");
		return -1;
	}

	handleUdpMsg(srvFd);   //处理接收到的数据

	close(srvFd);

	return NULL;
}


int runCommPro(fid* pfdRecv,fid* pfdSend)
{
	pthread_t tid;

	close(pfdSend[0]);
	close(pfdRecv[1]);

	pthread_create(&tid, NULL, sendSetCmd, pfdSend);
	pthread_create(&tid, NULL, readRunPara, pfdRecv);
	pthread_create(&tid, NULL, modbusServer, NULL);
	pthread_create(&tid, NULL, webServer, NULL);
	pthread_join(tid, NULL);

	sem_destroy(&semtSetCmd);
	pthread_mutex_destroy(&mutexSetPara);
	return 0;
}



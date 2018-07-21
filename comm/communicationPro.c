/*
 * communicationPro.c
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "linuxType.h"
#include "para.h"
#include "communicationPro.h"
#include <string.h>
#include "queue.h"
#include <stdlib.h>
#define POOL_COUNT 256

static TPowerRunPara tPowerRunPara =
		{
			.tGyRunBasePara = {0},
			.tGyRunSuperPara = {0},
			.tDyRunPara = {0}
		};

static TPowerSetPara tPowerSetPara =
		{
			.tGySetPara   = GY_SET_PARA,
			.tDySetPara   = {0},
			.tTimeSetPara = {0},
			.tGySetOper   = {0},
			.tDySetOper   = {0}
		};

//void* sendSetCmd(void* fdSend)
//{
//	while(1)
//	{
//		if (write(*((fid*)fdSend+1), "hello parent\n", 13) < 0)
//			printf("write pipe error.\n");
//		sleep(13);
//	}
//
//	return NULL;
//}

void checkParaChange()
{
	;
}

void writeSetCmd(TQueue* ptQ, fid* fdSend)
{
	uint16 usCnt;
	unsigned char i;
	usCnt = Queue_iCount(ptQ);
	TChangeParaMsg *pt = malloc(usCnt * sizeof(TChangeParaMsg));

	if (0 == usCnt)
		return;

	for (i=0; i<usCnt; i++)
	{
		(void)Queue_bFetch(ptQ, &pt[i].usPos);
		pt[i].u16Value = ((uint16*)&tPowerSetPara)[pt[i].usPos];
	}

	if (write(*((fid*)fdSend+1), pt, usCnt*sizeof(TChangeParaMsg)) < 0)
		printf("write pipe error.\n");
	free(pt);
}

void* sendSetCmd(void* fdSend)
{
	TQueue tQueue;
	//modbusComm()  To do.

	unsigned char ucQueSize = sizeof(TPowerSetPara)/2;
	while (1)
	{
		Queue_bInit(&tQueue, ucQueSize);
		Queue_bInsert(&tQueue, 4);
		Queue_bInsert(&tQueue, 1);
		Queue_bInsert(&tQueue, 0);
		Queue_bInsert(&tQueue, 2);

		checkParaChange(&tQueue);
		if(!Queue_bIsEmpty(&tQueue))
			writeSetCmd(&tQueue, fdSend);

		sleep(2);
	}

	return NULL;
}



void paraPrintf(uint16 *pPara, int length)
{
	int i;
	for (i=0; i<length; i++)
		printf("%d  ",pPara[i]);
	printf("\n");
}



void* readRunPara(void* fdRecv)
{
	while(1)
	{
		int n;
		char para[POOL_COUNT];
		n = read(*(fid*)fdRecv, para, POOL_COUNT);

		if (n == sizeof(tPowerRunPara))
		{
			memcpy(&tPowerRunPara, para, n);
			printf("receive run para.\n");
		}
		else if (n == sizeof(tPowerSetPara))
		{
			memcpy(&tPowerSetPara, para, n);
			printf("receive set para.\n");
		}
		else
			printf("comm module receive data  error.\n");

		paraPrintf((uint16*)para, n/2);
	}

	return NULL;
}

int runCommPro(fid* pfdRecv,fid* pfdSend)
{
	pthread_t tid;

	close(pfdSend[0]);
	close(pfdRecv[1]);

	pthread_create(&tid, NULL, sendSetCmd, pfdSend);
	//pthread_create(&tid, NULL, readRunPara, pfdRecv);

	while(1)
	{
		sleep(1);
	}

	return 0;
}



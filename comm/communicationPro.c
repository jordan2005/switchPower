/*
 * communicationPro.c
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include<semaphore.h>
#include "linuxType.h"
#include "para.h"
#include "communicationPro.h"
#include <string.h>
#include "queue.h"
#include <stdlib.h>
#include "modbusTcpSrv.h"
#define POOL_COUNT 256
sem_t semtSetCmd;
pthread_mutex_t mutexSetPara;

void checkParaChange(TQueue* ptQ)
{
	uint16* pBak = (uint16*)getSetParaBak();
	uint16* pHold = getRegHolding();
	int i = 0;
	for (i=0; i<sizeof(TPowerSetPara)/2; i++)
	{
		if ((pBak[i]!=pHold[i]) && !Queue_bIsFull(ptQ))
		{
			Queue_bInsert(ptQ, i);
		}
	}

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
		TPowerSetPara* ptPara = (TPowerSetPara*)getRegHolding();
		(void)Queue_bFetch(ptQ, &pt[i].usPos);
		pt[i].u16Value = ((uint16*)ptPara)[pt[i].usPos];
		printf("\n pos: %d, value: %d\n", pt[i].usPos, pt[i].u16Value);
	}

	if (write(*((fid*)fdSend+1), pt, usCnt*sizeof(TChangeParaMsg)) < 0)
		printf("write pipe error.\n");

	free(pt);
}

void* sendSetCmd(void* fdSend)
{
	TQueue tQueue;

	unsigned char ucQueSize = sizeof(TPowerSetPara)/2;
	Queue_bInit(&tQueue, ucQueSize);

	while (1)
	{
		sem_wait(&semtSetCmd);
    	pthread_mutex_lock(&mutexSetPara);
		checkParaChange(&tQueue);
		pthread_mutex_unlock(&mutexSetPara);
		if(!Queue_bIsEmpty(&tQueue))
			writeSetCmd(&tQueue, fdSend);
	}
	return NULL;
}


void* readRunPara(void* fdRecv)
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

int runCommPro(fid* pfdRecv,fid* pfdSend)
{
	pthread_t tid;

	close(pfdSend[0]);
	close(pfdRecv[1]);

	pthread_create(&tid, NULL, sendSetCmd, pfdSend);
	pthread_create(&tid, NULL, readRunPara, pfdRecv);
	pthread_create(&tid, NULL, modbusServer, NULL);
	pthread_join(tid, NULL);
//	while(1)
//	{
//		sleep(1);
//	}

	sem_destroy(&semtSetCmd);
	pthread_mutex_destroy(&mutexSetPara);
	return 0;
}



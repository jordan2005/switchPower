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
#define POOL_COUNT 256

static TPowerRunPara tPowerRunPara =
		{
			.tGyRunBasePara = {0},
			.tGyRunSuperPara = {0},
			.tDyRunPara = {0}
		};

static TPowerSetPara tPowerSetPara =
		{
			.tGySetPara   = {0},
			.tDySetPara   = {0},
			.tTimeSetPara = {0},
			.tGySetOper   = {0},
			.tDySetOper   = {0}
		}


void* sendSetCmd(void* fdSend)
{
	while(1)
	{
		if (write(*((fid*)fdSend+1), "hello parent\n", 13) < 0)
			printf("write pipe error.\n");
		sleep(13);
	}

	return NULL;
}

//void* readRunPara(void* fdRecv)
//{
//	while(1)
//	{
//		int n;
//		int i;
//		uint16* pPara;
//		n = read(*(fid*)fdRecv, (char*)&tPowerRunPara, sizeof(TPowerRunPara));
//
//		pPara = &tPowerRunPara;
//		for (i=0; i<n/2; i++)
//			printf("%d  ",pPara[i]);
//		printf("\n");
//	}
//
//	return NULL;
//}
void paraPrintf(uint16 *pPara, int length)
{
	int i;
	pPara = &tPowerRunPara;
	for (i=0; i<length; i++)
		printf("%d  ",pPara[i]);
	printf("\n");
}

void* readRunPara(void* fdRecv)
{
	while(1)
	{
		int n;
		int i;
		char para[POOL_COUNT];
		uint16* pPara;
		n = read(*(fid*)fdRecv, para, POOL_COUNT);

		if (n == sizeof(tPowerRunPara))
			memcpy(&tPowerRunPara, para, n);
		else if (n == sizeof(tPowerSetPara))
			memcpy(&tPowerSetPara, para, n);
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
	pthread_create(&tid, NULL, readRunPara, pfdRecv);

	while(1)
	{
		sleep(1);
	}

	return 0;
}



/*
 * controlPro.c
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "linuxType.h"
#include "para.h"
#include "controlPro.h"

static volatile TPowerRunPara tPowerRunPara =
		{
			.tGyRunBasePara  = RUN_BASE_PARA,
			.tGyRunSuperPara = {8,7,6,5},
			.tDyRunPara      = {5,6,7,8}
		};

static volatile TPowerSetPara tPowerSetPara =
		{
			.tGySetPara   = GY_SET_PARA,
			.tDySetPara   = {1},
			.tTimeSetPara = {2},
			.tGySetOper   = {3},
			.tDySetOper   = {4}
		};

#define MAXLINE 1024

void simRunPara()
{
	uint16 a[] = RUN_BASE_PARA;
	uint16* pRunPara = (uint16*)&tPowerRunPara.tGyRunBasePara;
	int len = sizeof(tPowerRunPara.tGyRunBasePara)/2;
	int i = 0;
	srand((unsigned)time(NULL));
	for( i = 0; i < len;i++ )
	{
		pRunPara[i] = a[i] + rand() % 10;
	}
}

void* sendRunPara(void* fdSend)
{
	while(1)
	{
		simRunPara();
		if (write(*((fid*)fdSend+1), (unsigned char*)&tPowerRunPara, sizeof(tPowerRunPara)) < 0)
			printf("write pipe error.\n");
		sleep(1);

		if (write(*((fid*)fdSend+1), (unsigned char*)&tPowerSetPara, sizeof(tPowerSetPara)) < 0)
			printf("write pipe error.\n");
		sleep(1);
	}

	return NULL;
}



void* readSetCmd(void* fdRecv)
{
	while(1)
	{
		int n;
		char para[MAXLINE];
		n = read(*(fid*)fdRecv, para, MAXLINE);
		printf("\n receieve para set change:\n");
		//display received para;
		{
			int i;
			TChangeParaMsg* ptRecv = (TChangeParaMsg*) para;
			uint16* ptSetPara =  (uint16*)&tPowerSetPara;
			for (i=0; i<n/sizeof(TChangeParaMsg); i++)
			{
				printf("changePos:%d changeValue:%d .\n", ptRecv[i].usPos, ptRecv[i].u16Value);
				ptSetPara[ptRecv[i].usPos] = ptRecv[i].u16Value;
			}
		}
	}

	return NULL;
}

int runCtrlPro(fid* pfdRecv,fid* pfdSend)
{
	pthread_t tid;

	close(pfdSend[0]);
	close(pfdRecv[1]);

	pthread_create(&tid, NULL, sendRunPara, pfdSend);
	pthread_create(&tid, NULL, readSetCmd, pfdRecv);

	while(1)
	{
		sleep(1);
	}

	return 0;
}

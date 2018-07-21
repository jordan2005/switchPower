/*
 * queue.c
 *
 *  Created on: 2018-7-16
 *      Author: Administrator
 */

#include<stdio.h>
#include<stdlib.h>
#include "queue.h"

BOOL Queue_bInit(TQueue *ptQ, uchar ucSize)
{
        ptQ->base = (TElemType *) malloc(ucSize * sizeof(TElemType));
        if(NULL == ptQ->base)
        {
        	return FALSE;
        }
        ptQ->front = 0;
        ptQ->rear = 0;
        ptQ->size = ucSize;
        return TRUE;
}



void Queue_vDestroy(TQueue *ptQ)
{
    free(ptQ->base);
}



BOOL Queue_bIsFull(TQueue *ptQ)
{
    return (ptQ->front == ((ptQ->rear + 1) % ptQ->size));
}



BOOL Queue_bIsEmpty(TQueue *ptQ)
{
    return (ptQ->front == ptQ->rear);
}


//add element
BOOL Queue_bInsert(TQueue *ptQ, TElemType tE)
{
    if (Queue_bIsFull(ptQ))
    {
        return FALSE; //full
    }
    else
    {
		ptQ->base[ptQ->rear] = tE;  //insert
		ptQ->rear = (ptQ->rear + 1) % ptQ->size;
		return TRUE;
    }
}


//delete element
BOOL Queue_bFetch(TQueue *ptQ, TElemType *ptE)
{
    if (Queue_bIsEmpty(ptQ))
        return FALSE; //empty
    else
    {
		*ptE = ptQ->base[ptQ->front];
		ptQ->front = (ptQ->front + 1) % ptQ->size;
		return TRUE;
    }
}


uint16 Queue_iCount(TQueue *ptQ)
{
	 return (ptQ->rear-ptQ->front+ptQ->size)%ptQ->size;
}

/*
 * queue.h
 *
 *  Created on: 2018-7-16
 *      Author: Administrator
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "para.h"
typedef unsigned char uchar;

#ifndef TYPEDEF_BOOL
#define TYPEDEF_BOOL
typedef unsigned char BOOL;
#endif

#define TRUE  (1)
#define FALSE (0)
#define QUEUE_SIZE (20)

typedef uint16 TElemType;
typedef struct _TQueue
{
        TElemType *base;  //queue ram point
        uchar front;
        uchar rear;
        uchar size;
}TQueue;

BOOL Queue_bInit(TQueue *ptQ, uchar ucSize);

BOOL Queue_bIsFull(TQueue *ptQ);

BOOL Queue_bIsEmpty(TQueue *ptQ);
//add element
BOOL Queue_bInsert(TQueue *ptQ, TElemType te);

//delete element
BOOL Queue_bFetch(TQueue *ptQ, TElemType *pte);
uint16 Queue_iCount(TQueue *ptQ);
#endif /* QUEUE_H_ */

/*
 * debugPrint.h
 *
 *  Created on: 2018-7-29
 *      Author: Administrator
 */

#ifndef DEBUGPRINT_H_
#define DEBUGPRINT_H_

#include <stdio.h>
#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG_PRINT(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(format,...)
#endif


#endif /* DEBUGPRINT_H_ */

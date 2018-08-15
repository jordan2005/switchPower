/*
 * database.c
 *
 *  Created on: 2018-8-13
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sqlite3.h"
#include "para.h"
#include "debugPrint.h"
#define SQL_STRING_SIZE (256)


static void getCurStrTime(char* buf)
{
	time_t t_time;
	struct tm* tm_ptr = NULL;
	time(&t_time);
	tm_ptr = localtime(&t_time);
	strftime(buf, 64, "%Y-%m-%d %H:%M:%S", tm_ptr);
}



static void getStrSqlCmd(char* sqlCmd, TPowerRunPara* ptPowerRunPara)
{
	char strTime[64] = "\0";
	int workMode = ptPowerRunPara->tGyRunBasePara.REgdfs;
	int u1 = ptPowerRunPara->tGyRunBasePara.REUd;
	float u2 = ptPowerRunPara->tGyRunBasePara.REU2/10;
	float i1 = ptPowerRunPara->tGyRunBasePara.REI1/10;
	int i2 = ptPowerRunPara->tGyRunBasePara.REI2;


	getCurStrTime(strTime);
	snprintf(sqlCmd, SQL_STRING_SIZE-1, "INSERT INTO PowerData VALUES(NULL, '%s', %d, %d, %.2f, %.2f, %d);",
			strTime, workMode, u1, u2, i1, i2);
	DEBUG_PRINT("%s\n", sqlCmd);
}



void DB_saveWorkModeRecord(TPowerRunPara* ptPowerRunPara)
{
	sqlite3* db=NULL;
	char* strErrMsg = 0;
	int ret = 0;
	ret = sqlite3_open("test.db", &db);//open the specific database,if not exist, create it.
	if (ret)
	{
		DEBUG_PRINT("Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else
	{
		DEBUG_PRINT("Open database sucessfully.\n");
	}

	//open the specific table,if not exist, create it.
	char *sql = "CREATE TABLE PowerData(\
	ID INTEGER PRIMARY KEY,\
	Time VARCHAR(20),\
	WorkMode INTEGER,\
	U1 INTEGER,\
	U2 REAL,\
	I1 REAL,\
	I2 INTEGER\
	);" ;
	sqlite3_exec(db, sql, 0, 0, &strErrMsg);

	//insert a piece of record
	char sqlCmd[SQL_STRING_SIZE] = "\0";
	getStrSqlCmd(sqlCmd, ptPowerRunPara);
	sqlite3_exec(db, sqlCmd, 0, 0, &strErrMsg);

	sqlite3_close(db);
}

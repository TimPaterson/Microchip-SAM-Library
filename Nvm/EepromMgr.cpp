//****************************************************************************
// Class EepromMgr
// EepromMgr.cpp
//
// Created 2/19/2019 4:45:39 PM by Tim
//
//****************************************************************************


#include <standard.h>
#include "EepromMgr.h"


ushort CalcChecksumRow(RowDesc *pRow, int cb)
{
	if (cb > RowDataSize)
		cb = RowDataSize;
	return CalcChecksum(pRow + 1, cb);
}

ushort CalcChecksum(void *pRow, int cb)
{
	int		i;
	int		sum;
	uint	*puData;
	
	sum = ChecksumBase;
	puData = (uint *)pRow;
	for (i = 0; i < (int)(cb / sizeof *puData); i++)
	{
		sum += *puData++;
	}
	sum ^= sum >> 16;
	return sum;
}

void memcpy32(void *pvDest, void *pvSrc, int cb)
{
	uint	*puDest;
	uint	*puSrc;
	
	puDest = (uint *)pvDest;
	puSrc = (uint *)pvSrc;
	for (cb /= sizeof(uint); cb > 0; cb--)
		*puDest++ = *puSrc++;
}

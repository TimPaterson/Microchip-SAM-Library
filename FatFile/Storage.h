#pragma once

// Mass storage errors
#define NEG_ENUM(sym)	sym##_plus2, sym = sym##_plus2 - 2

enum STORAGE_ERROR
{
	STERR_None,
	NEG_ENUM(STERR_Busy),			// -1
	NEG_ENUM(STERR_NoMedia),		// -2
	NEG_ENUM(STERR_NotMounted),		// -3
	NEG_ENUM(STERR_BadBlock),		// -4
	NEG_ENUM(STERR_WriteProtect),	// -5
	NEG_ENUM(STERR_BadCmd),			// -6
	NEG_ENUM(STERR_DevFail),		// -7
	NEG_ENUM(STERR_InvalidAddr),	// -8
	NEG_ENUM(STERR_TimeOut),		// -9
	NEG_ENUM(STERR_Last),			// -10
};

class Storage
{
public:
	virtual int InitDev();
	virtual int MountDev();
	virtual int StartRead(ulong block);
	virtual int ReadData(void *pv);
	virtual int StartWrite(ulong block);
	virtual int WriteData(void *pv);
	virtual int GetStatus();
	virtual int DismountDev();

public:
	static bool IsError(int err)	{ return err < 0; }
};

// Enumeration returned by MountDev
#define STOR_PartitionUnk	0
#define	STOR_PartitionYes	1
#define STOR_PartitionNo	2


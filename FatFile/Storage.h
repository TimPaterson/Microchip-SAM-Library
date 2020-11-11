#pragma once

// Mass storage errors
#define NEG_ENUM(sym)	sym##_plus2, sym = sym##_plus2 - 2

enum STORAGE_ERROR
{
	STERR_None,
	NEG_ENUM(STERR_Busy),
	NEG_ENUM(STERR_NoMedia),
	NEG_ENUM(STERR_NotMounted),
	NEG_ENUM(STERR_BadBlock),
	NEG_ENUM(STERR_WriteProtect),
	NEG_ENUM(STERR_BadCmd),
	NEG_ENUM(STERR_DevFail),
	NEG_ENUM(STERR_InvalidAddr),
	NEG_ENUM(STERR_TimeOut),
	NEG_ENUM(STERR_Last),
};

class Storage
{
public:
	virtual int Init();
	virtual int MountDev();
	virtual int StartRead(ulong block);
	virtual int ReadData(void *pv);
	virtual int StartWrite(ulong block);
	virtual int WriteData(void *pv);
	virtual int GetStatus();
	virtual int Eject();

public:
	static bool IsError(int err)	{ return err < 0; }
};

// Enumeration returned by MountDev
#define STOR_PartitionUnk	0
#define	STOR_PartitionYes	1
#define STOR_PartitionNo	2


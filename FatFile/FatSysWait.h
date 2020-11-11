//****************************************************************************
// FatSysWait.h
//
// Created 11/11/2020 1:37:06 PM by Tim
//
//****************************************************************************

#pragma once

#include <FatFile\FatSys.h>


class FatSysWait : public FatSys
{
	static int WaitResult(uint handle)
	{
		int	status;

		do
		{
			status = GetStatus(handle);
		} while (status == FATERR_Busy);

		return status;
	}

	//****************************************************************************

	static int OpenWait(const char *pchName, uint hFolder = 0, uint flags = OPENFLAG_File | OPENFLAG_Folder, int cchName = 0)
	{
		int		err;

		err = Open(pchName, hFolder, flags, cchName);
		if (IsError(err))
			return err;

		hFolder = err;
		err = WaitResult(hFolder);
		if (IsError(err))
		{
			Close(hFolder);
			return err;
		}
		return hFolder;
	}

	//****************************************************************************

	static int CloseWait(uint handle)
	{
		int		err;

		err = Close(handle);
		if (err == FATERR_Busy)
			err = WaitResult(handle);
		return err;
	}

	//****************************************************************************

	static int ReadWait(uint handle, void *pv, int cb)
	{
		int		err;

		err = Read(handle, pv, cb);
		if (IsError(err))
			return WaitResult(err);	// just convert error code to WORD

		return WaitResult(handle);
	}

	//****************************************************************************

	static int WriteWait(uint handle, void *pv, int cb)
	{
		int		err;

		err = Write(handle, pv, cb);
		if (IsError(err))
			return WaitResult(err);	// just convert error code to WORD

		return WaitResult(handle);
	}

	//*********************************************************************

	static int DeleteWait(const char *pchName, uint hFolder = 0, uint flags = OPENFLAG_File | OPENFLAG_Folder, int cchName = 0)
	{
		uint	hFile;
		int		err;

		err = Open(pchName, hFolder, flags | OPENFLAG_Delete, cchName);
		if (IsError(err))
			return err;

		hFile = err;
		err = Delete(hFile);
		if (!(IsError(err)))
			err = WaitResult(hFile);

		return err;
	}

	//*********************************************************************

	static int EnumNextWait(uint handle, char *pch, int cbMax)
	{
		uint	hFile;
		int		err;

		err = EnumNext(handle, pch, cbMax);
		if (IsError(err))
			return err;

		hFile = err;
		err = WaitResult(hFile);
		if (IsError(err))
		{
			Close(hFile);
			return err;
		}
		return hFile;
	}

	//*********************************************************************

	static int GetDateWait(uint handle)
	{
		int		err;

		err = StartGetDate(handle);
		if (IsError(err))
			return err;

		return WaitResult(handle);
	}

	//*********************************************************************

	static int RenameWait(const char *pchName, uint hFolder, uint hFileSrc, int cchName = 0)
	{
		int		err;

		err = Rename(pchName, hFolder, hFileSrc, cchName);
		if (!(IsError(err)))
			err = WaitResult(hFileSrc);

		return err;
	}

	//*********************************************************************

	static ulong SeekWait(uint handle, ulong ulPos, int origin = FAT_SEEK_SET)
	{
		ulong	ulCurPos;
		int		err;

		ulCurPos = Seek(handle, ulPos, origin);
		if (ulCurPos != (ulong)-1)
		{
			err = WaitResult(handle);
			if (IsError(err))
				return -1;
		}
		return ulCurPos;
	}
};

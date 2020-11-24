//****************************************************************************
// FatSys.h
//
// Created 11/11/2020 11:08:07 AM by Tim
//
//****************************************************************************

#pragma once

#include <FatFile\FatDrive.h>


#define FAT_DRIVES_LIST(...) FatDrive *FatSys::m_arDrives[FAT_NUM_DRIVES] = {__VA_ARGS__};

class FatSys
{
	//*********************************************************************
	// Public interface
	//*********************************************************************

public:
	static bool IsError(int err)		{return FatDrive::IsError(err);}
	static byte IsFolder(uint handle)	{return HandleToPointer(handle)->IsFolder();}
	static FatDateTime GetFatDate(uint handle)	
		{return DriveToPointer(HandleToPointer(handle)->GetDrive())->m_state.DateTime;}
	static byte *GetDataBuf()			{ return FatDrive::s_pvDataBuf; }

	//static byte SetDate(byte handle, ulong dwTime) {return SetFatDate(handle, ToFatTime(dwTime));}
	/*
	static byte LockBuffer() {return LockBuffer();}
	static void WriteLockedBuffer(byte idBuf) {WriteLockedBuffer(idBuf);}
	static void WriteCurrentBuffer() {WriteCurrentBuffer();}
	static void UnlockBufferMakeCurrent(byte idBuf) {UnlockBufferMakeCurrent(idBuf);}
	static void UnlockBuffer(byte idBuf) {UnlockBuffer(idBuf);}
	*/

public:
	static int GetDriveStatus(uint bDrive)
	{
		FatDrive	*pDrive;

		// Perform pending operation for each drive
		for (byte drive = 0; drive < FAT_NUM_DRIVES; drive++)
		{
			pDrive = DriveToPointer(drive);
			pDrive->PerformOp();
		}

		pDrive = DriveToPointer(bDrive);
		return pDrive->Status();
	}

	//****************************************************************************

	static int GetStatus(uint handle)
	{
		uint	drive;

		if (handle == 0)
			drive = 0;
		else
			drive = HandleToPointer(handle)->GetDrive();

		return GetDriveStatus(drive);
	}

	//****************************************************************************

	static int Init()
	{
		int err;

		FatBuffer::InvalidateAll();
		FatDrive::InvalidateHandles();

		for (int i = 0; i < FAT_NUM_DRIVES; i++)
		{
			err = m_arDrives[i]->Init(i);
			if (err != FATERR_None)
				return err;
		}
		return err;
	}

	//****************************************************************************

	static int Mount(uint drive)
	{
		FatDrive	*pDrive;

		pDrive = DriveToPointer(drive);
		return pDrive->Mount();
	}

	//****************************************************************************

	static void Dismount(byte drive)
	{
		DriveToPointer(drive)->Dismount();
	}

	//****************************************************************************

	static int Open(const char *pchName, uint hFolder = 0, uint flags = OPENFLAG_File | OPENFLAG_Folder, int cchName = 0) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		uint		drive;
		int			h;
		int			err;

		h = GetHandle(hFolder, flags);
		pf = HandleToPointer(h);

		if (IsError(h))
			return h;

		if (cchName == 0)
			cchName = strlen(pchName);

		// Check for drive specifier
		if (FAT_NUM_DRIVES > 1)
		{
			if (cchName >= 2 && *(pchName + 1) == ':')
			{
				drive = *pchName - 'A';
				if (drive > 'Z' - 'A')
					drive -= 'a' - 'A';
				if (drive >= FAT_NUM_DRIVES)
				{
					err = FATERR_InvalidDrive;
					goto CloseErr;
				}

				cchName -= 2;
				pchName += 2;
				pDrive = DriveToPointer(drive);
				pDrive->InitRootSearch(pf);	// ignore passed-in root
			}
			else
				pDrive = DriveToPointer(pf->GetDrive());
		}
		else
			pDrive = DriveToPointer(0);

		if (pDrive->m_state.op != FATOP_None)
		{
			err = FATERR_Busy;
			goto CloseErr;
		}

		if (!pDrive->IsMounted())
		{
			err = STERR_NotMounted;
			goto CloseErr;
		}

		pDrive->m_state.cchName = cchName;
		pDrive->m_state.pchName = (char *)pchName;
		pDrive->m_state.handle = h;
		pDrive->m_state.info.OpenFlags = flags;

		pDrive->m_state.cchFolderName = pDrive->ParseFolder();
		if (pDrive->m_state.cchFolderName == 0)
		{
			if ((flags & OPENFLAG_CreateBits) == OPENFLAG_CreateNew)
			{
				err = FATERR_InvalidFileName;
				goto CloseErr;
			}

			// If no file name, return dup of hFolder
			pDrive->m_state.status = FATERR_None;
			pDrive->m_state.op = FATOP_Status;
			return h;
		}

		pDrive->m_state.op = FATOP_Open;
		err = pDrive->StartOpen(pf);
		if (err != FATERR_Busy)
		{
			pDrive->m_state.op = FATOP_None;
	CloseErr:
			pf->Close();
			return err;
		}

		return h;
	}

	//****************************************************************************

	static int Close(uint handle) NO_INLINE_ATTR
	{
		int	err;

		if (handle != 0)
		{
			if (handle > FAT_MAX_HANDLES)
				return FATERR_InvalidHandle;

			err = Flush(handle);
			if (IsError(err))
				return err;

			HandleToPointer(handle)->Close();
		}

		return FATERR_None;
	}

	//****************************************************************************

	static int Flush(uint handle) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;

		if (handle != 0)
		{
			pf = HandleToPointer(handle);
			if (pf->IsDirty())
			{
				pDrive = DriveToPointer(pf->GetDrive());
				if (pDrive->m_state.op != FATOP_None)
					return FATERR_Busy;

				pDrive->m_state.handle = handle;
				pDrive->m_state.status = 0;
				pDrive->m_state.op = FATOP_Close;
				return pDrive->StartBuf(pf->m_DirSec);
			}

			FatBuffer::ClearPriority(handle);
		}

		return FATERR_None;
	}

	//****************************************************************************

	static int FlushAll(uint drive) NO_INLINE_ATTR
	{
		FatDrive	*pDrive;

		pDrive = DriveToPointer(drive);
		if (pDrive->m_state.op != FATOP_None)
			return FATERR_Busy;

		pDrive->m_state.status = 0;
		pDrive->m_state.op = FATOP_FlushAll;
		return FATERR_Busy;
	}

	//****************************************************************************

	static int StartEnum(uint handle, uint flags = OPENFLAG_File | OPENFLAG_Folder) NO_INLINE_ATTR
	{
		int			h;
		FatFile		*pf;

		// Start new enumeration
		h = GetHandle(handle, flags);
		if (IsError(h))
			return h;
		pf = HandleToPointer(h);
		pf->m_flags.fNextClus = 1;
		pf->m_flags.OpenType = OPENTYPE_Enum;
		pf->m_OpenFlags = flags;
		return h;
	}

	//****************************************************************************

	static int EnumNext(uint hParent, char *pchNameBuf, int cbBuf) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatFile		*pfParent;
		FatDrive	*pDrive;
		int			h;
		int			err;

		if (hParent == 0)
			return FATERR_InvalidHandle;

		pfParent = HandleToPointer(hParent);
		if (pfParent->m_flags.OpenType != OPENTYPE_Enum)
			return FATERR_InvalidHandle;

		if (cbBuf < FAT_MIN_NAME_BUF)
			return FATERR_InvalidArgument;

		// Get the handle to return if found
		h = GetHandle(hParent, 0);
		if (IsError(h))
			return h;
		pf = HandleToPointer(h);

		pDrive = DriveToPointer(pf->GetDrive());
		if (pDrive->m_state.op != FATOP_None)
		{
			pf->Close();
			return FATERR_Busy;
		}

		// Initialize search location
		pfParent->CopySearchLoc(pf);

		pDrive->m_state.cchName = cbBuf - 1;// allow room for null terminator
		pDrive->m_state.pchName = pchNameBuf;
		pDrive->m_state.handle = h;
		pDrive->m_state.info.hParent = hParent;	// remember parent
		pDrive->m_state.info.OpenFlags = pfParent->m_OpenFlags;
		pDrive->m_state.op = FATOP_Enum;

		if (pfParent->m_flags.fNextClus)
		{
			// Initialize to read first entry
			pfParent->m_flags.fNextClus = 0;
			err = pDrive->ReadFirstDir(pf);
			if (err != FATERR_Busy)
			{
				pf->Close();
				return err;
			}
		}
		else
			pf->SearchNext();

		return h;
	}

	//****************************************************************************

	static int Delete(uint handle) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		int			err;

		pf = HandleToPointer(handle);
		if (pf->m_flags.OpenType != OPENTYPE_Delete)
			return FATERR_InvalidHandle;

		pDrive = DriveToPointer(pf->GetDrive());
		if (pDrive->m_state.op != FATOP_None)
			return FATERR_Busy;

		err = pDrive->StartBuf(pf->m_DirSec);
		if (err != FATERR_Busy)
			return err;
		pDrive->m_state.op = FATOP_Delete;
		return FATERR_None;
	}

	//****************************************************************************

	static int Rename(const char *pchName, uint hFolder, uint hSrc, int cchName = 0) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		int			err;

		pf = HandleToPointer(hSrc);
		if (pf->m_flags.OpenType != OPENTYPE_Delete)
			return FATERR_InvalidHandle;

		pDrive = DriveToPointer(pf->GetDrive());
		if (pDrive->m_state.op != FATOP_None)
			return FATERR_Busy;

		pDrive->m_state.info.hParent = hSrc;
		err = Open(pchName, hFolder, OPENFLAG_CreateNew, cchName);
		if (IsError(err))
			return err;

		pDrive->m_state.op = FATOP_RenameOpen;
		return FATERR_None;
	}

	//****************************************************************************

	static int Read(uint handle, void *pv, uint cb)
	{
		return ReadWrite(handle, pv, cb, FATOP_Read);
	}

	//****************************************************************************

	static int Write(uint handle, void *pv, uint cb)
	{
		return ReadWrite(handle, pv, cb, FATOP_Write);
	}

	//****************************************************************************

	static ulong Seek(uint handle, ulong ulPos, int origin = FAT_SEEK_SET) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		ulong		ulTmp;

		pf = HandleToPointer(handle);
		if (!pf->IsOpen() || pf->IsFolder())
			return -1;

		// Make sure we don't read past EOF
		if (origin != FAT_SEEK_SET)
		{
			if (origin == FAT_SEEK_END)
				ulTmp = pf->Length();
			else if (origin == FAT_SEEK_CUR)
				ulTmp = pf->CurPos();
			else
				return -1;

			// Check for seek before start of file
			if ((long)ulPos < 0 && -ulPos > ulTmp)
				ulPos = 0;
			else
				ulPos += ulTmp;
		}

		pDrive = DriveToPointer(pf->GetDrive());
		if (pDrive->m_state.op != FATOP_None)
			return -1;

		if (ulPos > pf->Length())
			return -1;

		pDrive->m_state.handle = handle;
		pDrive->m_state.op = FATOP_Seek;
		pDrive->m_state.dwSeekPos = ulPos;

		return ulPos;
	}

	//****************************************************************************

	static ulong GetPosition(uint handle) NO_INLINE_ATTR
	{
		FatFile		*pf;

		pf = HandleToPointer(handle);
		if (!pf->IsOpen() || pf->IsFolder())
			return -1;

		return pf->CurPos();
	}

	//****************************************************************************

	static ulong GetSize(uint handle) NO_INLINE_ATTR
	{
		FatFile		*pf;

		pf = HandleToPointer(handle);
		if (!pf->IsOpen() || pf->IsFolder())
			return -1;

		return pf->Length();
	}

	//****************************************************************************

	static int StartGetDate(uint handle) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		int			err;

		pf = HandleToPointer(handle);
		if (!pf->IsOpen())
			return FATERR_InvalidHandle;

		pDrive = DriveToPointer(pf->GetDrive());
		err = pDrive->StartBuf(pf->m_DirSec);
		if (err != FATERR_Busy)
			return err;
		pDrive->m_state.op = FATOP_GetDate;
		return FATERR_None;
	}

	//*********************************************************************
	// Helpers
	//*********************************************************************

protected:
	static FatDrive *DriveToPointer(byte drive)	{ return m_arDrives[drive]; }
	static FatFile *HandleToPointer(int h)	{return FatDrive::HandleToPointer(h);}

	//****************************************************************************

	static int ReadWrite(uint handle, void *pv, uint cb, byte op) NO_INLINE_ATTR
	{
		FatFile		*pf;
		FatDrive	*pDrive;
		ulong		ulTmp;

		pf = HandleToPointer(handle);
		if (pf->m_flags.OpenType != OPENTYPE_Normal || pf->IsFolder())
			return FATERR_InvalidHandle;

		pDrive = DriveToPointer(pf->GetDrive());
		if (pDrive->m_state.op != FATOP_None)
			return FATERR_Busy;

		pDrive->m_state.pb = (byte *)pv;
		pDrive->m_state.handle = handle;
		pDrive->m_state.status = 0;

		if (op == FATOP_Read)
		{
			// Make sure we don't read past EOF
			if (pf->Length() <= pf->CurPos())
				cb = 0;
			else
			{
				ulTmp = pf->Length() - pf->CurPos();
				if (ulTmp < cb)
					cb = ulTmp;
			}
		}
		else
			pf->m_flags.fDirty = 1;

		if (cb > 0xFFFF)
			cb = 0xFFFF;
		pDrive->m_state.cb = cb;
		if (cb == 0)
			op = FATOP_Status;

		pDrive->m_state.op = op;
		return FATERR_None;
	}

	//****************************************************************************

	static int GetHandle(uint hFolder, uint flags) NO_INLINE_ATTR
	{
		FatFile		*pfFolder;
		FatFile		*pf;
		byte		h;

 		pf = FatDrive::GetHandleList();

		if (hFolder != 0)
		{
			pfFolder = HandleToPointer(hFolder);
			if (!pfFolder->IsOpen() || !pfFolder->IsFolder())
				return FATERR_InvalidHandle;
		}

		// Make sure we have an unused handle available
		for (h = 1; ;)
		{
			if (!pf->IsOpen())
				break;
			pf++;
			h++;

			if (h > FAT_MAX_HANDLES)
				return FATERR_NoHandles;
		}

		// Duplicate handle
		if (hFolder == 0)
			DriveToPointer(0)->InitRootSearch(pf);
		else
			HandleToPointer(hFolder)->DupFolder(pf);

		return h;
	}

	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

protected:
	static FatDrive	*m_arDrives[FAT_NUM_DRIVES];
};

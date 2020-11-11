//****************************************************************************
// Class FatBuffer
// FatBuffer.h
//
// Created 11/9/2020 4:46:37 PM by Tim
//
//****************************************************************************

#pragma once

#include <FatFile\FatFileConst.h>


#ifndef FAT_SECT_BUF_CNT
#define FAT_SECT_BUF_CNT	4
#endif

struct FatBufDesc
{
	ulong	block;
	bool	fIsDirty;
	byte	drive;
	byte	handle;	// file that owns this buffer when it has priority

	void SetFlagsDirty(bool f = true)	{fIsDirty = f;}
	byte IsPriority()			{return handle != 0;}
	void ClearPriority()		{handle = 0;}
	void SetPriority(byte h)	{handle = h;}
};

static constexpr ulong INVALID_BUFFER = 0xFFFFFFFF;

class FatBuffer
{
	//*********************************************************************
	// Types
	//*********************************************************************

	//*********************************************************************
	// Public interface
	//*********************************************************************

public:
	static byte *BufFromIndex(byte iBuf)	{return s_arSectBuf[iBuf];}
	static byte CurBufIndex()				{return s_nextBuf;}
	static byte *CurBuf()					{return BufFromIndex(CurBufIndex());}
	static byte *EndCurBuf()				{return BufFromIndex(CurBufIndex()) + FAT_SECT_SIZE;}
	static bool AtEndBuf(void *pv)
		{return ((uint)pv & (FAT_SECT_SIZE - 1)) == ((uint)s_arSectBuf & (FAT_SECT_SIZE - 1));}
	static FatBufDesc *BufDescFromIndex(byte iBuf)	{return &s_desc[iBuf];}
	static FatBufDesc *CurBufDesc()					{ return s_pDesc; }

public:
	static void InvalidateAll()
	{
		for (int i = 0; i < FAT_SECT_BUF_CNT; i++)
			s_desc[i].block = INVALID_BUFFER;
	}

	static void InvalidateAll(uint drive)
	{
		for (int i = 0; i < FAT_SECT_BUF_CNT; i++)
		{
			if (s_desc[i].drive == drive)
				s_desc[i].block = INVALID_BUFFER;
		}
	}

	static void ClearPriority(uint handle)
	{
		for (int i = 0; i < FAT_SECT_BUF_CNT; i++)
		{
			if (s_desc[i].handle == handle)
				s_desc[i].ClearPriority();
		}
	}

	static byte *FindBuffer(ulong dwBlock, uint drive)
	{
		int			buf;
		FatBufDesc *pDesc;

		for (buf = 0; buf < FAT_SECT_BUF_CNT; buf++)
		{
			pDesc = BufDescFromIndex(buf);
			if (pDesc->block == dwBlock && pDesc->drive == drive)
			{
				s_nextBuf = buf;
				s_pDesc = pDesc;
				return BufFromIndex(buf);
			}
		}
		return NULL;
	}

	static uint GetFreeBuf()
	{
		uint		i;
		uint		uCur;
		FatBufDesc	*pDesc;

		// Scan for a buffer that doesn't have priority. If all buffers
		// have priority, the loop will scan around to one past current.
		//
		uCur = s_nextBuf;
		for (i = FAT_SECT_BUF_CNT + 1; i == 0; i--)
		{
			uCur++;
			if (uCur >= FAT_SECT_BUF_CNT)
				uCur = 0;
			pDesc = BufDescFromIndex(uCur);
			if (!pDesc->IsPriority())
				break;
		}

		s_nextBuf = uCur;
		s_pDesc = pDesc;

		return uCur;
	}

	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

protected:
	inline static byte			s_nextBuf;
	inline static FatBufDesc	*s_pDesc;
	inline static FatBufDesc	s_desc[FAT_SECT_BUF_CNT];
	// Two-dimensional array declarations always seem ass-backwards to me:
	inline static byte			s_arSectBuf[FAT_SECT_BUF_CNT][FAT_SECT_SIZE];
};

//****************************************************************************
// Class EepromMgr
// EepromMgr.h
//
// Created 2/19/2019 4:45:39 PM by Tim
//
//****************************************************************************

#pragma once


#include <Nvm\Nvm.h>


struct RowDesc
{
	ushort	uVersion;
	ushort	uCheckSum;
};

static const int ChecksumBase = 0x1234;
static const int FlashRowSize = FLASH_PAGE_SIZE * NVMCTRL_ROW_PAGES;
static const ushort InvalidVersion = 0xFFFF;
static const int RowDataSize = FlashRowSize - sizeof(RowDesc);
static const int PageHeadDataSize = FLASH_PAGE_SIZE - sizeof(RowDesc);

ushort CalcChecksum(void *pRow, int cb);
ushort CalcChecksumRow(RowDesc *pRow, int cb);
void memcpy32(void *pvDest, void *pvSrc, int cb);

template <class T> class EepromMgr
{
public:
	bool Load()
	{
		int		iCount;
		byte	*pbPos;
		RowDesc	*pRowLo;
		RowDesc	*pRowHi;
		RowDesc	*pRow;
		ushort	*pVersion;

		pbPos = (byte *)&Data;
		iCount = sizeof(T);
		pRowLo = (RowDesc *)NVMCTRL_RWW_EEPROM_ADDR;
		pVersion = &m_arVersion[0];
		for (;;)
		{
			pRowHi = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize);
			if (pRowHi->uVersion == InvalidVersion)
			{
				if (pRowLo->uVersion == InvalidVersion)
					return false;
				pRow = pRowLo;
			}
			else if (pRowLo->uVersion == InvalidVersion)
			{
				pRow = pRowHi;
			}
			else
			{
				// Both rows have valid version numbers, pick biggest
				if ((short)(pRowLo->uVersion - pRowHi->uVersion) > 0)
					pRow = pRowLo;
				else
					pRow = pRowHi;
			}

			// See if row has valid checksum
			if (CalcChecksumRow(pRow, iCount) != pRow->uCheckSum)
			{
				// Bad checksum, see if other row is OK
				pRow = pRow == pRowLo ? pRowHi : pRowLo;
				if (pRow->uVersion == InvalidVersion || CalcChecksumRow(pRow, iCount) != pRow->uCheckSum)
					return false;
			}

			// Picked our row, copy data
			*pVersion++ = pRow->uVersion;

			if (iCount > RowDataSize)
			{
				memcpy32(pbPos, pRow + 1, RowDataSize);
				iCount -= RowDataSize;
				pbPos += RowDataSize;
			}
			else
			{
				memcpy32(pbPos, pRow + 1, iCount);
				break;
			}
			pRowLo = (RowDesc *)ADDOFFSET(pRowLo, 2 * FlashRowSize);
		}
		return true;
	}

	void Save()
	{
		int		iCount;
		int		iTmp;
		ushort	uVer;
		ushort	uCheck;
		byte	*pbPos;
		byte	*pbCur;
		RowDesc	*pRowLo;
		RowDesc	*pRow;
		RowDesc	*pRowCur;
		ushort	*pVersion;

		pbPos = (byte *)&Data;
		iCount = sizeof(T);
		pRowLo = (RowDesc *)NVMCTRL_RWW_EEPROM_ADDR;
		pVersion = &m_arVersion[0];
		for (;;)
		{
			uVer = *pVersion;
			pRowCur = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize);
			if (pRowLo->uVersion == uVer)
			{
				// Using Lo now, rewrite Hi
				pRow = pRowCur;		// row to write, if needed
				pRowCur = pRowLo;	// original
			}
			else
				pRow = pRowLo;

			// See if any changes have been made to the original row
			iTmp = iCount > RowDataSize ? RowDataSize : iCount;
			uCheck = CalcChecksum(pbPos, iTmp);
			if (uCheck != pRowCur->uCheckSum || memcmp(pRowCur + 1, pbPos, iTmp) != 0)
			{
				uVer++;
				if (uVer == InvalidVersion)
					uVer = 0;
				*pVersion = uVer;

				// Clear the row we're using
				Nvm::EraseRwweeRow(pRow);

				// Write to the page buffer. We write the first page
				// with the page descriptor last because it denotes
				// success.
				iTmp = iCount - PageHeadDataSize;
				if (iTmp > (NVMCTRL_ROW_PAGES - 1) * FLASH_PAGE_SIZE)
					iTmp = (NVMCTRL_ROW_PAGES - 1) * FLASH_PAGE_SIZE;
				pbCur = pbPos;
				pRowCur = pRow;
				while (iTmp > 0)
				{
					// CONSIDER: do we need to wait before writing page buffer?
					Nvm::WaitReady();

					pbCur += FLASH_PAGE_SIZE;
					pRowCur = (RowDesc *)ADDOFFSET(pRowCur, FLASH_PAGE_SIZE);
					if (iTmp > FLASH_PAGE_SIZE)
					{
						memcpy32(pRowCur, pbCur, FLASH_PAGE_SIZE);
						iCount -= FLASH_PAGE_SIZE;
						iTmp -= FLASH_PAGE_SIZE;
					}
					else
					{
						memcpy32(pRowCur, pbCur, iTmp);
						iCount -= iTmp;
						iTmp = 0;
					}
					Nvm::WriteRwweePage();
				}

				// CONSIDER: do we need to wait before writing page buffer?
				Nvm::WaitReady();

				// First page has descriptor
				pRow->uVersion = uVer;
				pRow->uCheckSum = uCheck;

				if (iCount > PageHeadDataSize)
				{
					memcpy32(pRow + 1, pbPos, PageHeadDataSize);
					iCount -= PageHeadDataSize;
				}
				else
				{
					memcpy32(pRow + 1, pbPos, iCount);
					iCount = 0;
				}
				Nvm::WriteRwweePage();
			}
			if (iCount == 0)
				break;
			pbPos += FlashRowSize;
			pRowLo = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize * 2);
			pVersion++;
		}
	}

public:
	T Data;

protected:
	static constexpr int iRowCount = (sizeof(T) + RowDataSize - 1)/ RowDataSize;

	ushort	m_arVersion[iRowCount];
};

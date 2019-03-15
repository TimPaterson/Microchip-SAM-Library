//****************************************************************************
// Class EepromMgr
// EepromMgr.h
//
// Created 2/19/2019 4:45:39 PM by Tim
//
//****************************************************************************

#pragma once


#include <Nvm\Nvm.h>
#include <algorithm>	// for min()


template <class T, const T *pInit> class EepromMgr
{
#define	ALIGN32(x)	(((x) + 3) & ~3)

protected:
	struct RowDesc
	{
		ushort	usVersion;
		ushort	usSize;
		ulong	ulCheckSum;
	};

	static constexpr int FlashRowSize = FLASH_PAGE_SIZE * NVMCTRL_ROW_PAGES;
	static constexpr ulong Unprogrammed = 0xFFFFFFFF;
	static constexpr ushort InvalidVersion = Unprogrammed;
	static constexpr int RowDataSize = FlashRowSize - sizeof(RowDesc);
	static constexpr int MaxEepromDataSize = NVMCTRL_RWWEE_PAGES / NVMCTRL_ROW_PAGES * RowDataSize;
	static constexpr int PageHeadDataSize = FLASH_PAGE_SIZE - sizeof(RowDesc);
	static constexpr int AlignedSize = ALIGN32(sizeof(T));
	static constexpr int iRowCount = (AlignedSize + RowDataSize - 1) / RowDataSize;
	
// public interface
public:
	void NO_INLINE_ATTR Init()
	{
		uint	iCount;
		
		iCount = Load();
		if (iCount < sizeof(T))
		{
			// Copy in exact amount of data. Padding will retain its zero initialization.
			memcpy(m_arbPaddedData + iCount, ADDOFFSET(pInit, iCount) , sizeof(T) - iCount);
		}
	}
	
	void NO_INLINE_ATTR Save()
	{
		int		iCount;
		int		cbRow;
		int		cbPage;
		ushort	usVer;
		ulong	ulCheck;
		byte	*pbPos;
		byte	*pbCur;
		RowDesc	*pRowLo;
		RowDesc	*pRow;
		RowDesc	*pRowCur;
		RowDesc	**ppRow;

		pbPos = m_arbPaddedData;
		iCount = AlignedSize;
		pRowLo = (RowDesc *)NVMCTRL_RWW_EEPROM_ADDR;
		ppRow = &m_arpRow[0];
		for (;;)
		{
			pRow = *ppRow;
			pRowCur = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize);
			if (pRowLo == pRow)
			{
				// Using Lo now, rewrite Hi
				pRow = pRowCur;		// row to write, if needed
				pRowCur = pRowLo;	// original
			}
			else
				pRow = pRowLo;

			// See if any changes have been made to the original row
			cbRow = std::min(iCount, RowDataSize);
			ulCheck = CalcChecksum(pbPos, cbRow);
			if (pRowCur->usSize != sizeof(T) || ulCheck != pRowCur->ulCheckSum || memcmp(pRowCur + 1, pbPos, cbRow) != 0)
			{
				usVer = pRowCur->usVersion + 1;
				if (usVer == InvalidVersion)
					usVer = 0;

				// Clear the row we're using
				Nvm::EraseRwweeRow(pRow);

				// Write to the page buffer. We write the first page
				// with the page descriptor last because it denotes
				// success.
				cbRow = std::min(iCount - PageHeadDataSize, (NVMCTRL_ROW_PAGES - 1) * FLASH_PAGE_SIZE);
				pbCur = pbPos + PageHeadDataSize;
				pRowCur = pRow;
				while (cbRow > 0)
				{
					Nvm::WaitReady();

					pRowCur = (RowDesc *)ADDOFFSET(pRowCur, FLASH_PAGE_SIZE);
					cbPage = std::min(cbRow, FLASH_PAGE_SIZE);
					memcpy32(pRowCur, pbCur, cbPage);
					Nvm::WriteRwweePage();
					pbCur += FLASH_PAGE_SIZE;
					iCount -= cbPage;
					cbRow -= cbPage;
				}

				Nvm::WaitReady();

				// First page has descriptor
				pRow->usVersion = usVer;
				pRow->usSize = sizeof(T);
				pRow->ulCheckSum = ulCheck;

				cbPage = std::min(iCount, PageHeadDataSize);
				memcpy32(pRow + 1, pbPos, cbPage);
				Nvm::WriteRwweePage();
				iCount -= cbPage;
			}
			else
				iCount -= cbRow;
				
			if (iCount == 0)
			{
				*ppRow = pRow;	// update row we are using
				break;
			}
			pbPos += RowDataSize;
			pRowLo = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize * 2);
			ppRow++;
		}
	}

// Protected methods
protected:
	int Load()
	{
		int		iCount;
		int		cbLoad;
		int		cbRow;
		RowDesc	*pRowLo;
		RowDesc	*pRowHi;
		RowDesc	*pRow;
		RowDesc	**ppRow;

		cbLoad = 0;
		pRowLo = (RowDesc *)NVMCTRL_RWW_EEPROM_ADDR;
		ppRow = &m_arpRow[0];
		do
		{
			pRowHi = (RowDesc *)ADDOFFSET(pRowLo, FlashRowSize);
			if (pRowHi->usVersion == InvalidVersion)
			{
				if (pRowLo->usVersion == InvalidVersion)
					return cbLoad;
				pRow = pRowLo;
			}
			else if (pRowLo->usVersion == InvalidVersion)
			{
				pRow = pRowHi;
			}
			else
			{
				// Both rows have valid version numbers, pick biggest.
				// Note version no. can wrap around zero, so we 
				// subtract them instead of just comparing.
				if ((short)(pRowLo->usVersion - pRowHi->usVersion) > 0)
					pRow = pRowLo;
				else
					pRow = pRowHi;
			}
			
			// Use count from first row
			if (cbLoad == 0)
				iCount = pRow->usSize;
				
			// See if row is valid
			if (pRow->usSize != iCount || 
				iCount > MaxEepromDataSize || 
				CalcChecksumRow(pRow, iCount - cbLoad) != pRow->ulCheckSum)
			{
				// Bad checksum, see if other row is OK
				pRow = pRow == pRowLo ? pRowHi : pRowLo;
				
				// Use count from first row
				if (cbLoad == 0)
					iCount = pRow->usSize;
				
				// See if row is valid
				if (pRow->usVersion == InvalidVersion || 
					pRow->usSize != iCount || 
					iCount > MaxEepromDataSize || 
					CalcChecksumRow(pRow, iCount - cbLoad) != pRow->ulCheckSum)
				{
					return cbLoad;
				}
			}

			// Picked our row, copy data
			*ppRow++ = pRow;

			cbRow = std::min(std::min(ALIGN32(iCount), AlignedSize) - cbLoad, RowDataSize);
			memcpy32(m_arbPaddedData + cbLoad, pRow + 1, cbRow);
			cbLoad += cbRow;
			pRowLo = (RowDesc *)ADDOFFSET(pRowLo, 2 * FlashRowSize);
			
		} while (cbLoad < iCount);
		
		return iCount;
	}

// Static functions
protected:
	static ulong NO_INLINE_ATTR CalcChecksumRow(RowDesc *pRow, int cb)
	{
		cb = ALIGN32(cb);
		return CalcChecksum(pRow + 1, std::min(cb, RowDataSize));
	}

	static ulong NO_INLINE_ATTR CalcChecksum(void *pRow, int cb)
	{
		int		sum;
		
#if	0	// Use CRC
		PAC->WRCTRL.reg = PAC_WRCTRL_KEY_CLR | PAC_WRCTRL_PERID(ID_DSU);
		DSU->ADDR.reg = (ulong)pRow;
		DSU->LENGTH.reg = cb;
		DSU->DATA.reg = 0xFFFFFFFF;
		DSU->CTRL.reg = DSU_CTRL_CRC;
		while (!DSU->STATUSA.bit.DONE);
		DSU->STATUSA.reg = DSU_STATUSA_DONE;	// reset bit
		sum = DSU->DATA.reg;
#else
		int		i;
		uint	*puData;
	
		sum = cb;
		puData = (uint *)pRow;
		for (i = 0; i < (int)(cb / sizeof *puData); i++)
		{
			sum += *puData++;
		}
#endif

		return sum;
	}

	static void NO_INLINE_ATTR memcpy32(void *pvDest, void *pvSrc, int cb)
	{
		uint	*puDest;
		uint	*puSrc;
	
		puDest = (uint *)pvDest;
		puSrc = (uint *)pvSrc;
		for (cb /= sizeof(uint); cb > 0; cb--)
			*puDest++ = *puSrc++;
	}

//*********************************************************************
// Data

public:
	union
	{
		T		Data;
		byte	m_arbPaddedData[AlignedSize];
	};

protected:
	RowDesc	*m_arpRow[iRowCount];
};

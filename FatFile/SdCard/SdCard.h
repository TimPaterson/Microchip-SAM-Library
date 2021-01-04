//****************************************************************************
// SdCard.h
//
// Created 11/11/2020 2:31:57 PM by Tim
//
//****************************************************************************

#pragma once

// Note that the following symbols must be defined at this point:
//
// FAT_SECT_BUF_CNT	- number of sector buffers
// FAT_MAX_HANDLES	- max number of file handles
// FAT_NUM_DRIVES	- number of drives
//
// This would typically be done in FatFileDef.h

#include <FatFile/FatDrive.h>
#include <FatFile/SdCard/SdConst.h>


#define SDCARD_MAX_READ_WAIT	1000
#define SDCARD_MAX_WRITE_WAIT	50000

#define	ErrGoTo(e, loc)		do {err = e; goto loc;} while (0)
#define	ErrGoDeselect(e)	ErrGoTo(e, Deselect)

template<class T>
class SdCard : public FatDrive, public T
{
	//*********************************************************************
	// Types
	//*********************************************************************

	enum SdReadWriteFlags
	{
		SDWAIT_None,
		SDWAIT_Read,
		SDWAIT_Write
	};

	enum SdMountFlags
	{
		SDMOUNT_NotMounted,
		SDMOUNT_LoCap,
		SDMOUNT_HiCap
	};

	//*********************************************************************
	// Implementation of Storage class
	//*********************************************************************
public:
	virtual int GetStatus()
	{
		int		err;
		uint	cBlock;

		if (m_state == SDWAIT_None)
			return STERR_None;

		Select();
		if (m_state == SDWAIT_Read)
		{
			err = ReadStatus();
			if (err == STERR_None)
			{
				T::ReadBytes(m_pData, SDCARD_BLOCK_SIZE);

				// Discard CRC bytes
				SpiRead();
				SpiRead();

				m_state = SDWAIT_None;
				Deselect();

				// Start next block
				cBlock = m_cBlock - 1;
				if (cBlock > 0)
					err = ReadData(++m_Lba, m_pData + SDCARD_BLOCK_SIZE, cBlock);
				return err;
			}
			else if (err != STERR_Busy)
				goto Finished;
		}
		else
		{
			err = SpiRead();
			if (err == 0)
			{
				if (--m_cRetry == 0)
					ErrGoTo(STERR_TimeOut, Finished);
				ErrGoDeselect(STERR_Busy);
			}

			err = GetCardStatus();
	Finished:
			m_state = SDWAIT_None;
		}

	Deselect:
		Deselect();
		return err;
	}

	virtual int InitDev()
	{
		return STERR_None;
	}

	virtual int MountDev()
	{
		return MountCard();
	}

	virtual int ReadData(ulong Lba, void *pv, uint cBlock)
	{
		int		err;

		m_Lba = Lba;
		m_pData = (byte *)pv;
		m_cBlock = cBlock;

		// Convert 512-byte block to byte address if not hi cap.
		if (m_fMount == SDMOUNT_LoCap)
			Lba <<= 9;

		Select();
		err = SendCommand(SDCARD_READ_BLOCK, Lba);
		Deselect();
		if (err != SDCARD_R1_READY)
			return MapR1Err(err);

		// Access has started
		m_state = SDWAIT_Read;
		m_cRetry = SDCARD_MAX_READ_WAIT;

		return STERR_Busy;
	}

	virtual int WriteData(ulong Lba, void *pv, uint cBlock)
	{
		byte	bTmp;
		int		err;

		m_Lba = Lba;
		m_pData = (byte *)pv;
		m_cBlock = cBlock;

		// Convert 512-byte block to byte address if not hi cap.
		if (m_fMount == SDMOUNT_LoCap)
			Lba <<= 9;

		Select();
		bTmp = SendCommand(SDCARD_WRITE_BLOCK, Lba);
		if (bTmp != SDCARD_R1_READY)
			ErrGoDeselect(MapR1Err(bTmp));

		// Send data token
		SpiWrite(SDCARD_START_TOKEN);
		T::WriteBytes((byte *)pv, SDCARD_BLOCK_SIZE);

		// Get data response
		for (int i = 0; i < 10; i++)
		{
			bTmp = SpiRead();
			if (bTmp != 0xFF)
				break;
		}

		bTmp &= SDCARD_DATRESP_MASK;
		if (bTmp != SDCARD_DATRESP_ACCEPTED)
		{
			if (bTmp == SDCARD_DATRESP_WRITE_ERR)
				ErrGoDeselect(GetCardStatus());
			ErrGoDeselect(STERR_DevFail);
		}

		// No error, set up time-out on write
		m_state = SDWAIT_Write;
		m_cRetry = SDCARD_MAX_WRITE_WAIT;
		err = STERR_Busy;

	Deselect:
		Deselect();
		return err;
	}

	virtual int DismountDev()
	{
		return STERR_None;
	}

	//*********************************************************************
	// Helpers
	//*********************************************************************

protected:
	void SpiWrite(byte b)	{ T::SpiWrite(b); }
	byte SpiRead()			{ return T::SpiRead(); }
	void Select()			{ T::Select(); }
	void Deselect()			{ T::Deselect(); }

	byte SendCommand(byte cmd, ulong ulArg = 0)
	{
		byte		bTmp;
		LONG_BYTES	lbArg;

		lbArg.ul = ulArg;

		// SPI must be ready
		SpiRead();
		SpiWrite(cmd | 0x40);

		SpiWrite(lbArg.bHi);
		SpiWrite(lbArg.bMidHi);
		SpiWrite(lbArg.bMidLo);
		SpiWrite(lbArg.bLo);

		if (cmd == SDCARD_SEND_IF_COND)
			bTmp = SDCARD_IF_COND_CRC;
		else
			bTmp = SDCARD_GO_IDLE_CRC;

		SpiWrite(bTmp);
		SpiRead();				// Skip first return byte

		// Get response
		for (int i = 0; i < 10; i++)
		{
			bTmp = SpiRead();
			if (bTmp != 0xFF)
				break;
		}

		return bTmp;
	}

	//****************************************************************************

	int MapR1Err(byte err)
	{
		if (err == SDCARD_R1_READY)
			return STERR_None;

		if (err & SDCARD_R1_ADDR_ERR)
			return STERR_InvalidAddr;

		if (err & SDCARD_R1_BAD_CMD)
			return STERR_BadCmd;

		if (err & SDCARD_R1_CRC_ERR)
			return STERR_BadBlock;

		return STERR_DevFail;
	}

	//****************************************************************************

	int ReadStatus() NO_INLINE_ATTR
	{
		int	err;

		err = CheckReadStatus();
		if (err != STERR_Busy)
			return err;

		// Still waiting for data
		if (--m_cRetry == 0)
			return STERR_TimeOut;
		return STERR_Busy;
	}

	//****************************************************************************

	int CheckReadStatus()
	{
		byte	bTmp;

		// Check for data
		bTmp = SpiRead();
		if (bTmp == SDCARD_START_TOKEN)
			return STERR_None;

		if (bTmp == 0xFF)
			return STERR_Busy;	// Still waiting for data

		// Data read error token
		if (bTmp & SDCARD_ERRTOK_BAD_DATA)
			return STERR_BadBlock;

		if (bTmp & SDCARD_ERRTOK_BAD_ADDR)
			return STERR_InvalidAddr;

		return STERR_DevFail;
	}

	//****************************************************************************

	byte GetCardStatus() NO_INLINE_ATTR
	{
		byte	b1;
		byte	b2;

		b1 = SendCommand(SDCARD_SEND_STATUS);
		b2 = SpiRead();	// 2nd byte of status
		if (b2 != 0)
		{
			if (b2 & (SDCARD_R2_WRITE_PROT | SDCARD_R2_CARD_LOCKED))
				return STERR_WriteProtect;
			return STERR_DevFail;
		}

		return MapR1Err(b1);
	}

	//****************************************************************************

	int MountCard()
	{
		byte	bTmp;
		int		i;
		int		err;

		if (!T::SdCardPresent())
			return STERR_NoMedia;

		T::SetClockSlow();

		for (i = 10; i > 0; i--)
			SpiRead();

		Select();

		for (i = SDCARD_BLOCK_SIZE - 1; i > 0; i--)
			SpiRead();

		bTmp = SendCommand(SDCARD_GO_IDLE_STATE);
		if (bTmp != SDCARD_R1_IDLE)
			ErrGoDeselect(STERR_NoMedia);

		// See if v.2 card
		bTmp = SendCommand(SDCARD_SEND_IF_COND, SDCARD_IF_COND_VHS_ARG);
		if (bTmp == SDCARD_R1_IDLE)
		{
			// v.2 card. Get last 4 bytes of response R7
			for (i = 3; i > 0; i--)
			{
				bTmp = SpiRead();
			}
			if (bTmp !=	SDCARD_VHS)
				goto Error;

			bTmp = SpiRead();
			if (bTmp !=	SDCARD_CHECK_PATTERN)
				goto Error;
		}

		// Initialize card
		for (i = 0; i < 10000; i++)
		{
			bTmp = SendCommand(SDCARD_APP_CMD);
			bTmp = SendCommand(SDCARD_APP_SEND_OP_COND, SDCARD_OP_COND_HCS_ARG);
			if (bTmp == SDCARD_R1_READY)
			{
				// See if hi capacity
				bTmp = SendCommand(SDCARD_READ_OCR);
				err = SDMOUNT_LoCap;
				if (bTmp == SDCARD_R1_READY)
				{
					// Get last 4 bytes of response R3
					bTmp = SpiRead();
					if (bTmp & SDCARD_R3_MSB_CCS)
						err = SDMOUNT_HiCap;

					for (int j = 3; j > 0; j--)
						bTmp = SpiRead();
				}
				goto Deselect;
			}
		}

	Error:
		err = STERR_TimeOut;
	Deselect:
		// Crank the speed up
		Deselect();
		T::SetClockFast();
		m_fMount = IsError(err) ? SDMOUNT_NotMounted : err;
		return err;
	}
	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

	byte	*m_pData;
	ulong	m_Lba;
	ushort	m_cRetry;
	byte	m_cBlock;
	byte	m_state;
	byte	m_fMount;
};

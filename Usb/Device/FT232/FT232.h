#pragma once

#include "FT232Def.h"
#include <Usb/Device/USBdevice.h>


#define FTDI_IN_ENDPOINT		1
#define FTDI_OUT_ENDPOINT		2
#define FTDI_BUFFER_SEED		USB_DEV_Product
#define FTDI_BUFFER_SEED_SIZE	2

//****************************************************************************
// FT232 Serial-to-USB converter
//****************************************************************************


class FT232 : public USBdevice
{
	//*********************************************************************
	// These are callbacks implemented in customized code
	//*********************************************************************

protected:
	static void RxData(void *pv, int cb);

	//*********************************************************************
	// Public interface
	//*********************************************************************

public:
	static int TxData(void *pbTx, int cbTx)
	{
		byte	*pb;
		int		cb;
		int		cbAvail;

		pb = (byte *)bufTrans.GetCur();
		cb = bufTrans.GetInUse();
		cbAvail = FTDI_IN_BUF_SIZE - cb;
		if (cbTx > cbAvail)
			cbTx = cbAvail;
		memcpy(pb + cb, pbTx, cbTx);
		cb += cbTx;
		bufTrans.SetInUse(cb);
		if (cb >= FTDI_IN_BUF_SIZE)
		{
			// buffer full, send it off
			TxDataRequest(FTDI_IN_ENDPOINT);
		}
		return cbTx;
	}

	static bool TxByte(byte b)
	{
		byte	*pb;
		int		cb;

		pb = (byte *)bufTrans.GetCur();
		cb = bufTrans.GetInUse();
		if (cb < FTDI_IN_BUF_SIZE)
		{
			pb[cb++] = b;
			bufTrans.SetInUse(cb);
			return true;
		}
		return false;
	}

	//*********************************************************************
	// Local Types
	//*********************************************************************

	template<int cbBuf>
	class BufMgr
	{
public:
		void *GetCur()			{ return &arBuf[usCur]; }
		void Swap()				{ usCur ^= 1; }
		// count of bytes in use only used for transmit buffers
		int GetInUse()			{ return usInUse; }
		void SetInUse(int cb)	{ usInUse = cb; }

protected:
		ushort		usCur;
		ushort		usInUse;
		uint32_t	arBuf[2][cbBuf/sizeof(uint32_t)];
	};

	struct SetupDataSrc
	{
		int		cb;
		const ushort *pus;
	};

	//*********************************************************************
	// Implementation of callbacks from USBdevice class
	//*********************************************************************

public:
	static void DeviceConfigured()
	{
		SeedTransBuf();
		ReceiveFromHost(FTDI_OUT_ENDPOINT, bufRcv.GetCur(), FTDI_OUT_BUF_SIZE);
	}

	static void RxData(int iEp, void *pv, int cb)
	{
		// Swap in other buffer
		bufRcv.Swap();
		ReceiveFromHost(FTDI_OUT_ENDPOINT, bufRcv.GetCur(), FTDI_OUT_BUF_SIZE);
		RxData(pv, cb);	// callback to report data
	}

	static void TxDataRequest(int iEp)
	{
		if (bufTrans.GetInUse() > FTDI_BUFFER_SEED_SIZE)
			SendBuffer(iEp);
	}

	static void TxDataSent(int iEp)
	{
	}

	static void StartOfFrame()
	{
		// If no data to send, go ahead and send empty buffer
		if (bufTrans.GetInUse() == FTDI_BUFFER_SEED_SIZE)
			SendBuffer(FTDI_IN_ENDPOINT);
	}

	static bool NonStandardSetup(UsbSetupPacket *pSetup)
	{
		int		cSrc;
		int		cCur;
		int		wTmp;
		ushort	*pus;
		const StringDesc	*pStr;
		const SetupDataSrc	*pSrc;

		switch (pSetup->bmRequestType)
		{
		case USBRT_DirOut | USBRT_TypeVendor | USBRT_RecipDevice:
			if (pSetup->bRequest == 3)
			{
				// Set baud rate
			}
			// Don't know what this is, just accept it
			AckControlPacket();
			return true;

		case USBRT_DirIn | USBRT_TypeVendor | USBRT_RecipDevice:
			switch(pSetup->bRequest)
			{
			case 0x05:
				// FT232 returns the idProduct for this request
				pus = (ushort *)GetSetupBuf();
				*pus = USB_DEV_Product;
				break;

			case 0x90:
				cCur = pSetup->wIndex;
				pSrc = arSrc;
				for (;;)
				{
					cSrc = pSrc->cb;
					if (cSrc == 0)
					{
						pStr = GetSerialStrDesc();
						cSrc = pStr->desc.bLength / 2;
						if (cCur < cSrc)
						{
							pus = (ushort *)pStr;
							wTmp = *(pus + cCur);
							goto SendBytes;
						}
					}
					else if (cCur < cSrc)
						break;
					pSrc++;
					if (pSrc >= &arSrc[_countof(arSrc)])
						return false;
					cCur -= cSrc;
				}

				pus = (ushort *)pSrc->pus;
				if (pus == NULL)
					wTmp = 0;
				else
					wTmp = *(pus + cCur);
SendBytes:
				pus = (ushort *)GetSetupBuf();
				*pus = wTmp;
				break;

			default:
				return false;

			} // switch bRequest

			SendControlPacket(2);
			return true;

		} // switch bmRequestType
		return false;
	}

	//*********************************************************************
	// Helpers
	//*********************************************************************

protected:
	static void SendBuffer(int iEp)
	{
		SendToHost(iEp, bufTrans.GetCur(), bufTrans.GetInUse());
		bufTrans.Swap();
		SeedTransBuf();
	}

	static void SeedTransBuf()
	{
		byte	*pb;

		// Seed the transmit buffer
		pb = (byte *)bufTrans.GetCur();
		*pb++ = LOBYTE(FTDI_BUFFER_SEED);
		*pb++ = HIBYTE(FTDI_BUFFER_SEED);
		bufTrans.SetInUse(FTDI_BUFFER_SEED_SIZE);
	}

	//*********************************************************************
	// const (flash) data
	//*********************************************************************

protected:
	// The FT232 has a vendor-specific request to return some data, some
	// of which has unknown meaning. This table provides the data as 
	// seen with a USB analyzer

	inline static const ushort arusSetupData1[] =
	{
		0x4000, USB_DEV_Vendor, USB_DEV_Product, 0x000, 0x2DA0, 
		0x0008, 0x0000, 0x0A98, 0x20A2, 0x12C2, 0x1023, 0x0005
	};

	inline static const ushort arusSetupData2[] =
	{
		0x0B3B, 0xC0AC
	};

	inline static const ushort arusSetupData3[] =
	{
		0xC85A
	};

	inline static const SetupDataSrc arSrc[] =
	{
		{ _countof(arusSetupData1), arusSetupData1 },
		{ VendorStr.desc.bLength / 2, (ushort *)&VendorStr },
		{ ProductStr.desc.bLength / 2, (ushort *)&ProductStr },
		{ 0, NULL },	// Zero length indicates Serial Number
		{ _countof(arusSetupData2), arusSetupData2 },
		{ 19, NULL },
		{ _countof(arusSetupData3), arusSetupData3 },
	};

	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

protected:
	inline static BufMgr<FTDI_IN_BUF_SIZE> bufTrans;
	inline static BufMgr<FTDI_OUT_BUF_SIZE> bufRcv;
};

//****************************************************************************
// Callbacks from USBdevice class
//****************************************************************************

inline void USBdevice::DeviceConfigured()
{
	FT232::DeviceConfigured();
}

inline void USBdevice::RxData(int iEp, void *pv, int cb)
{
	FT232::RxData(iEp, pv, cb);
}

inline void USBdevice::TxDataRequest(int iEp)
{
	FT232::TxDataRequest(iEp);
}

inline void USBdevice::TxDataSent(int iEp)
{
	FT232::TxDataSent(iEp);
}

inline bool USBdevice::NonStandardSetup(UsbSetupPacket *pSetup)
{
	return FT232::NonStandardSetup(pSetup);
}

#ifdef USB_SOF_INT
inline void USBdevice::StartOfFrame()
{
	FT232::StartOfFrame();
}
#endif

//****************************************************************************
// KeyboardHost.h
//
// Created 9/20/2020 12:01:37 PM by Tim
//
//****************************************************************************

#pragma once

#include <Usb/Host/UsbHostDriver.h>


class KeyboardHost : public UsbHostDriver
{
	//*********************************************************************
	// Types
	//*********************************************************************

	static constexpr int PacketBufferSize = 8;

	enum DevState
	{
		DS_Idle,
		DS_SetConfig,
		DS_WaitConfig,
		DS_SetProtocol,
		DS_WaitProtocol,
		DS_Poll,
		DS_HaveKey,
	};

	struct KeyBuffer
	{
		union
		{
			byte	bModifiers;
			struct 
			{
				byte	Lctl:1;
				byte	Lshift:1;
				byte	Lalt:1;
				byte	Lgui:1;
				byte	Rctl:1;
				byte	Rshift:1;
				byte	Ralt:1;
				byte	Rgui:1;
			};
		};
		byte	Reserved;
		byte	Keys[6];
	};

	//*********************************************************************
	// Override of virtual functions
	//*********************************************************************

	virtual UsbHostDriver *IsDriverForDevice(ulong ulVidPid, ulong ulClass, UsbConfigDesc *pConfig)
	{
		int		iPipe;
		int		cEp;
		UsbInterfaceDesc	*pIf;
		UsbEndpointDesc		*pEp;

		if (ulClass != 0)
			return NULL;

		m_bConfig = pConfig->bConfigValue;

		pIf = (UsbInterfaceDesc *)ADDOFFSET(pConfig, pConfig->bLength);
		cEp = pIf->bNumEndpoints;
		if (pIf->bInterfaceClass != USBCLASS_Hid || 
			pIf->bInterfaceSubclass != USBHIDSUB_Boot || 
			pIf->bInterfaceProtocol != USBHIDPROTO_Keyboard)
			return NULL;

		pEp = (UsbEndpointDesc *)ADDOFFSET(pIf, pIf->bLength);
		
		// Search for IN endpoint
		while (cEp > 0)
		{
			if (pEp->bDescType == USBDESC_Endpoint)
			{
				if ((pEp->bEndpointAddr & USBEP_DirIn) &&
					pEp->bmAttributes == USBEP_Interrupt &&
					pEp->wMaxPacketSize <= 8)
				{
					iPipe = USBhost::RequestPipe(this, pEp);
					if (iPipe < 0)
						return NULL;
					m_bInPipe = iPipe;
					m_stDev = DS_SetConfig;
					DEBUG_PRINT("Keyboard driver loaded\n");
					return this;
				}
				cEp--;
			}
			pEp = (UsbEndpointDesc *)ADDOFFSET(pEp, pEp->bLength);
		}
		return NULL;
	}

	virtual void SetupTransactionComplete(int cbTransfer)
	{
		switch (m_stDev)
		{
		case DS_WaitConfig:
			m_stDev = DS_SetProtocol;
			break;

		case DS_WaitProtocol:
			m_stDev = DS_Poll;
			InitPolling();
			break;
		}
	}

	virtual void TransferComplete(int iPipe, int cbTransfer)
	{
		if (cbTransfer >= 3 && m_bufKey.Keys[0] != 0)
			m_stDev = DS_HaveKey;
		InitPolling();
	}

	virtual void TransferError(int iPipe, TransferErrorCode err)
	{
		DEBUG_PRINT("Data transfer error\n");
		InitPolling();
	}

	virtual void Process()
	{
		USBhost::ControlPacket	pkt;

		switch (m_stDev)
		{
		case DS_SetConfig:
			// Must set next state before call
			m_stDev = DS_WaitConfig;
			if (!USBhost::SetConfiguration(this, m_bConfig))
				m_stDev = DS_SetConfig;	// change state back
			break;

		case DS_SetProtocol:
			pkt.packet.bmRequestType = USBRT_DirOut | USBRT_TypeClass | USBRT_RecipIface;
			pkt.packet.bRequest = USBHIDREQ_Set_Protocol;
			pkt.packet.wValue = USBHIDRP_Boot;
			pkt.packet.wIndex = 0;	// interface no.
			pkt.packet.wLength = 0;
			// Must set next state before call
			m_stDev = DS_WaitProtocol;
			if (!USBhost::ControlTransfer(this, NULL, pkt.u64))
				m_stDev = DS_SetProtocol;	// change state back
			break;

		case DS_Poll:
			if (IsPollTime())
				USBhost::ReceiveData(m_bInPipe, &m_bufKey, PacketBufferSize);
			break;

		case DS_HaveKey:
			DEBUG_PRINT("Key: %i\n", m_bufKey.Keys[0]);
			m_stDev = DS_Poll;
			break;
		}
	}

	//*********************************************************************
	// Helpers
	//*********************************************************************

protected:
	void InitPolling()
	{
		m_fPollStart = true;
	}

	bool IsPollTime()
	{
		if (m_fPollStart)
		{
			m_fPollStart = false;
			return true;
		}
		return false;
	}

	//*********************************************************************
	// Instance data
	//*********************************************************************

	// USB buffer must 4-byte aligned
	KeyBuffer	m_bufKey ALIGNED_ATTR(4);

	bool	m_fPollStart;
	byte	m_stDev;
	byte	m_bConfig;
	byte	m_bInPipe;
};

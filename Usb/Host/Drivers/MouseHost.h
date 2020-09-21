//****************************************************************************
// MouseHost.h
//
// Created 9/20/2020 3:55:22 PM by Tim
//
//****************************************************************************

#pragma once


#include <Usb/Host/UsbHostDriver.h>


class MouseHost : public UsbHostDriver
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
		DS_HaveMove,
	};

	struct MouseBuffer
	{
		union
		{
			byte	bButtons;
			struct 
			{
				byte	btnLeft:1;
				byte	btnRight:1;
				byte	btnMiddle:1;
				byte	btnX0:1;
				byte	btnX1:1;
				byte	btnX2:1;
				byte	btnX3:1;
				byte	btnX4:1;
			};
		};
		sbyte	moveX;
		sbyte	moveY;
		byte	Reserved[7];
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
			pIf->bInterfaceProtocol != USBHIDPROTO_Mouse)
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
					DEBUG_PRINT("Mouse driver loaded\n");
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
		if (cbTransfer >= 3)
			m_stDev = DS_HaveMove;
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
				USBhost::ReceiveData(m_bInPipe, &m_bufMouse, PacketBufferSize);
			break;

		case DS_HaveMove:
			DEBUG_PRINT("Motion X: %i, Y: %i, buttons: %02X\n", m_bufMouse.moveX, m_bufMouse.moveY, m_bufMouse.bButtons);
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
	MouseBuffer	m_bufMouse;// ALIGNED_ATTR(4);

	bool	m_fPollStart;
	byte	m_stDev;
	byte	m_bConfig;
	byte	m_bInPipe;
};

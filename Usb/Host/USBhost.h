//****************************************************************************
// USBhost.h
//
// Created 9/8/2020 5:21:28 PM by Tim
//
//****************************************************************************

#pragma once

#include "SamUsbHost.h"
#include "..\UsbCtrl.h"
#include "UsbHostDriver.h"


#define DEFINE_USB_ISR()		void USB_Handler() { USBhost::UsbIsr(); }
#define USB_DRIVER_LIST(...)	UsbHostDriver *USBhost::arHostDriver[] = {__VA_ARGS__};

class EnumerationDriver;
void HexDump(byte *pb, int cb);


//*************************************************************************
// USBhost Class
//*************************************************************************

class USBhost : public UsbCtrl
{
	//*********************************************************************
	// Types
	//*********************************************************************

	static constexpr int DelayConnectToResetMs = 500;
	static constexpr int DelayResetToGetDescriptorMs = 500;

public:
	union ControlPacket
	{
		UsbSetupPacket	packet;
		uint64_t		u64;
	};
		
protected:
	static constexpr int SetupBufSize = 256;

	// Tuck the pointer to the UsbHostDriver into unused fields
	// of Bank1 of the Pipe descriptor
	union PipeDescriptor
	{
		UsbHostDescBank		HostDescBank[2];
		struct  
		{
			UsbHostDescBank	Bank0;
			ulong			Reserved[2];	// ADDR and PCKSIZE fields
			UsbHostDriver	*pDriver;
		};
	};

	union SetupBuffer_t
	{
		uint32_t		buf[SetupBufSize / sizeof(uint32_t)];
		UsbSetupPacket	packet;
		UsbDeviceDesc	descDev;
		uint64_t		u64;
	};

	enum EnumState
	{
		ES_Idle,
		ES_Connected,
		ES_ResetComplete,
		ES_SetAddress,
		ES_DevDesc,
		ES_ConfigDesc,
		ES_FindDriver,
	};

	enum SetupState
	{
		SS_Idle,
		SS_CtrlRead,
		SS_CtrlWrite,
		SS_NoData,
		SS_GetStatus,	// after a write
		SS_SendStatus,	// after a read
		SS_WaitAck,		// Get/send final ACK
	};

	//*********************************************************************
	// Public interface
	//*********************************************************************

public:
	static void Init()
	{
		UsbCtrl::Init();
		USB->HOST.CTRLA.reg = USB_CTRLA_MODE_HOST | USB_CTRLA_ENABLE;
		// Enable Connect/Disconnect and Reset interrupts
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DCONN | 
			USB_HOST_INTENSET_DDISC | USB_HOST_INTENSET_RST;

		// Set pointer to pipe descriptor in RAM
		USB->HOST.DESCADD.reg = (uint32_t)&PipeDesc;
	};

	static void Enable()
	{
		// Set VBUSOK
		USB->HOST.CTRLB.reg = USB_HOST_CTRLB_VBUSOK;
	}

	static void Disable()
	{
		// Turn off VBUSOK
		USB->HOST.CTRLB.reg = 0;
		stSetup = SS_Idle;
		stEnum = ES_Idle;
	}

	void Process()
	{
		if (stEnum != ES_Idle && stSetup == SS_Idle)
			ProcessEnum();
	}

	static bool ControlTransfer(UsbHostDriver *pDriver, void *pv, uint64_t u64packet) NO_INLINE_ATTR
	{
		if (stSetup != SS_Idle)
			return false;

		SetupBuffer.u64 = u64packet;
		StartSetup(pDriver, pv);
		return true;
	}

	static bool GetDescriptor(UsbHostDriver *pDriver, void *pv, ushort wValue, ushort wLength) NO_INLINE_ATTR
	{
		if (stSetup != SS_Idle)
			return false;

		// Initialize Setup packet
		SetupBuffer.packet.bmRequestType =  USBRT_DirIn | USBRT_TypeStd | USBRT_RecipDevice;
		SetupBuffer.packet.bRequest = USBREQ_Get_Descriptor;
		SetupBuffer.packet.wValue = wValue;
		SetupBuffer.packet.wIndex = 0;
		SetupBuffer.packet.wLength = wLength;
		StartSetup(pDriver, pv);
		return true;
	}

	//*********************************************************************
	// Helpers
	//*********************************************************************

protected:
	static void StartSetup(UsbHostDriver *pDriver, void *pv) NO_INLINE_ATTR
	{
		PipeDesc[0].HostDescBank[0].ADDR.reg = (uint32_t)&SetupBuffer;
		PipeDesc[0].HostDescBank[0].PCKSIZE.reg =
			USB_HOST_PCKSIZE_BYTE_COUNT(sizeof(SetupBuffer.packet)) | USB_HOST_PCKSIZE_SIZE(pDriver->m_PackSize);
		PipeDesc[0].HostDescBank[0].CTRL_PIPE.reg = pDriver->m_bAddr;
		PipeDesc[0].pDriver = pDriver;
		pvSetupData = pv;

		if (SetupBuffer.packet.Dir == USBRT_DirIn_Val)
			stSetup = SS_CtrlRead;
		else if (SetupBuffer.packet.wLength == 0)
			stSetup = SS_NoData;
		else
			stSetup = SS_CtrlWrite;
			
		USB->HOST.HostPipe[0].PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
			USB_HOST_PCFG_PTOKEN_SETUP;
		USB->HOST.HostPipe[0].PINTENSET.reg = USB_HOST_PINTFLAG_TXSTP |
			USB_HOST_PINTFLAG_STALL | USB_HOST_PINTFLAG_PERR;
		// Send the packet
		USB->HOST.HostPipe[0].PSTATUSSET.reg = USB_HOST_PSTATUSSET_BK0RDY;
		USB->HOST.HostPipe[0].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE;
	}

	static void ProcessEnum() NO_INLINE_ATTR
	{
		UsbHostDriver	*pDriver;
		ControlPacket	pkt;

		pDriver = arHostDriver[0];

		switch (stEnum)
		{
		case ES_Connected:
			if (!tmrEnum.CheckDelay_ms(DelayConnectToResetMs))
				return;

			// Device is connected, reset it
			USB->HOST.CTRLB.reg	 = USB_HOST_CTRLB_VBUSOK | USB_HOST_CTRLB_BUSRESET;
			stEnum = ES_Idle;
			break;

		case ES_ResetComplete:
			if (!tmrEnum.CheckDelay_ms(DelayResetToGetDescriptorMs))
				return;

			// Initialize Pipe 0 for control
			pDriver->m_bAddr = 0;	// default address
			pDriver->m_PackSize = USB_HOST_PCKSIZE_SIZE_8_Val;
			// Get 8 bytes of the device descriptor
			GetDescriptor(pDriver, &SetupBuffer, USBVAL_Type(USBDESC_Device), 8);
			break;

		case ES_SetAddress:
			pkt.packet.bmRequestType = USBRT_DirOut | USBRT_TypeStd | USBRT_RecipDevice;
			pkt.packet.bRequest = USBREQ_Set_Address;
			pkt.packet.wValue = 1;	// UNDONE: device address
			pkt.packet.wIndex = 0;
			pkt.packet.wLength = 0;
			ControlTransfer(pDriver, NULL, pkt.u64);
			break;

		case ES_DevDesc:
			GetDescriptor(pDriver, &SetupBuffer, USBVAL_Type(USBDESC_Device), sizeof SetupBuffer);
			break;

		case ES_ConfigDesc:
			GetDescriptor(pDriver, &SetupBuffer, USBVAL_Type(USBDESC_Config), sizeof SetupBuffer);
			break;
		}
	}

	// This function is called from the Interrupt Service Routine.
	// It sets enumeration state to the next step for ProcessEnum().
	//
	static void SetupComplete(uint cbTransfer)
	{
		int		cb;
		UsbHostDriver *pDriver;

		pDriver = arHostDriver[0];

		DEBUG_PRINT("Setup complete\n");
		if (cbTransfer != 0)
			HexDump((byte *)&SetupBuffer, cbTransfer);

		switch (stEnum)
		{
		case ES_ResetComplete:
			if (cbTransfer > offsetof(UsbDeviceDesc, bMaxPacketSize0))
			{
				cb = SetupBuffer.descDev.bMaxPacketSize0;
				// Make sure it's a power of two
				if ((cb & (cb - 1)) != 0 || cb < 8 || cb > 64)
					cb = 8;	// just use smallest packet size
				pDriver->m_PackSize = GetPacketSize(cb);
				stEnum = ES_SetAddress;
			}
			break;

		case ES_SetAddress:
			pDriver->m_bAddr = SetupBuffer.packet.wValue;	// Set the address
			stEnum = ES_DevDesc;
			break;

		case ES_DevDesc:
			// UNDONE: Get data from device descriptor
			stEnum = ES_ConfigDesc;
			break;

		case ES_ConfigDesc:
			// UNDONE: Get data from configuration descriptor
			stEnum = ES_Idle;
			break;
		}
	}

	static PipePacketSize GetPacketSize(uint cb)
	{
		uint	uSize;

		// Make sure it's a power of two
		if ((cb & (cb - 1)) != 0 || cb < 8 || cb > 64)
			return USB_HOST_PCKSIZE_SIZE_8_Val;	// just use smallest packet size

		if (cb < 32)
			uSize = cb / 16;
		else
			uSize = cb / 32 + 1;

		return (PipePacketSize)uSize;
	};

	//*********************************************************************
	// Interrupt service routine
	//*********************************************************************

public:
	static void UsbIsr() INLINE_ATTR
	{
		int		iPipe;
		int		intFlags;
		int		intEnFlags;
		int		pipeIntSummary;
		UsbHostPipe		*pPipe;
		PipeDescriptor	*pPipeDesc;
		UsbHostDriver	*pDriver;

		//*****************************************************************
		// Handle host-level interrupts

		intFlags = USB->HOST.INTFLAG.reg;
		if (intFlags & USB_HOST_INTFLAG_RST)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_RST;

			DEBUG_PRINT("USB reset done\n");

			stEnum = ES_ResetComplete;
			tmrEnum.Start();
		}

		if (intFlags & USB_HOST_INTFLAG_DCONN)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DCONN;

			DEBUG_PRINT("Device connected\n");

			stEnum = ES_Connected;
			tmrEnum.Start();
		}

		if (intFlags & USB_HOST_INTFLAG_DDISC)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DDISC;

			DEBUG_PRINT("Device disconnected\n");

			// Device is disconnected
			stEnum = ES_Idle;
		}

		// Check for other host-level flags here

		//*****************************************************************
		// Handle pipe-level interrupts

		pipeIntSummary = USB->HOST.PINTSMRY.reg;

		for (iPipe = 0; pipeIntSummary != 0; iPipe++, pipeIntSummary >>= 1)
		{
			if ((pipeIntSummary & 1) == 0)
				continue;

			pPipe = &USB->HOST.HostPipe[iPipe];
			pPipeDesc = &PipeDesc[iPipe];
			pDriver = pPipeDesc->pDriver;
			intFlags = pPipe->PINTFLAG.reg;
			intEnFlags = pPipe->PINTENSET.reg;

			PipeDesc[0].HostDescBank[0].ADDR.reg = (uint32_t)&SetupBuffer;

			if (iPipe == 0)
			{
				int		cb;

				// Control pipe
				if (intFlags & USB_HOST_PINTFLAG_TXSTP)
				{
					// Clear the interrupt flag
					pPipe->PINTFLAG.reg = USB_HOST_PINTFLAG_TXSTP;
					DEBUG_PRINT("Control packet sent\n");

					pPipeDesc->HostDescBank[0].ADDR.reg = (ulong)pvSetupData;
					cb = SetupBuffer.packet.wLength;

					switch (stSetup)
					{
					case SS_NoData:
						cbTransfer = 0;
						goto GetStatus;

					case SS_CtrlRead:
						pPipe->PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
							USB_HOST_PCFG_PTOKEN_IN;
						pPipeDesc->HostDescBank[0].PCKSIZE.reg = 
							USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(cb) | 
							USB_DEVICE_PCKSIZE_SIZE(pDriver->m_PackSize);
						// BK0RDY is zero, indicating bank is empty and ready to receive
						stSetup = SS_SendStatus;
						break;

					case SS_CtrlWrite:
						pPipe->PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
							USB_HOST_PCFG_PTOKEN_OUT;
						pPipeDesc->HostDescBank[0].PCKSIZE.reg = 
							USB_DEVICE_PCKSIZE_BYTE_COUNT(cb) | 
							USB_DEVICE_PCKSIZE_SIZE(pDriver->m_PackSize) |
							USB_DEVICE_PCKSIZE_AUTO_ZLP;
						// Set BK0RDY to indicate bank is full and ready to send
						pPipe->PSTATUSSET.reg = USB_HOST_PSTATUSSET_BK0RDY;
						stSetup = SS_GetStatus;
						break;
					};
						
					pPipe->PINTENSET.reg = USB_HOST_PINTFLAG_TRCPT0 | 
						USB_HOST_PINTFLAG_STALL | USB_HOST_PINTFLAG_PERR;
					pPipe->PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE;
				}

				if (intFlags & USB_HOST_PINTFLAG_TRCPT0)
				{
					// Clear the interrupt flag
					pPipe->PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT0;

					switch (stSetup)
					{
					case SS_GetStatus:
						DEBUG_PRINT("Control data sent\n");
						cbTransfer = pPipeDesc->HostDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE;
GetStatus:
						pPipe->PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
							USB_HOST_PCFG_PTOKEN_IN;
						pPipeDesc->HostDescBank[0].PCKSIZE.reg = 
							USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) | 
							USB_DEVICE_PCKSIZE_SIZE(pDriver->m_PackSize);
						// BK0RDY is zero, indicating bank is empty and ready to receive
						break;

					case SS_SendStatus:
						DEBUG_PRINT("Control data received\n");
						cbTransfer = pPipeDesc->HostDescBank[0].PCKSIZE.bit.BYTE_COUNT;
						pPipe->PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
							USB_HOST_PCFG_PTOKEN_OUT;
						pPipeDesc->HostDescBank[0].PCKSIZE.reg = 
							USB_DEVICE_PCKSIZE_BYTE_COUNT(0) | 
							USB_DEVICE_PCKSIZE_SIZE(pDriver->m_PackSize) |
							USB_DEVICE_PCKSIZE_AUTO_ZLP;
						// Set BK0RDY to indicate bank is full and ready to send
						pPipe->PSTATUSSET.reg = USB_HOST_PSTATUSSET_BK0RDY;
						break;

					case SS_WaitAck:
						DEBUG_PRINT("ACK sent/received\n");
						pDriver->SetupTransactionComplete(cbTransfer);
						stSetup = SS_Idle;
						return;
					}
						
					stSetup = SS_WaitAck;
					// Send or receive final ACK (or error)
					pPipe->PINTENSET.reg = USB_HOST_PINTFLAG_TRCPT0 | 
						USB_HOST_PINTFLAG_STALL | USB_HOST_PINTFLAG_PERR;
					pPipe->PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE;
				}

				if (intFlags & USB_HOST_PINTFLAG_STALL)
				{
					// Clear the interrupt flag
					pPipe->PINTFLAG.reg = USB_HOST_PINTFLAG_STALL;
					DEBUG_PRINT("Control STALL received\n");

					stSetup = SS_Idle;
				}

				if (intFlags & USB_HOST_PINTFLAG_PERR)
				{
					byte	status;

					// Clear the interrupt flag
					pPipe->PINTFLAG.reg = USB_HOST_PINTFLAG_PERR;

					status = pPipeDesc->HostDescBank[0].STATUS_PIPE.reg;
					if (status & USB_HOST_STATUS_PIPE_TOUTER)
						DEBUG_PRINT("Timeout\n");
					else
					{
						DEBUG_PRINT("Pipe error: %02X\n", status);
						stSetup = SS_Idle;
					}
				}
			}
			else
			{
				// Not the control pipe
				if ((intFlags & USB_HOST_PINTFLAG_TRCPT0) && 
					(intEnFlags && USB_HOST_PINTFLAG_TRCPT0))
				{
				}
			}
		}
	}

	//*********************************************************************
	// const (flash) data
	//*********************************************************************

	// Note this is NOT inline. It is defined in a code file using
	// the USB_DRIVER_LIST() macro.
	static UsbHostDriver *arHostDriver[HOST_DRIVER_COUNT];

	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

protected:
	inline static SetupBuffer_t SetupBuffer;
	inline static PipeDescriptor PipeDesc[8];
	inline static void *pvSetupData;
	inline static int cbTransfer;
	inline static Timer tmrEnum;
	inline static byte stEnum;
	inline static byte stSetup;

	//*********************************************************************
	// Allow enumeration driver to access stuff

	friend EnumerationDriver;
};


//****************************************************************************
// UsbHostDriver used during enumeration
//****************************************************************************

class EnumerationDriver : public UsbHostDriver
{
	virtual bool IsDriverForDevice(ulong ulVidPid, UsbConfigDesc *pConfig)
	{ 
		return true; 
	}

	virtual void SetupTransactionComplete(int cbTransfer)
	{
		USBhost::SetupComplete(cbTransfer);
	}

	virtual void TransferComplete(int iPipe)
	{
	}
};

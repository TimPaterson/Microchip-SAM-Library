//****************************************************************************
// USBhost.h
//
// Created 9/8/2020 5:21:28 PM by Tim
//
//****************************************************************************

#pragma once

#include "..\UsbCtrl.h"
#include "SamUsbHost.h"

#define DEFINE_USB_ISR(dev)	void USB_Handler() { dev::ISR.UsbIsr(); }


//*************************************************************************
// USBhostBase Class
//*************************************************************************

class USBhostBase : public UsbCtrl
{
	//*********************************************************************
	// Local Types
	//*********************************************************************

	enum UsbConnectionState
	{
		USBST_Detached,
		USBST_Attached,
		USBST_Default,
		USBST_Address,
		USBST_Configured,
	};

	//*********************************************************************
	// Public interface
	//*********************************************************************

public:
	static void Init()
	{
		UsbCtrl::Init();
		USB->HOST.CTRLA.reg = USB_CTRLA_MODE_HOST | USB_CTRLA_ENABLE;
		// Set pointer to pipe descriptor in RAM
		USB->HOST.DESCADD.reg = (uint32_t)&PipeDesc;
		// Enable Connect/Disconnect and Reset interrupts
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DCONN | 
			USB_HOST_INTENSET_DDISC | USB_HOST_INTENSET_RST;

		PipeDesc[0].HostDescBank[0].ADDR.reg = 0;

		// Set VBUSOK
		USB->HOST.CTRLB.reg = USB_HOST_CTRLB_VBUSOK;
	};

	//*********************************************************************
	// static (RAM) data
	//*********************************************************************

protected:
	inline static UsbHostDescriptor PipeDesc[8];
	inline static byte state;
};


//*************************************************************************
// USBhostIsr template class
//
// Interrupt service routine for USBhost. Passes callbacks to template
// parameter class T.
//
//*************************************************************************

template<class T>
class USBhostIsr : public USBhostBase
{

	//*********************************************************************
	// These are callbacks implemented in class T, the template argument
	//*********************************************************************

	/*
	*/

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
		UsbHostPipe	*pPipe;

		//*****************************************************************
		// Handle host-level interrupts

		intFlags = USB->HOST.INTFLAG.reg;
		if (intFlags & USB_HOST_INTFLAG_RST)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_RST;

			DEBUG_PRINT("USB reset done\n");

			// Initialize Pipe 0 for control
			USB->HOST.HostPipe[0].PCFG.reg = USB_HOST_PCFG_PTYPE_CONTROL | 
				USB_HOST_PCFG_PTOKEN_SETUP;
		}

		if (intFlags & USB_HOST_INTFLAG_DCONN)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DCONN;

			DEBUG_PRINT("Device connected\n");

			// Device is connected
		}

		if (intFlags & USB_HOST_INTFLAG_DDISC)
		{
			// Clear the interrupt flag
			USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DDISC;

			DEBUG_PRINT("Device disconnected\n");

			// Device is disconnected
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
			intFlags = pPipe->PINTFLAG.reg;
			intEnFlags = pPipe->PINTENSET.reg;

			if (iPipe == 0)
			{
				// Control pipe
				if ((intFlags & USB_HOST_PINTFLAG_TRCPT0) &&
					(intEnFlags & USB_HOST_PINTFLAG_TRCPT0))
				{
					pPipe->PINTENCLR.reg = USB_HOST_PINTFLAG_TRCPT0;
				}
			}
			else
			{
				// Not the control pipe
			}
		}
	}
};


//*************************************************************************
// USBhost
//
//*************************************************************************

template<class T>
class USBhost : public USBhostBase
{
	//*********************************************************************
	// The interrupt service routine - class with no data
	//*********************************************************************

public:
	static USBhostIsr<T>	ISR;
};

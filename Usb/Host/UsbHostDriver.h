//****************************************************************************
// UsbHostDriver.h
//
// Created 9/11/2020 1:24:04 PM by Tim
//
//****************************************************************************

#pragma once


class USBhost;

class UsbHostDriver
{
	//*********************************************************************
	// virtual functions called from ISR
	//*********************************************************************

	virtual bool IsDriverForDevice(ulong ulVidPid, UsbConfigDesc *pConfig) = 0;
	virtual void SetupTransactionComplete(int cbTransfer) = 0;
	virtual void TransferComplete(int iPipe) = 0;

	//*********************************************************************
	// Instance data
	//*********************************************************************

	byte	m_bAddr;
	PipePacketSize	m_PackSize;

	//*********************************************************************
	// Allow host to access stuff

	friend USBhost;
};

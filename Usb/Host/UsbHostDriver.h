//****************************************************************************
// UsbHostDriver.h
//
// Created 9/11/2020 1:24:04 PM by Tim
//
//****************************************************************************

#pragma once


class USBhost;

enum TransferErrorCode
{
	TEC_None,
	TEC_NoDriver,
	TEC_Stall,
	TEC_Error,
};

class UsbHostDriver
{
	//*********************************************************************
	// virtual functions
	//*********************************************************************

	//  called from ISR
	virtual UsbHostDriver *IsDriverForDevice(ulong ulVidPid, ulong ulClass, UsbConfigDesc *pConfig) = 0;
	virtual void SetupTransactionComplete(int cbTransfer) = 0;
	virtual void TransferComplete(int iPipe, int cbTranfer) = 0;
	virtual void TransferError(int iPipe, TransferErrorCode err) = 0;
	
	// called from main level
	virtual void Process() = 0;

	//*********************************************************************
	// Instance data
	//*********************************************************************

	UsbHostDriver	*m_pDriverNext;
	byte			m_bAddr;
	PipePacketSize	m_PackSize;

	//*********************************************************************
	// Allow host to access stuff

	friend USBhost;
};

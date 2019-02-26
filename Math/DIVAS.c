//****************************************************************************
// DIVAS.c
//
// Created 10/16/2018 12:00:02 PM by Tim
//
//****************************************************************************

#include <standard.h>


//****************************************************************************
// Helpers for Interrupts
//****************************************************************************

static inline uint IrqSaveAndDisable()
{
	uint	uSaveIrq;
	
	uSaveIrq = __get_PRIMASK();
	__disable_irq();
	return uSaveIrq;
}

static inline void IrqRestore(uint uSaveIrq)
{
	__set_PRIMASK(uSaveIrq);
}


//****************************************************************************
// 32-bit signed divide and mod
//
// 64-bit result has quotient in low half, remainder in high half
// Remainder ignored for divide

int __aeabi_idiv(int iDividend, int iDivisor) __attribute__ ((alias("__aeabi_idivmod")));

uint64_t __aeabi_idivmod(int iDividend, int iDivisor)
{
	uint	uSaveIrq;
	int		iQuotient;
	int		iRemainder;
	
	// Ensure interrupts disabled
	uSaveIrq = IrqSaveAndDisable();
	
	DIVAS_IOBUS->CTRLA.reg = DIVAS_CTRLA_SIGNED;	// signed, optimize leading zeros

	DIVAS_IOBUS->DIVIDEND.reg = iDividend;
	DIVAS_IOBUS->DIVISOR.reg = iDivisor;		// Starts the operation
	
	while (DIVAS_IOBUS->STATUS.bit.BUSY);		// Wait for completion

	iQuotient = DIVAS_IOBUS->RESULT.reg;
	iRemainder = DIVAS_IOBUS->REM.reg;
	
	// Restore interrupts
	IrqRestore(uSaveIrq);
	
	return ((uint64_t)iRemainder << 32) | (uint)iQuotient;
}

//****************************************************************************
// 32-bit unsigned divide
//
// 64-bit result has quotient in low half, remainder in high half
// Remainder ignored for divide

uint __aeabi_uidiv(uint uDividend, uint uDivisor) __attribute__ ((alias("__aeabi_uidivmod")));

uint64_t __aeabi_uidivmod(int uDividend, int uDivisor)
{
	uint	uSaveIrq;
	uint	uQuotient;
	uint	uRemainder;
	
	// Ensure interrupts disabled
	uSaveIrq = IrqSaveAndDisable();
	
	DIVAS_IOBUS->CTRLA.reg = 0;		// unsigned, optimize leading zeros

	DIVAS_IOBUS->DIVIDEND.reg = uDividend;
	DIVAS_IOBUS->DIVISOR.reg = uDivisor;		// Starts the operation
	
	while (DIVAS_IOBUS->STATUS.bit.BUSY);		// Wait for completion

	uQuotient = DIVAS_IOBUS->RESULT.reg;
	uRemainder = DIVAS_IOBUS->REM.reg;
	
	// Restore interrupts
	IrqRestore(uSaveIrq);
	
	return ((uint64_t)uRemainder << 32) | uQuotient;
}

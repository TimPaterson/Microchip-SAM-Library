//****************************************************************************
// I2c.h
//
// Created 11/14/2023 5:54:00 PM by Tim
//
//****************************************************************************

#pragma once


// Declare I2C port
//
// Read the SERCOM number as the last character of the name string (SERCOMn)
#define DECLARE_I2C(usart, ...) I2c<#usart[6] - '0', __VA_ARGS__>

// Define ISR
#define DEFINE_I2C_ISR(usart, var) \
	void usart##_Handler() {var.I2cIsr();}

//****************************************************************************

enum I2cStatus
{
	I2CSTAT_Ok,
	I2CSTAT_Busy,
	I2CSTAT_Error = 0x80,
};

#ifdef F_CPU
template <int iUsart, int bitRate, int clock = F_CPU>
#else
template <int iUsart, int bitRate, int clock>
#endif
class I2c
{
protected:
	static constexpr int ReadBit = 1;
	
	enum I2cBusState
	{
		I2CBUSSTATE_Unknown,
		I2CBUSSTATE_Idle,
		I2CBUSSTATE_Owner,
		I2CBUSSTATE_Busy
	};

	enum I2cCommands
	{
		I2CCMD_NoAction,
		I2CCMD_RepeatStart,
		I2CCMD_Read,
		I2CCMD_Stop,
		I2CCMD_Nack		// bit flag to OR in to command
	};

	static SercomI2cm *GetI2c(int i) {return ((SercomI2cm *)((byte *)SERCOM0 + ((byte *)SERCOM1 - (byte *)SERCOM0) * i)); }

public:
	byte GetStatus()					{ return m_status; }
public:
	static bool IsStatusOk(uint status)	{ return status == 0; }
	static bool IsBusy(byte status)		{ return status == I2CSTAT_Busy; }
		
public:
	static void I2cInit()
	{
		SERCOM_I2CM_CTRLA_Type	i2cCtrlA;
		SERCOM_I2CM_BAUD_Type	i2cBaud;

#if	defined(GCLK_PCHCTRL_GEN_GCLK0)
		// Enable clock
		MCLK->APBCMASK.reg |= 1 << (MCLK_APBCMASK_SERCOM0_Pos + iUsart);

		// Clock it with GCLK0
		GCLK->PCHCTRL[SERCOM0_GCLK_ID_CORE + iUsart].reg = GCLK_PCHCTRL_GEN_GCLK0 |
			GCLK_PCHCTRL_CHEN;
#else
		// Enable clock
		PM->APBCMASK.reg |= 1 << (PM_APBCMASK_SERCOM0_Pos + iUsart);

		// Clock it with GCLK0
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
			(GCLK_CLKCTRL_ID_SERCOM0_CORE + iUsart);
#endif

		i2cCtrlA.reg = 0;
		i2cCtrlA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;
		i2cCtrlA.bit.RUNSTDBY = 1;
		GetI2c(iUsart)->CTRLA.reg = i2cCtrlA.reg;
		GetI2c(iUsart)->CTRLB.reg = 0;
		
		// Set bit rate with 1/3 hi 2/3 lo duty cycle
		int period = clock / bitRate;
		int hi = (period + 1) / 3;
		int lo = period - hi;
		i2cBaud.reg = 0;
		i2cBaud.bit.BAUD = hi - 5;
		i2cBaud.bit.BAUDLOW = lo - 5;
		GetI2c(iUsart)->BAUD.reg = i2cBaud.reg;
	}

	static void Enable()
	{
		GetI2c(iUsart)->INTENSET.reg = SERCOM_I2CM_INTFLAG_ERROR | SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
		GetI2c(iUsart)->CTRLA.bit.ENABLE = 1;
		GetI2c(iUsart)->STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(I2CBUSSTATE_Idle);
		NVIC_EnableIRQ((IRQn)(SERCOM0_IRQn + iUsart));
	}

	static void Disable()
	{
		NVIC_DisableIRQ((IRQn)(SERCOM0_IRQn + iUsart));
		GetI2c(iUsart)->CTRLA.bit.ENABLE = 0;
		GetI2c(iUsart)->INTENCLR.reg = SERCOM_I2CM_INTFLAG_ERROR | SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
	}

public:
	void StartWriteRead(int addr, const void *pWrite, int cbWrite, void *pRead = NULL, int cbRead = 0) INLINE_ATTR
	{
		m_status = I2CSTAT_Busy;
		if (pWrite == NULL)
			cbWrite = 0;
		if (pRead == NULL)
			cbRead = 0;
		m_pWrite = (const byte *)pWrite;
		m_pRead = (byte *)pRead;
		m_cbWrite = cbWrite;
		m_cbRead = cbRead;
		m_addr = addr;
		GetI2c(iUsart)->ADDR.reg = addr;
	}
	
protected:
	inline static void SendCmd(int cmd)
	{
		*(((volatile byte *)&GetI2c(iUsart)->CTRLB.reg) + 2) = cmd;
	}
	
	//************************************************************************
	// Interrupt service routine

public:
	void I2cIsr()
	{
		uint	status;
		SercomI2cm	*pI2c = GetI2c(iUsart);

		if (pI2c->INTFLAG.bit.ERROR)
		{
			m_status = I2CSTAT_Error;
		}
		else if (pI2c->INTFLAG.bit.MB)
		{
			// Address or data byte was sent
			status = pI2c->STATUS.reg;
			if (status & SERCOM_I2CM_STATUS_RXNACK)
			{
				// byte not acknowledged
				m_status = I2CSTAT_Error;
				SendCmd(I2CCMD_Stop | I2CCMD_Nack);
			}
			else if (m_cbWrite != 0)
			{
				pI2c->DATA.reg = *m_pWrite++;
				m_cbWrite--;
			}
			else
			{
				// All done writing, read if requested
				if (m_cbRead != 0)
				{
					pI2c->ADDR.reg = m_addr | ReadBit;	// repeated start
				}
				else
				{
					m_status = I2CSTAT_Ok;
					SendCmd(I2CCMD_Stop | I2CCMD_Nack);
				}
			}
		}
		else if (pI2c->INTFLAG.bit.SB)
		{
			*m_pRead++ = pI2c->DATA.reg;
			if (--m_cbRead != 0)
				SendCmd(I2CCMD_Read);
			else
			{
				m_status = I2CSTAT_Ok;
				SendCmd(I2CCMD_Stop | I2CCMD_Nack);
			}
		}
		
		// Clear any interrupt flags
		pI2c->INTFLAG.reg = SERCOM_I2CM_INTFLAG_ERROR | SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
	}
	
	//*********************************************************************
	// instance (RAM) data
	//*********************************************************************
protected:
	// written in ISR
	volatile byte	m_status;
protected:
	byte	m_addr;
	ushort	m_cbWrite;
	ushort	m_cbRead;
	const byte*	m_pWrite;
	byte*	m_pRead;
};


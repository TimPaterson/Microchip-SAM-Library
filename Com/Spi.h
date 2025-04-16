//****************************************************************************
// Spi.h
//
// Created 6/18/2019 1:37:43 PM by Tim
//
//****************************************************************************

#pragma once


// Declare SPI port
//
// Read the SERCOM number as the last character of the name string (SERCOMn)
#define DECLARE_SPI(usart, ...) Spi<#usart[6] - '0', __VA_ARGS__>

//****************************************************************************

enum SpiInPad
{
	SPIMISOPAD_Pad0,
	SPIMISOPAD_Pad1,
	SPIMISOPAD_Pad2,
	SPIMISOPAD_Pad3
};

enum SpiOutPad
{
	SPIOUTPAD_Pad0_MOSI_Pad1_SCK,
	SPIOUTPAD_Pad2_MOSI_Pad3_SCK,
	SPIOUTPAD_Pad3_MOSI_Pad1_SCK,
	SPIOUTPAD_Pad0_MOSI_Pad3_SCK
};

enum SpiMode
{
	SPIMODE_0,	// CPOL = 0, CPHA = 0
	SPIMODE_1,	// CPOL = 0, CPHA = 1
	SPIMODE_2,	// CPOL = 1, CPHA = 0
	SPIMODE_3,	// CPOL = 1, CPHA = 1
};

//****************************************************************************

template <int sercom, uint ssPin, uint ssPort = 0, byte bDummy = 0>
class Spi
{
	static SercomSpi *pSpi() { return ((SercomSpi *)((byte *)SERCOM0 + ((byte *)SERCOM1 - (byte *)SERCOM0) * sercom)); }

public:
	static void SpiInit(SpiInPad padMiso, SpiOutPad padMosi, SpiMode modeSpi = SPIMODE_0)
	{
		SERCOM_SPI_CTRLA_Type	spiCtrlA;
		
		// Ensure SS pin is output and high level
		SetPins(ssPin, ssPort);
		DirOutPins(ssPin, ssPort);		

#if	defined(GCLK_PCHCTRL_GEN_GCLK0)
		// Enable clock
		MCLK->APBCMASK.reg |= 1 << (MCLK_APBCMASK_SERCOM0_Pos + sercom);

		// Clock it with GCLK0
		GCLK->PCHCTRL[SERCOM0_GCLK_ID_CORE + sercom].reg = GCLK_PCHCTRL_GEN_GCLK0 |
			GCLK_PCHCTRL_CHEN;
#else
		// Enable clock
		PM->APBCMASK.reg |= 1 << (PM_APBCMASK_SERCOM0_Pos + sercom);

		// Clock it with GCLK0
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
			(GCLK_CLKCTRL_ID_SERCOM0_CORE + sercom);
#endif

		// standard 8-bit, MSB first
		spiCtrlA.reg = 0;
		spiCtrlA.bit.MODE = 3;		// SPI host mode
		spiCtrlA.bit.DOPO = padMosi;
		spiCtrlA.bit.DIPO = padMiso;
		spiCtrlA.bit.CPHA = modeSpi & 1;
		spiCtrlA.bit.CPOL = modeSpi & 2;
		spiCtrlA.bit.IBON = 1;
		pSpi()->CTRLA.reg = spiCtrlA.reg;
		pSpi()->CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
	}

	static void Enable()
	{
		pSpi()->CTRLA.bit.ENABLE = 1;
	}

	static void Disable()
	{
		pSpi()->CTRLA.bit.ENABLE = 0;
	}

#ifdef F_CPU
	static uint CalcBaudRateConst(uint32_t rate)
	{
		return CalcBaudRateConst(rate, F_CPU);
	}

	static void SetBaudRateConst(uint32_t rate)
	{
		SetBaudRateConst(rate, F_CPU);
	}

	static void SetBaudRateConstEnabled(uint32_t rate)
	{
		SetBaudRateConstEnabled(rate, F_CPU);
	}
#endif

	static uint CalcBaudRateConst(uint32_t rate, uint32_t clock)
	{
		return DIV_UINT_RND(clock, rate * 2) - 1;
	}

	static void SetBaudRateConst(uint32_t rate, uint32_t clock)
	{
		SetBaudRateReg(CalcBaudRateConst(rate, clock));
	}

	static void SetBaudRateConstEnabled(uint32_t rate, uint32_t clock)
	{
		SetBaudRateRegEnabled(CalcBaudRateConst(rate, clock));
	}

	static void Select(bool fSelect)	{ if (fSelect) Select(); else Deselect(); }

	static byte WriteByte(byte b = bDummy) NO_INLINE_ATTR
	{
		SpiWrite(b);
		while (!IsByteReady());
		return SpiRead();
	}

	static void ReadBytes(void *pv, uint cb) NO_INLINE_ATTR
	{
		uint	cbSend;
		uint	cbRead;
		byte	*pb = (byte *)pv;

		cbRead = cb;
		cbSend = 2;		// size of read buffer

		for (;;)
		{
			if (cbSend > 0 && cb > 0 && CanSendByte())
			{
				SpiWrite();
				cbSend--;
				cb--;
			}

			if (IsByteReady())
			{
				*pb++ = SpiRead();
				if (--cbRead == 0)
					break;
				cbSend++;	
			}
		}
	}

	static void WriteBytes(const void *pv, uint cb) NO_INLINE_ATTR
	{
		const byte	*pb = (byte *)pv;
		
		ClearTxComplete();
		pSpi()->CTRLB.reg = 0;	// Disable receiver
		for (;;)
		{
			if (CanSendByte())
			{
				SpiWrite(*pb++);
				if (--cb == 0)
					break;
			}
		}
		while (!IsTxComplete());
		pSpi()->CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
	}

	static void WriteBytes(int cb, ...)
	{
		va_list	args;
		va_start(args, cb);
		for ( ; cb > 0; cb--)
			WriteByte(va_arg(args, int));
		va_end(args);
	}

	static void Select()	{ ClearPins(ssPin, ssPort); }
	static void Deselect()	{ SetPins(ssPin, ssPort); }

protected:
	static byte SpiRead()
	{
		return pSpi()->DATA.reg;
	}

	static void SpiWrite(byte b = bDummy)
	{
		pSpi()->DATA.reg = b;
	}

	static bool IsByteReady()
	{
		return pSpi()->INTFLAG.bit.RXC;
	}

	static bool CanSendByte()
	{
		return pSpi()->INTFLAG.bit.DRE;
	}

	static bool IsTxComplete()
	{
		return pSpi()->INTFLAG.bit.TXC;
	}

	static void ClearTxComplete()
	{
		pSpi()->INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC;
	}

	static bool IsRxOverflow()
	{
		return pSpi()->INTFLAG.bit.ERROR;
	}

	static void ClearOverflow()
	{
		pSpi()->INTFLAG.reg = SERCOM_SPI_INTFLAG_ERROR;
	}

	static void SetBaudRateReg(uint val)
	{
		pSpi()->BAUD.reg = val;
	}

	static void SetBaudRateRegEnabled(uint baud) INLINE_ATTR
	{
		SERCOM_SPI_CTRLA_Type	ctrlA;
		SERCOM_SPI_CTRLA_Type	ctrlAsave;

		ctrlAsave.reg = pSpi()->CTRLA.reg;
		ctrlA.reg = ctrlAsave.reg;
		ctrlA.bit.ENABLE = 0;
		pSpi()->CTRLA.reg = ctrlA.reg;
		SetBaudRateReg(baud);
		pSpi()->CTRLA.reg = ctrlAsave.reg;
	}
};

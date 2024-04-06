//****************************************************************************
// SpiClient.h
//
// Created 3/29/2024 3:57:49 PM by Tim
//
//****************************************************************************

#pragma once


// Declare client SPI port
//
// Read the SERCOM number as the last character of the name string (SERCOMn)
#define DECLARE_SPI_CLIENT(spi) SpiClient<#spi[6] - '0'>

#define DEFINE_SPI_CLIENT_ISR(spi, ...)	void spi##_Handler() { __VA_ARGS__; }

//****************************************************************************

template <int sercom>
class SpiClient
{
	static SercomSpi *pSpi() { return ((SercomSpi *)((byte *)SERCOM0 + ((byte *)SERCOM1 - (byte *)SERCOM0) * sercom)); }

public:
	enum MosiPad
	{
		MOSIPAD_Pad0,
		MOSIPAD_Pad1,
		MOSIPAD_Pad2,
		MOSIPAD_Pad3
	};

	enum ClientPad
	{
		CLIENTPAD_Pad0_MISO_Pad1_SCK_Pad2_SS,
		CLIENTPAD_Pad2_MISO_Pad3_SCK_Pad1_SS,
		CLIENTPAD_Pad3_MISO_Pad1_SCK_Pad2_SS,
		CLIENTPAD_Pad0_MISO_Pad3_SCK_Pad1_SS
	};

	enum SpiMode
	{
		SPIMODE_0,	// CPOL = 0, CPHA = 0
		SPIMODE_1,	// CPOL = 0, CPHA = 1
		SPIMODE_2,	// CPOL = 1, CPHA = 0
		SPIMODE_3,	// CPOL = 1, CPHA = 1
	};

public:
	static void SpiInit(MosiPad padMosi, ClientPad padMiso, SpiMode modeSpi)
	{
		SERCOM_SPI_CTRLA_Type	spiCtrlA;

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
		spiCtrlA.bit.MODE = 2;		// SPI client mode
		spiCtrlA.bit.DOPO = padMiso;
		spiCtrlA.bit.DIPO = padMosi;
		spiCtrlA.bit.CPHA = modeSpi & 1;
		spiCtrlA.bit.CPOL = modeSpi & 2;
		spiCtrlA.bit.IBON = 1;
		pSpi()->CTRLA.reg = spiCtrlA.reg;
		// Due to errata with SSDE, we can't set RXEN until after SPI is enabled
		pSpi()->CTRLB.reg = SERCOM_SPI_CTRLB_SSDE;
	}

	static void Enable()
	{
		pSpi()->CTRLA.bit.ENABLE = 1;
		// Due to errata with SSDE, we can't set RXEN until after SPI is enabled
		pSpi()->CTRLB.reg = SERCOM_SPI_CTRLB_SSDE | SERCOM_SPI_CTRLB_RXEN;
		NVIC_EnableIRQ((IRQn)(SERCOM0_IRQn + sercom));
	}

	static void Disable()
	{
		NVIC_DisableIRQ((IRQn)(SERCOM0_IRQn + sercom));
		pSpi()->CTRLA.bit.ENABLE = 0;
		// Due to errata with SSDE, we can't set RXEN until after SPI is enabled
		pSpi()->CTRLB.reg = SERCOM_SPI_CTRLB_SSDE;
	}
	
	static byte ReadByte()
	{
		return pSpi()->DATA.reg;
	}

	static void WriteByte(byte b)
	{
		pSpi()->DATA.reg = b;
	}
	
protected:
	static uint GetIntFlags()
	{
		return pSpi()->INTFLAG.reg;
	}

	static uint ClearIntFlags(uint flags)
	{
		return pSpi()->INTFLAG.reg = flags;
	}
	
	static void EnableInterrupts(uint flags)
	{
		pSpi()->INTENSET.reg = flags;
	}
	
	static void DisableInterrupts(uint flags)
	{
		pSpi()->INTENCLR.reg = flags;
	}

	static bool IsByteReady(uint flags)
	{
		return flags & SERCOM_SPI_INTFLAG_RXC;
	}

	static bool IsByteReady()
	{
		return pSpi()->INTFLAG.bit.RXC;
	}

	static bool CanSendByte(uint flags)
	{
		return flags & SERCOM_SPI_INTFLAG_DRE;
	}

	static bool CanSendByte()
	{
		return pSpi()->INTFLAG.bit.DRE;
	}

	static bool IsTxComplete(uint flags)
	{
		return flags & SERCOM_SPI_INTFLAG_TXC;
	}

	static bool IsTxComplete()
	{
		return pSpi()->INTFLAG.bit.TXC;
	}

	static void ClearTxComplete()
	{
		pSpi()->INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC;
	}

	static bool IsTxStart(uint flags)
	{
		return flags & SERCOM_SPI_INTFLAG_SSL;
	}

	static bool IsTxStart()
	{
		return pSpi()->INTFLAG.bit.SSL;
	}

	static void ClearTxStart()
	{
		pSpi()->INTFLAG.reg = SERCOM_SPI_INTFLAG_SSL;
	}

	static bool IsRxOverflow()
	{
		return pSpi()->INTFLAG.bit.ERROR;
	}

	static void ClearOverflow()
	{
		pSpi()->INTFLAG.reg = SERCOM_SPI_INTFLAG_ERROR;
	}
};

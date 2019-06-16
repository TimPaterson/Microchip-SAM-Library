//****************************************************************************
// UsartBuf.h
//
//****************************************************************************

#pragma once

#include <IoBuf\IoBuf.h>
#include <stdarg.h>

typedef void (*UsartHalfDuplexDriver_t)();
inline void UsartEmptyDriver() {}

#define SERCOM_ASYNC_BAUD(baud, clock)	((uint16_t)-(((uint32_t)(((uint64_t)baud * (65536 * 16 * 2)) / clock) + 1) / 2))

const int SERCOM_SIZE = 0x400;
const int SERCOM0_ADDR = SERCOM_BASE;
const int SERCOM1_ADDR = SERCOM_BASE + SERCOM_SIZE;
const int SERCOM2_ADDR = SERCOM_BASE + 2 * SERCOM_SIZE;
const int SERCOM3_ADDR = SERCOM_BASE + 3 * SERCOM_SIZE;
#define SERCOM_INDEX(sercom)	(((uint32_t)sercom - SERCOM_BASE) / SERCOM_SIZE)

// Declare and define full-duplex version
#define DECLARE_USART(usart, var, inbuf, outbuf) \
	typedef UsartBuf<usart##_ADDR, inbuf, outbuf> var##_t;

#define DECLARE_STDIO(usart, var, inbuf, outbuf) \
	typedef UsartBuf<usart##_ADDR, inbuf, outbuf> var##Base_t; \
	typedef StdIo<var##Base_t, outbuf> var##_t;	

#define DEFINE_USART(usart, var) \
	var##_t var; \
	void usart##_Handler() {var.UsartIsr();}

#define DEFINE_USART_TYPE(usart, var, type) \
	type var; \
	void usart##_Handler() {var.UsartIsr();}

// Declare and define half-duplex version. 
// These add calls to turn output driver on in WriteByte, and turn it off
// again in the Transmit Complete interrupt
//
#define DECLARE_USART_HALF(usart, var, inbuf, outbuf) \
	typedef UsartBufHalf<usart##_ADDR, inbuf, outbuf, &var##_DriverOff, &var##_DriverOn> var##_t;

#define DEFINE_USART_HALF(usart, var) \
	var##_t var; \
	void usart##_Handler() {var.UsartIsr();}
										
#define DEFINE_USART_HALF_TYPE(usart, var, type) \
	type var; \
	void usart##_Handler() {var.UsartIsr();}
										
//****************************************************************************

enum RxPad
{
	RXPAD_Pad0,
	RXPAD_Pad1,
	RXPAD_Pad2,
	RXPAD_Pad3
};

enum TxPad
{
	TXPAD_Pad0,
	TXPAD_Pad2,
	TXPAD_Pad0_RTS_Pad2_CTS_Pad3,
	TXPAD_Pad0_TE_Pad2
};

//****************************************************************************

class UsartBuf_t : public IoBuf
{
public:
	//************************************************************************
	// These methods override methods in IoBuf.h
	
protected:
	// We need our WriteByte to enable interrupts
	void WriteByteInline(BYTE b)
	{
		IoBuf::WriteByteInline(b);
		// Enable interrupts
		GetUsart()->INTENSET.reg = SERCOM_USART_INTENCLR_DRE;
	}

public:
	// We need our WriteByte to enable interrupts
	void WriteByte(BYTE b) NO_INLINE_ATTR
	{
		WriteByteInline(b);
	}

	// We need our own WriteString to use our own WriteByte
	void WriteString(const char *psz) NO_INLINE_ATTR
	{
		char	ch;
		
		for (;;)
		{
			ch = *psz++;
			if (ch == 0)
				return;
			if (ch == '\n')
				WriteByte('\r');
			WriteByte(ch);
		}
	}

	// We need our own WriteBytes to use our own WriteByte
	void WriteBytes(void *pv, BYTE cb) NO_INLINE_ATTR
	{
		BYTE	*pb;
		
		for (pb = (BYTE *)pv; cb > 0; cb--, pb++)
			WriteByte(*pb);
	}

	//************************************************************************
	// These are additional methods
	
	void Init(RxPad padRx, TxPad padTx)
	{
		SERCOM_USART_CTRLA_Type	serCtrlA;
		uint32_t	index;
		
		index = SERCOM_INDEX(GetUsart());
		
#if	defined(GCLK_PCHCTRL_GEN_GCLK0)
		// Enable clock
		MCLK->APBCMASK.reg |= 1 << (MCLK_APBCMASK_SERCOM0_Pos + index);
		
		// Clock it with GCLK0
		GCLK->PCHCTRL[SERCOM0_GCLK_ID_CORE + index].reg = GCLK_PCHCTRL_GEN_GCLK0 | GCLK_PCHCTRL_CHEN;
#else
		// Enable clock
		PM->APBCMASK.reg |= 1 << (PM_APBCMASK_SERCOM0_Pos + index);
		
		// Clock it with GCLK0
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | 
			(GCLK_CLKCTRL_ID_SERCOM0_CORE + index);
#endif
			
		// standard 8,N,1 parameters
		serCtrlA.reg = 0;
		serCtrlA.bit.DORD = 1;		// LSB first
		serCtrlA.bit.MODE = 1;		// internal clock
		serCtrlA.bit.RXPO = padRx;
		serCtrlA.bit.TXPO = padTx;
		GetUsart()->CTRLA.reg = serCtrlA.reg;
		GetUsart()->CTRLB.reg = SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;
	}
	
	void Enable()
	{
		NVIC_EnableIRQ((IRQn)(SERCOM0_IRQn + SERCOM_INDEX(GetUsart())));
		GetUsart()->INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
		GetUsart()->CTRLA.bit.ENABLE = 1;
	}
	
	void Disable()
	{
		GetUsart()->CTRLA.bit.ENABLE = 0;
		GetUsart()->INTENCLR.reg = SERCOM_USART_INTFLAG_RXC | SERCOM_USART_INTFLAG_DRE;
		NVIC_DisableIRQ((IRQn)(SERCOM0_IRQn + SERCOM_INDEX(GetUsart())));
	}

	uint32_t IsEnabled()
	{
		return GetUsart()->CTRLA.reg & SERCOM_USART_CTRLA_ENABLE;
	}

	void SetBaudReg(uint16_t rate)
	{
		GetUsart()->BAUD.reg = rate;
	}

	void SetBaudRate(uint32_t rate, uint32_t clock) NO_INLINE_ATTR
	{
		uint32_t	quo;
		uint32_t	quoBit;
		
		rate *= 16;		// actual clock frequency
		// Need 17-bit result of rate / clock
		for (quo = 0, quoBit = 1 << 16; quoBit != 0; quoBit >>= 1)
		{
			if (rate >= clock)
			{
				rate -= clock;
				quo |= quoBit;
			}
			rate <<= 1;
		}
		// Round
		if (rate >= clock)
			quo++;
		SetBaudReg((uint16_t)-quo);
	}
	
	void SetBaudRate(uint32_t rate)
	{
		SetBaudRate(rate, F_CPU);
	}
	
	void SetBaudRateConst(uint32_t rate)
	{
		SetBaudRateConst(rate, F_CPU);
	}
	
	void SetBaudRateConst(uint32_t rate, uint32_t clock)
	{
		SetBaudReg(SERCOM_ASYNC_BAUD(rate, clock));
	}

	BYTE IsXmitInProgress()
	{
		return GetUsart()->INTENCLR.reg & SERCOM_USART_INTFLAG_DRE;
	}

	SercomUsart *GetUsart()		{return (SercomUsart *)m_pvIO;}
};

//****************************************************************************

template <int pusart, BYTE cbRcvBuf, BYTE cbXmitBuf> class UsartBuf : public UsartBuf_t
{
public:
	static const int RcvBufSize = cbRcvBuf - 1;
	static const int XmitBufSize = cbXmitBuf - 1;

public:
	UsartBuf()
	{
		IoBuf::Init(cbRcvBuf, cbXmitBuf);
		m_pvIO = (SercomUsart *)pusart;
	}

	//************************************************************************
	// Interrupt service routine
	
public:
	void UsartIsr()
	{
		SercomUsart *pUsart = GetUsart();
		
		if (pUsart->INTFLAG.bit.RXC)
			ReceiveByte(pUsart->DATA.reg);
		
		if (pUsart->INTFLAG.bit.DRE && pUsart->INTENCLR.bit.DRE)
		{
			if (IsByteToSend())
				pUsart->DATA.reg = SendByte();
			else
				pUsart->INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
		}
	}

protected:
	BYTE	reserved[cbRcvBuf];
	BYTE	m_arbXmitBuf[cbXmitBuf];
};

//****************************************************************************
// Std I/O

template <class Base, int buf> class StdIo : public Base
{
public:
	void printf(const char *__fmt, ...)
	{
		va_list	args;
		va_start(args, __fmt);
		vsnprintf(m_archPrintBuf, sizeof m_archPrintBuf, __fmt, args);
		va_end(args);
		this->WriteString(m_archPrintBuf);
	}
	
protected:
	char m_archPrintBuf[buf];
};

//****************************************************************************
// Half-duplex version

template <int pusart, BYTE cbRcvBuf, BYTE cbXmitBuf, UsartHalfDuplexDriver_t driverOff, 
	UsartHalfDuplexDriver_t driverOn> class UsartBufHalf : public UsartBuf_t
{
public:
	UsartBufHalf()
	{
		IoBuf::Init(cbRcvBuf, cbXmitBuf);
		m_pvIO = (SercomUsart *)pusart;
	}

	// We need our WriteByte to enable output driver
	void WriteByte(BYTE b) NO_INLINE_ATTR
	{
		driverOn();
		UsartBuf_t::WriteByteInline(b);
	}

	// We need our own WriteString to use our own WriteByte
	void WriteString(const char *psz) NO_INLINE_ATTR
	{
		char	ch;
		
		for (;;)
		{
			ch = *psz++;
			if (ch == 0)
				return;
			if (ch == '\n')
				WriteByte('\r');
			WriteByte(ch);
		}
	}

	// We need our own WriteBytes to use our own WriteByte
	void WriteBytes(void *pv, BYTE cb) NO_INLINE_ATTR
	{
		BYTE	*pb;
		
		for (pb = (BYTE *)pv; cb > 0; cb--, pb++)
			WriteByte(*pb);
	}

	//************************************************************************
	// These methods override methods in UsartBuf_t
	
public:
	void Enable()
	{
		UsartBuf_t::Enable();
		UsartBuf_t::GetUsart()->INTENSET.reg = SERCOM_USART_INTFLAG_TXC;
	}
	
	//************************************************************************
	// Interrupt service routine
	
public:
	void UsartIsr()
	{
		SercomUsart *pUsart = UsartBuf_t::GetUsart();
		
		if (pUsart->INTFLAG.bit.RXC)
			UsartBuf_t::ReceiveByte(pUsart->DATA.reg);
		
		if (pUsart->INTFLAG.bit.DRE && pUsart->INTENCLR.bit.DRE)
		{
			if (UsartBuf_t::IsByteToSend())
			{
				pUsart->DATA.reg = UsartBuf_t::SendByte();
			}
			else
				pUsart->INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
		}
		
		if (pUsart->INTFLAG.bit.TXC)
		{
			driverOff();
			pUsart->INTFLAG.bit.TXC = 1;	//clear TXC
		}
	}

protected:
	BYTE	reserved[cbRcvBuf];
	BYTE	m_arbXmitBuf[cbXmitBuf];
};

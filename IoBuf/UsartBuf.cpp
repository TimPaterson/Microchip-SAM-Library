//****************************************************************************
// UsartBuf.cpp
//
// Created 7/6/2018 4:46:17 PM by Tim
//
//****************************************************************************


#include <standard.h>
#include <IoBuf\UsartBuf.h>


//****************************************************************************
// Implementation of UsartBuf_t

// We need our WriteByte to enable interrupts
void UsartBuf_t::WriteByte(BYTE b)
{
	WriteByteInline(b);
}

// We need our own WriteString to use our own WriteByte
void UsartBuf_t::WriteString(const char *psz)
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
void UsartBuf_t::WriteBytes(void *pv, BYTE cb)
{
	BYTE	*pb;
		
	for (pb = (BYTE *)pv; cb > 0; cb--, pb++)
		WriteByte(*pb);
}

void UsartBuf_t::SetBaudRate(uint32_t rate, uint32_t clock)
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

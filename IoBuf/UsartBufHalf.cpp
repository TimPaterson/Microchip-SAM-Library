//****************************************************************************
// UsartBufHalf.cpp
//
// Created 7/6/2018 4:57:49 PM by Tim
//
// IMPORTANT: This file must be included in a wrapper that defines
// UsartDriveOn	- function that enables transmit driver
//
//****************************************************************************


// We need our WriteByte to enable output driver
void UsartBufHalf_t::WriteByte(BYTE b)
{
	UsartDriveOn();
	UsartBuf_t::WriteByteInline(b);
}

// We need our own WriteString to use our own WriteByte
void UsartBufHalf_t::WriteString(const char *psz)
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
void UsartBufHalf_t::WriteBytes(void *pv, BYTE cb)
{
	BYTE	*pb;
		
	for (pb = (BYTE *)pv; cb > 0; cb--, pb++)
		WriteByte(*pb);
}

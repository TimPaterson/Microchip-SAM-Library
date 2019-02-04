//****************************************************************************
// IoBuf.cpp
//
// Created 7/7/2018 10:57:45 AM by Tim
//
//****************************************************************************


#include <standard.h>
#include <IoBuf/IoBuf.h>

void IoBuf::WriteByte(BYTE b)				{ WriteByteInline(b); }
void IoBuf::WriteBytes(void *pv, BYTE cb)	{ WriteBytesInline(pv, cb); }
BYTE IoBuf::ReadByte()						{ return ReadByteInline(); }
void IoBuf::ReadBytes(void *pv, BYTE cb)	{ ReadBytesInline(pv, cb); }
BYTE IoBuf::ReadByteWdr()					{ return ReadByteWdrInline(); }
BYTE IoBuf::PeekByte()						{ return PeekByteInline(); }
BYTE IoBuf::PeekByte(int off)				{ return PeekByteInline(off); }
int IoBuf::BytesCanWrite()					{ return BytesCanWriteInline(); }
int IoBuf::BytesCanRead()					{ return BytesCanReadInline(); }
bool IoBuf::CanWriteByte()					{ return CanWriteByteInline(); }
void IoBuf::DiscardReadBuf(BYTE bCnt)		{ DiscardReadBufInline(bCnt); }
void IoBuf::WriteString(const char *psz)	{ WriteStringInline(psz); }


/*
* Nvm.h
*
* Created: 2/19/2019 2:44:12 PM
* Author: Tim
*/

#pragma once


class Nvm
{
public:
	static bool IsReady()	{ return NVMCTRL->INTFLAG.bit.READY; }

public:
	static void NO_INLINE_ATTR WaitReady()
	{
		while (!IsReady());
	}

	//*********************************************************************
	// Assume NVM ready, inline and don't wait

	static void EraseRowReady()
	{
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
	}

	static void EraseRowReady(void *pv)
	{
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		EraseRowReady();
	}

	static void WritePageReady()
	{
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
	}

	static void WritePageReady(void *pv)
	{
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		WritePageReady();
	}

	//*********************************************************************
	// Wait for NVM ready, not inline

	static void NO_INLINE_ATTR EraseRow()
	{
		WaitReady();
		EraseRowReady();
	}

	static void NO_INLINE_ATTR EraseRow(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		EraseRowReady();
	}

	static void NO_INLINE_ATTR WritePage()
	{
		WaitReady();
		WritePageReady();
	}

	static void NO_INLINE_ATTR WritePage(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		WritePageReady();
	}

#ifdef NVMCTRL_CTRLA_CMD_RWWEEER

	//*********************************************************************
	// Assume NVM ready, inline and don't wait

	static void EraseRwweeRowReady()
	{
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEER;
	}

	static void EraseRwweeRowReady(void *pv)
	{
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		EraseRwweeRowReady();
	}

	static void WriteRwweePageReady()
	{
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEWP;
	}

	static void WriteRwweePageReady(void *pv)
	{
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		WriteRwweePageReady();
	}

	//*********************************************************************
	// Wait for NVM ready, not inline

	static void NO_INLINE_ATTR EraseRwweeRow()
	{
		WaitReady();
		EraseRwweeRowReady();
	}

	static void NO_INLINE_ATTR EraseRwweeRow(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		EraseRwweeRowReady();
	}

	static void NO_INLINE_ATTR WriteRwweePage()
	{
		WaitReady();
		WriteRwweePageReady();
	}

	static void NO_INLINE_ATTR WriteRwweePage(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (ulong)pv >> 1;
		WriteRwweePageReady();
	}

#else
	static void EraseRwweeRowReady()			{ EraseRowReady(); }
	static void EraseRwweeRowReady(void *pv)	{ EraseRowReady(pv); }
	static void WriteRwweePageReady()			{ WritePageReady(); }
	static void WriteRwweePageReady(void *pv)	{ WritePageReady(pv); }

	static void EraseRwweeRow()				{ EraseRow(); }
	static void EraseRwweeRow(void *pv)		{ EraseRow(pv); }
	static void WriteRwweePage()			{ WritePage(); }
	static void WriteRwweePage(void *pv)	{ WritePage(pv); }
#endif

	static void NO_INLINE_ATTR memcpy32(void *pvDest, void *pvSrc, int cb)
	{
		ulong	*puDest;
		ulong	*puSrc;
	
		puDest = (ulong *)pvDest;
		puSrc = (ulong *)pvSrc;
		for (cb /= sizeof(ulong); cb > 0; cb--)
			*puDest++ = *puSrc++;
	}
};

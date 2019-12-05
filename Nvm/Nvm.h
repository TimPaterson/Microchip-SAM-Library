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

	static void NO_INLINE_ATTR EraseRow()
	{
		WaitReady();
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
	}

	static void NO_INLINE_ATTR EraseRow(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (uint)pv >> 1;
		EraseRow();
	}

	static void NO_INLINE_ATTR WritePage()
	{
		WaitReady();
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
	}

	static void NO_INLINE_ATTR WritePage(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (uint)pv >> 1;
		WritePage();
	}

#ifdef NVMCTRL_CTRLA_CMD_RWWEEER

	static void NO_INLINE_ATTR EraseRwweeRow()
	{
		WaitReady();
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEER;
	}

	static void NO_INLINE_ATTR EraseRwweeRow(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (uint)pv >> 1;
		EraseRwweeRow();
	}

	static void NO_INLINE_ATTR WriteRwweePage()
	{
		WaitReady();
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEWP;
	}

	static void NO_INLINE_ATTR WriteRwweePage(void *pv)
	{
		WaitReady();
		NVMCTRL->ADDR.reg = (uint)pv >> 1;
		WriteRwweePage();
	}

#else
	static void EraseRwweeRow()				{ EraseRow(); }
	static void EraseRwweeRow(void *pv)		{ EraseRow(pv); }
	static void WriteRwweePage()			{ WritePage(); }
	static void WriteRwweePage(void *pv)	{ WritePage(pv); }
#endif

	static void NO_INLINE_ATTR memcpy32(void *pvDest, void *pvSrc, int cb)
	{
		uint	*puDest;
		uint	*puSrc;
	
		puDest = (uint *)pvDest;
		puSrc = (uint *)pvSrc;
		for (cb /= sizeof(uint); cb > 0; cb--)
			*puDest++ = *puSrc++;
	}
};

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
	static void WaitReady();
	static void EraseRow();
	static void EraseRow(void *pv);
	static void EraseRwweeRow();
	static void EraseRwweeRow(void *pv);
	static void WritePage();
	static void WritePage(void *pv);
	static void WriteRwweePage();
	static void WriteRwweePage(void *pv);
};

/*
* Nvm.cpp
*
* Created: 2/19/2019 2:44:12 PM
* Author: Tim
*/


#include <standard.h>
#include "Nvm.h"


void Nvm::WaitReady()
{
	while (!IsReady());
}
	
void Nvm::EraseRow()
{
	WaitReady();
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
}

void Nvm::EraseRow(void *pv)
{
	WaitReady();
	NVMCTRL->ADDR.reg = (uint)pv >> 1;
	EraseRow();
}

void Nvm::WritePage()
{
	WaitReady();
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
}

void Nvm::WritePage(void *pv)
{
	WaitReady();
	NVMCTRL->ADDR.reg = (uint)pv >> 1;
	WritePage();
}

#ifdef NVMCTRL_CTRLA_CMD_RWWEEER

void Nvm::EraseRwweeRow()
{
	WaitReady();
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEER;
}

void Nvm::EraseRwweeRow(void *pv)
{
	WaitReady();
	NVMCTRL->ADDR.reg = (uint)pv >> 1;
	EraseRwweeRow();
}

void Nvm::WriteRwweePage()
{
	WaitReady();
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEWP;
}

void Nvm::WriteRwweePage(void *pv)
{
	WaitReady();
	NVMCTRL->ADDR.reg = (uint)pv >> 1;
	WriteRwweePage();
}

#endif

/*
 * sam_spec.h
 *
 * Created: 8/18/2018 12:13:08 PM
 *  Author: Tim
 */ 

#pragma once

#if		defined(_SYSTEM_SAMC20_H_INCLUDED_)
	
#define __SAMC20__

#elif	defined(_SYSTEM_SAMC21_H_INCLUDED_)

#define __SAMC21__

#elif	defined(_SYSTEM_SAMD10_H_INCLUDED_)

#define __SAMD10__

#elif	defined(_SYSTEM_SAMD11_H_INCLUDED_)

#define __SAMD11__

#elif	defined(_SYSTEM_SAMD20_H_INCLUDED_)

#define __SAMD20__

#elif	defined(_SYSTEM_SAMD21_H_INCLUDED_)

#define __SAMD21__

#endif

//*********************************************************************
// Define SERCOM base

#if		defined(__SAMC20__) || defined(__SAMC21__)

#define SERCOM_BASE	(0x42000400)

#elif	defined(__SAMD10__) || defined(__SAMD11__) || defined(__SAMD20__) || defined(__SAMD21__)

#define SERCOM_BASE	(0x42000800)

#endif

//*********************************************************************
// Define calibration row

#if		defined(__SAMC20__) || defined(__SAMC21__)

struct NvmSofwareCal_t
{
	uint64_t	ADC0_BIASREFBUF:3;
	uint64_t	ADC0_BIASCOMP:3;
	uint64_t	ADC1_LINEARITY:3;
	uint64_t	ADC1_BIAS:3;
	uint64_t	OSC32K:7;
	uint64_t	CAL48M_5V:22;
	uint64_t	CAL48M_3V3:22;
};

#define NVM_SOFTWARE_CAL	((NvmSofwareCal_t *)NVMCTRL_OTP5)

#elif	defined(__SAMD10__) || defined(__SAMD11__) || defined(__SAMD20__) || defined(__SAMD21__)

struct NvmSofwareCal_t
{
	uint64_t	:27;
	uint64_t	ADC_LINEARITY:8;
	uint64_t	ADC_BIAS:3;
	uint64_t	OSC32K:7;
	uint64_t	:13;
	// WARNING!: the DFLL48M values are not set in Rev C devices!
	uint64_t	DFLL48M_COARSE:6;	// not set in Rev C
	uint64_t	DFLL48M_FINE:10;	// not set in Rev C
	uint64_t	:54;
};

#define NVM_SOFTWARE_CAL	((NvmSofwareCal_t *)NVMCTRL_OTP4)

#endif

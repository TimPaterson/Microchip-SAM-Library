//****************************************************************************
// TimerLib.h
//
// Created 4/5/2017 12:11:28 PM by Tim
//
// To use this class, two things must be defined before it is included:
// TimerClockFreq	- type double with the clock rate of the timer
// GetTickCount		- function returning Timer_t of tick count
//
//****************************************************************************

#pragma once

#ifndef	Timer_t
#define Timer_t	uint16_t
#endif


class Timer
{
public:
	INLINE_ATTR static void Delay(double sec)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay(sec));
	}
	
	INLINE_ATTR static void Delay_us(double us)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay_us(us));
	}
	
	INLINE_ATTR static void Delay_ms(double ms)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay_ms(ms));
	}
	
public:
	INLINE_ATTR Timer_t Start()						{ return m_uLastTime = GetTickCount(); }
	INLINE_ATTR Timer_t Start(Timer_t uVal)			{ return m_uLastTime = uVal; }
	INLINE_ATTR Timer_t GetStartTime()				{ return m_uLastTime; }

	INLINE_ATTR bool CheckDelay(double sec)			{ return CheckDelay_ticks(TicksFromSec(sec)); }
	INLINE_ATTR bool CheckDelay_us(double us)		{ return CheckDelay_ticks(TicksFromUs(us)); }
	INLINE_ATTR bool CheckDelay_ms(double ms)		{ return CheckDelay_ticks(TicksFromMs(ms)); }
	INLINE_ATTR bool CheckDelay_rate(double f)		{ return CheckDelay_ticks(TicksFromFreq(f)); }

	INLINE_ATTR bool CheckInterval(double sec)		{ return CheckInterval_ticks(TicksFromSec(sec)); }
	INLINE_ATTR bool CheckInterval_us(double us)	{ return CheckInterval_ticks(TicksFromUs(us)); }
	INLINE_ATTR bool CheckInterval_ms(double ms)	{ return CheckInterval_ticks(TicksFromMs(ms)); }
	INLINE_ATTR bool CheckInterval_rate(double f)	{ return CheckInterval_ticks(TicksFromFreq(f)); }
		
	// These are for action triggered by a missing event, like
	// a watchdog timer. fForceRestart signals event occurred.
	// Timer is not reset until event occurs.
	INLINE_ATTR bool CheckDelay(double sec, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromSec(sec), fForceRestart); }
	INLINE_ATTR bool CheckDelay_us(double us, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromUs(us), fForceRestart); }
	INLINE_ATTR bool CheckDelay_ms(double ms, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromMs(ms), fForceRestart); }
	INLINE_ATTR bool CheckDelay_rate(double f, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromFreq(f), fForceRestart); }

	// These are for action triggered by an event or a timeout.
	// fForceRestart signals if the event occurred.
	// Example: Receiver sends response after receiving a packet.
	// If incoming packet is late, response is sent anyway.
	// fForceRestart = did receive packet.
	INLINE_ATTR bool CheckInterval(double sec, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromSec(sec), fForceRestart); }
	INLINE_ATTR bool CheckInterval_us(double us, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromUs(us), fForceRestart); }
	INLINE_ATTR bool CheckInterval_ms(double ms, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromMs(ms), fForceRestart); }
	INLINE_ATTR bool CheckInterval_rate(double f, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromFreq(f), fForceRestart); }
			
public:
	// All of the above are now duplicated with the timer count passed in.
	// Thise helps when the timer count was also needed in the caller.
		
	INLINE_ATTR bool CheckDelay(double sec, Timer_t time)		{ return CheckDelay_ticks(TicksFromSec(sec), time); }
	INLINE_ATTR bool CheckDelay_us(double us, Timer_t time)		{ return CheckDelay_ticks(TicksFromUs(us), time); }
	INLINE_ATTR bool CheckDelay_ms(double ms, Timer_t time)		{ return CheckDelay_ticks(TicksFromMs(ms), time); }
	INLINE_ATTR bool CheckDelay_rate(double f, Timer_t time)	{ return CheckDelay_ticks(TicksFromFreq(f), time); }

	INLINE_ATTR bool CheckInterval(double sec, Timer_t time)	{ return CheckInterval_ticks(TicksFromSec(sec), time); }
	INLINE_ATTR bool CheckInterval_us(double us, Timer_t time)	{ return CheckInterval_ticks(TicksFromUs(us), time); }
	INLINE_ATTR bool CheckInterval_ms(double ms, Timer_t time)	{ return CheckInterval_ticks(TicksFromMs(ms), time); }
	INLINE_ATTR bool CheckInterval_rate(double f, Timer_t time){ return CheckInterval_ticks(TicksFromFreq(f), time); }
		
	// These are for action triggered by a missing event, like
	// a watchdog timer. fForceRestart signals event occurred.
	// Timer is not reset until event occurs.
	INLINE_ATTR bool CheckDelay(double sec, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromSec(sec), fForceRestart, time); }
	INLINE_ATTR bool CheckDelay_us(double us, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromUs(us), fForceRestart, time); }
	INLINE_ATTR bool CheckDelay_ms(double ms, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromMs(ms), fForceRestart, time); }
	INLINE_ATTR bool CheckDelay_rate(double f, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromFreq(f), fForceRestart, time); }

	// These are for action triggered by an event or a timeout.
	// fForceRestart signals if the event occurred.
	// Example: Receiver sends response after receiving a packet.
	// If incoming packet is late, response is sent anyway.
	// fForceRestart = did receive packet.
	INLINE_ATTR bool CheckInterval(double sec, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromSec(sec), fForceRestart, time); }
	INLINE_ATTR bool CheckInterval_us(double us, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromUs(us), fForceRestart, time); }
	INLINE_ATTR bool CheckInterval_ms(double ms, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromMs(ms), fForceRestart, time); }
	INLINE_ATTR bool CheckInterval_rate(double f, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromFreq(f), fForceRestart, time); }
			
public:
	// Calculate tick count from interval or rate
	INLINE_ATTR static Timer_t TicksFromUs(double us)	{ return lround(us * TimerClockFreq / 1000000); }
	INLINE_ATTR static Timer_t TicksFromMs(double ms)	{ return lround(ms * TimerClockFreq / 1000); }
	INLINE_ATTR static Timer_t TicksFromSec(double sec)	{ return lround(sec * TimerClockFreq); }
	INLINE_ATTR static Timer_t TicksFromFreq(double f)	{ return lround(TimerClockFreq / f); }

	// Integer versions suitable for use at runtime
	INLINE_ATTR static Timer_t TicksFromFreq(uint f)	{ return DIV_UINT_RND((uint)lround(TimerClockFreq), f); }
	INLINE_ATTR static Timer_t TicksFromFreq(int f)		{ return TicksFromFreq((uint)f); }
		
public:
	// Get the interval so far
	INLINE_ATTR Timer_t GetIntervalTicks()
	{
		return GetIntervalTicks(GetTickCount());
	}

	INLINE_ATTR Timer_t GetIntervalTicks(Timer_t time)
	{
		return (Timer_t)(time - m_uLastTime);
	}

public:
	INLINE_ATTR bool CheckDelay_ticks(Timer_t ticks)
	{
		return CheckDelay_ticks(ticks, GetTickCount());
	}
	
	INLINE_ATTR bool CheckDelay_ticks(Timer_t ticks, Timer_t time)
	{
		return (Timer_t)(time - m_uLastTime) >= ticks;
	}
	
	INLINE_ATTR bool CheckDelay_ticks(Timer_t ticks, bool fForceRestart)
	{
		return CheckDelay_ticks(ticks, fForceRestart, GetTickCount());
	}
	
	INLINE_ATTR bool CheckDelay_ticks(Timer_t ticks, bool fForceRestart, Timer_t time)
	{
		if (fForceRestart)
		{
			m_uLastTime = time;
			return false;
		}
		return (Timer_t)(time - m_uLastTime) >= ticks;
	}
	
	INLINE_ATTR bool CheckInterval_ticks(Timer_t ticks)
	{
		return CheckInterval_ticks(ticks, GetTickCount());
	}
	
	INLINE_ATTR bool CheckInterval_ticks(Timer_t ticks, Timer_t time)
	{
		if ((Timer_t)(time - m_uLastTime) >= ticks)
		{
			m_uLastTime += ticks;
			return true;
		}
		return false;
	}
	
	INLINE_ATTR bool CheckInterval_ticks(Timer_t ticks, bool fForceRestart)
	{
		return CheckInterval_ticks(ticks, fForceRestart, GetTickCount());
	}
	
	INLINE_ATTR bool CheckInterval_ticks(Timer_t ticks, bool fForceRestart, Timer_t time)
	{
		if (fForceRestart)
		{
			m_uLastTime = time;
			return true;
		}
		else if ((Timer_t)(time - m_uLastTime) >= ticks)
		{
			m_uLastTime += ticks;
			return true;
		}
		return false;
	}
	
protected:
	Timer_t	m_uLastTime;	
};
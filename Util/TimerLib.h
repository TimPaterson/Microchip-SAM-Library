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
	static void Delay(double sec)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay(sec));
	}
	
	static void Delay_us(double us)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay_us(us));
	}
	
	static void Delay_ms(double ms)
	{
		Timer	tmr;
		
		tmr.Start();
		while (!tmr.CheckDelay_ms(ms));
	}
	
public:
	Timer_t Start()						{ return m_uLastTime = GetTickCount(); }
	Timer_t Start(Timer_t uVal)			{ return m_uLastTime = uVal; }
	Timer_t GetStartTime()				{ return m_uLastTime; }

	bool CheckDelay(double sec)			{ return CheckDelay_ticks(TicksFromSec(sec)); }
	bool CheckDelay_us(double us)		{ return CheckDelay_ticks(TicksFromUs(us)); }
	bool CheckDelay_ms(double ms)		{ return CheckDelay_ticks(TicksFromMs(ms)); }
	bool CheckDelay_rate(double f)		{ return CheckDelay_ticks(TicksFromFreq(f)); }

	bool CheckInterval(double sec)		{ return CheckInterval_ticks(TicksFromSec(sec)); }
	bool CheckInterval_us(double us)	{ return CheckInterval_ticks(TicksFromUs(us)); }
	bool CheckInterval_ms(double ms)	{ return CheckInterval_ticks(TicksFromMs(ms)); }
	bool CheckInterval_rate(double f)	{ return CheckInterval_ticks(TicksFromFreq(f)); }
		
	// These are for action triggered by a missing event, like
	// a watchdog timer. fForceRestart signals event occurred.
	// Timer is not reset until event occurs.
	bool CheckDelay(double sec, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromSec(sec), fForceRestart); }
	bool CheckDelay_us(double us, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromUs(us), fForceRestart); }
	bool CheckDelay_ms(double ms, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromMs(ms), fForceRestart); }
	bool CheckDelay_rate(double f, bool fForceRestart)
		{ return CheckDelay_ticks(TicksFromFreq(f), fForceRestart); }

	// These are for action triggered by an event or a timeout.
	// fForceRestart signals if the event occurred.
	// Example: Receiver sends response after receiving a packet.
	// If incoming packet is late, response is sent anyway.
	// fForceRestart = did receive packet.
	bool CheckInterval(double sec, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromSec(sec), fForceRestart); }
	bool CheckInterval_us(double us, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromUs(us), fForceRestart); }
	bool CheckInterval_ms(double ms, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromMs(ms), fForceRestart); }
	bool CheckInterval_rate(double f, bool fForceRestart)
		{ return CheckInterval_ticks(TicksFromFreq(f), fForceRestart); }
			
public:
	// All of the above are now duplicated with the timer count passed in.
	// Thise helps when the timer count was also needed in the caller.
		
	bool CheckDelay(double sec, Timer_t time)		{ return CheckDelay_ticks(TicksFromSec(sec), time); }
	bool CheckDelay_us(double us, Timer_t time)		{ return CheckDelay_ticks(TicksFromUs(us), time); }
	bool CheckDelay_ms(double ms, Timer_t time)		{ return CheckDelay_ticks(TicksFromMs(ms), time); }
	bool CheckDelay_rate(double f, Timer_t time)	{ return CheckDelay_ticks(TicksFromFreq(f), time); }

	bool CheckInterval(double sec, Timer_t time)	{ return CheckInterval_ticks(TicksFromSec(sec), time); }
	bool CheckInterval_us(double us, Timer_t time)	{ return CheckInterval_ticks(TicksFromUs(us), time); }
	bool CheckInterval_ms(double ms, Timer_t time)	{ return CheckInterval_ticks(TicksFromMs(ms), time); }
	bool CheckInterval_rate(double f, Timer_t time){ return CheckInterval_ticks(TicksFromFreq(f), time); }
		
	// These are for action triggered by a missing event, like
	// a watchdog timer. fForceRestart signals event occurred.
	// Timer is not reset until event occurs.
	bool CheckDelay(double sec, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromSec(sec), fForceRestart, time); }
	bool CheckDelay_us(double us, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromUs(us), fForceRestart, time); }
	bool CheckDelay_ms(double ms, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromMs(ms), fForceRestart, time); }
	bool CheckDelay_rate(double f, bool fForceRestart, Timer_t time)
		{ return CheckDelay_ticks(TicksFromFreq(f), fForceRestart, time); }

	// These are for action triggered by an event or a timeout.
	// fForceRestart signals if the event occurred.
	// Example: Receiver sends response after receiving a packet.
	// If incoming packet is late, response is sent anyway.
	// fForceRestart = did receive packet.
	bool CheckInterval(double sec, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromSec(sec), fForceRestart, time); }
	bool CheckInterval_us(double us, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromUs(us), fForceRestart, time); }
	bool CheckInterval_ms(double ms, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromMs(ms), fForceRestart, time); }
	bool CheckInterval_rate(double f, bool fForceRestart, Timer_t time)
		{ return CheckInterval_ticks(TicksFromFreq(f), fForceRestart, time); }
			
public:
	// Calculate tick count from interval or rate
	static Timer_t TicksFromUs(double us)	{ return lround(us * TimerClockFreq / 1000000); }
	static Timer_t TicksFromMs(double ms)	{ return lround(ms * TimerClockFreq / 1000); }
	static Timer_t TicksFromSec(double sec)	{ return lround(sec * TimerClockFreq); }
	static Timer_t TicksFromFreq(double f)	{ return lround(TimerClockFreq / f); }

	// Integer versions suitable for use at runtime
	static Timer_t TicksFromFreq(uint f)	{ return DIV_UINT_RND((uint)lround(TimerClockFreq), f); }
	static Timer_t TicksFromFreq(int f)		{ return TicksFromFreq((uint)f); }
		
public:
	bool CheckDelay_ticks(Timer_t ticks)
	{
		return CheckDelay_ticks(ticks, GetTickCount());
	}
	
	bool CheckDelay_ticks(Timer_t ticks, Timer_t time)
	{
		return (Timer_t)(time - m_uLastTime) >= ticks;
	}
	
	bool CheckDelay_ticks(Timer_t ticks, bool fForceRestart)
	{
		return CheckDelay_ticks(ticks, fForceRestart, GetTickCount());
	}
	
	bool CheckDelay_ticks(Timer_t ticks, bool fForceRestart, Timer_t time)
	{
		if (fForceRestart)
		{
			m_uLastTime = time;
			return false;
		}
		return (Timer_t)(time - m_uLastTime) >= ticks;
	}
	
	bool CheckInterval_ticks(Timer_t ticks)
	{
		return CheckInterval_ticks(ticks, GetTickCount());
	}
	
	bool CheckInterval_ticks(Timer_t ticks, Timer_t time)
	{
		if ((Timer_t)(time - m_uLastTime) >= ticks)
		{
			m_uLastTime += ticks;
			return true;
		}
		return false;
	}
	
	bool CheckInterval_ticks(Timer_t ticks, bool fForceRestart)
	{
		return CheckInterval_ticks(ticks, fForceRestart, GetTickCount());
	}
	
	bool CheckInterval_ticks(Timer_t ticks, bool fForceRestart, Timer_t time)
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
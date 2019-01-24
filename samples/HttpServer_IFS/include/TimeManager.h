#ifndef __TIME_MANAGER_H
#define __TIME_MANAGER_H

#include "SystemClock.h"
#include "SolarCalculator.h"
#include "JsonConfigFile.h"
#include <functional>
#include "Timezone.h"

// Allow some drift on RTC so updates aren't too frequent
#define MAX_CLOCK_DRIFT 10 // in seconds

DECLARE_FSTR(ATTR_LATITUDE)
DECLARE_FSTR(ATTR_LONGITUDE)
DECLARE_FSTR(STR_BST)
DECLARE_FSTR(STR_GMT)

// Callback gets time adjustment in seconds as (tNew - tCur)
typedef std::function<void(int adjustSecs)> TimeChangeCallback;

// Type of absolute timer
enum TimeType {
	time_invalid,
	// Absolute time, given in seconds
	time_absolute,
	// The current date/time
	time_now,
	// Dawn
	time_sunrise,
	// Dusk
	time_sunset
};

class TimeManager : public Timezone
{
public:
	void onChange(TimeChangeCallback callback)
	{
		m_onChange = callback;
	}

	void configure(const JsonObject& config);
	void update(time_t timeUTC);

	bool timeValid()
	{
		return m_timeValid;
	}

	time_t getTime(TimeType tt, int offset_secs);

	String timeStr(time_t t);

	String timeStr(TimeType tt, int offset_secs)
	{
		return timeStr(getTime(tt, offset_secs));
	}

	String nowStr()
	{
		return timeStr(SystemClock.now(eTZ_Local));
	}

private:
	// RTC is volatile and requires setting at reset
	bool m_timeValid = false;
	//
	SolarCalculator m_solarCalc;
	//
	TimeChangeCallback m_onChange = nullptr;
};

extern TimeManager timeManager;

#endif // __TIME_MANAGER_H

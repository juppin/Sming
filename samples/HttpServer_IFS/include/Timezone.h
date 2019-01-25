/*----------------------------------------------------------------------*
 * Arduino Timezone Library                                             *
 * Jack Christensen Mar 2012                                            *
 *                                                                      *
 * Arduino Timezone Library Copyright (C) 2018 by Jack Christensen and  *
 * licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html   *
 *----------------------------------------------------------------------*/

#ifndef __DAYLIGHT_H
#define __DAYLIGHT_H

#include "DateTime.h"

// convenient constants for TimeChangeRules
enum __attribute__((packed)) week_t { Last, First, Second, Third, Fourth };

enum __attribute__((packed)) dow_t { Sun = 0, Mon, Tue, Wed, Thu, Fri, Sat };

enum __attribute__((packed)) month_t { Jan = 1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
struct TimeChangeRule {
	char tag[6];   //
	week_t week;   // First, Second, Third, Fourth, or Last week of the month
	dow_t dow;	 // day of week, 0=Sun
	month_t month; // 1=Jan
	uint8_t hour;  // 0-23
	int offset;	// offset from UTC in minutes
};

class Timezone
{
public:
	Timezone()
	{
	}

	Timezone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart)
	{
		init(dstStart, stdStart);
	}

	void init(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart);

	time_t toLocal(time_t utc, const TimeChangeRule** p_tcr = nullptr);
	time_t toUTC(time_t local);
	bool utcIsDST(time_t utc);
	bool locIsDST(time_t local);

	const char* timeTag(bool isDst)
	{
		return isDst ? dstRule.tag : stdRule.tag;
	}

	const char* utcTimeTag(time_t utc)
	{
		return timeTag(utcIsDST(utc));
	}

	const char* localTimeTag(time_t local)
	{
		return timeTag(locIsDST(local));
	}

private:
	void calcTimeChanges(int yr);
	time_t toTime_t(TimeChangeRule r, int yr);

private:
	// rule for start of dst or summer time for any year
	TimeChangeRule dstRule = {"DST", First, Sun, Jan, 1, 0};
	// rule for start of standard time for any year
	TimeChangeRule stdRule = {"UTC", First, Sun, Jan, 1, 0};
	// dst start for given/current year, given in UTC
	time_t dstStartUTC = 0;
	// std time start for given/current year, given in UTC
	time_t stdStartUTC = 0;
	// dst start for given/current year, given in local time
	time_t dstStartLoc = 0;
	// std time start for given/current year, given in local time
	time_t stdStartLoc = 0;
};

#endif // __DAYLIGHT_H

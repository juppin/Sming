/*
 * timemgmt.cpp
 *
 *  Created on: 25 Jun 2018
 *      Author: mikee47
 */

#include "TimeManager.h"
#include "Timezone.h"

DEFINE_FSTR(ATTR_LATITUDE, "latitude")
DEFINE_FSTR(ATTR_LONGITUDE, "longitude")
DEFINE_FSTR(STR_BST, "BST")
DEFINE_FSTR(STR_GMT, "GMT")

static const TimeChangeRule timechangeRules[] PROGMEM = {
	// United Kingdom (London, Belfast)
	{"BST", Last, Sun, Mar, 1, 60},
	{"GMT", Last, Sun, Oct, 2, 0}};

TimeManager timeManager;

static bool findTimechangeRule(const String& tag, TimeChangeRule& rule)
{
	for(unsigned i = 0; i < ARRAY_SIZE(timechangeRules); ++i) {
		TimeChangeRule r;
		memcpy_P(&r, &timechangeRules[i], sizeof(TimeChangeRule));
		if(tag == r.tag) {
			rule = r;
			return true;
		}
	}

	debug_w("Timechange rule '%s' not found", tag.c_str());
	return false;
}

void TimeManager::configure(const JsonObject& config)
{
	SolarRef& ref = solarCalc.getRef();
	CONFIG_READ(config, ATTR_LATITUDE, ref.latitude);
	CONFIG_READ(config, ATTR_LONGITUDE, ref.longitude);

	// Get timezone info.
	TimeChangeRule bst, gmt;
	findTimechangeRule(STR_BST, bst);
	findTimechangeRule(STR_GMT, gmt);
	init(bst, gmt);
}

void TimeManager::update(time_t timeUTC)
{
	time_t tNew = toLocal(timeUTC);
	time_t tCur = SystemClock.now(eTZ_Local);
	debug_i("TimeManager::update(%s), current: %s", timeStr(tNew).c_str(), timeStr(tCur).c_str());

	// Update system clock if it's drifted sufficiently
	if(!timeIsValid || (abs(tCur - tNew) > MAX_CLOCK_DRIFT)) {
		// Time zone difference also accounts for DST
		int diff = tNew - timeUTC;
		debug_i("TZ diff = %d secs", diff);
		SystemClock.setTimeZoneOffset(diff);
		SystemClock.setTime(tNew, eTZ_Local);
		timeIsValid = true;

#if DEBUG_BUILD

		debug_i("Now:     %s", timeStr(time_now, 0).c_str());
		debug_i("Sunrise: %s", timeStr(time_sunrise, 0).c_str());
		debug_i("Sunset:  %s", timeStr(time_sunset, 0).c_str());

#endif

		if(changeCallback) {
			changeCallback(tNew - tCur);
		}
	}
}

/*
 * All calculated in local time:
 *
 * time_now:      The current time, plus offset_secs
 * time_absolute: Today's date plus offset_secs since midnight
 * time_sunrise:  The next sunrise, plus offset_secs
 * time_sunset:   The next sunset, plus offset_secs
 */
time_t TimeManager::getTime(TimeType timetype, int offset_secs)
{
	time_t tNow = SystemClock.now(eTZ_Local);

	if(timetype == time_now) {
		return tNow + offset_secs;
	}

	DateTime dt(tNow);
	dt.Hour = 0;
	dt.Minute = 0;
	dt.Second = 0;

	if(timetype == time_absolute) {
		time_t t = dt + offset_secs;
		// If time has already passed, then make it tomorrow.
		if(tNow > t) {
			t += SECS_PER_DAY;
		}
		return t;
	}

	if(timetype == time_sunrise || timetype == time_sunset) {
		/*
		 * We use the y/m/d from local time for sunrise/sunset calculations, and the solar calculator
		 * returns the time from midnight in UTC for that day. We therefore need to adjust this
		 * to account for timezone and daylight savings.
		 */
		offset_secs += SECS_PER_MIN * solarCalc.sunRiseSet(timetype == time_sunrise, dt.Year, dt.Month + 1, dt.Day);
		//    return toLocal(t + SECS_PER_MIN * m_solarCalc.sunrise(dt.Year, dt.Month + 1, dt.Day));

		time_t t = toLocal(dt + offset_secs);
		// If time has already passed, then make it tomorrow
		if(t < tNow) {
			t = toLocal(dt + offset_secs + SECS_PER_DAY);
		}
		return t;
	}

	// Unknown/invalid
	return 0;
}

String TimeManager::timeStr(time_t t)
{
	return DateTime(t).toFullDateTimeString() + String(' ') + localTimeTag(t);
}

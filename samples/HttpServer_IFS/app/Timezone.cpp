/*----------------------------------------------------------------------*
 * Arduino Timezone Library                       *
 * Jack Christensen Mar 2012                      *
 *                                    *
 * Arduino Timezone Library Copyright (C) 2018 by Jack Christensen and  *
 * licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html   *
 *----------------------------------------------------------------------*/

#include "Timezone.h"

static int year(time_t t)
{
	return DateTime(t).Year;
}

/*
 * Convert the given UTC time to local time, standard or daylight time,
 * as appropriate, and return a pointer to the time change rule used to
 * do the conversion. The caller must take care not to alter this rule.
 */
time_t Timezone::toLocal(time_t utc, const TimeChangeRule** p_tcr)
{
	// recalculate the time change points if needed
	int y = year(utc);
	if(y != year(m_dstUTC))
		calcTimeChanges(y);

	const TimeChangeRule& tcr = utcIsDST(utc) ? m_dst : m_std;

	if(p_tcr)
		*p_tcr = &tcr;

	return utc + (tcr.offset * SECS_PER_MIN);
}

/*
 * Convert the given local time to UTC time.
 *
 * WARNING:
 * This function is provided for completeness, but should seldom be
 * needed and should be used sparingly and carefully.
 *
 * Ambiguous situations occur after the Standard-to-DST and the
 * DST-to-Standard time transitions. When changing to DST, there is
 * one hour of local time that does not exist, since the clock moves
 * forward one hour. Similarly, when changing to standard time, there
 * is one hour of local times that occur twice since the clock moves
 * back one hour.
 *
 * This function does not test whether it is passed an erroneous time
 * value during the Local -> DST transition that does not exist.
 * If passed such a time, an incorrect UTC time value will be returned.
 *
 * If passed a local time value during the DST -> Local transition
 * that occurs twice, it will be treated as the earlier time, i.e.
 * the time that occurs before the transistion.
 *
 * Calling this function with local times during a transition interval
 * should be avoided.
 */
time_t Timezone::toUTC(time_t local)
{
	// recalculate the time change points if needed
	int y = year(local);
	if(y != year(m_dstLoc))
		calcTimeChanges(y);

	return local - (locIsDST(local) ? m_dst.offset : m_std.offset) * SECS_PER_MIN;
}

/*
 * Determine whether the given UTC time_t is within the DST interval or the Standard time interval.
 */
bool Timezone::utcIsDST(time_t utc)
{
	// recalculate the time change points if needed
	int y = year(utc);
	if(y != year(m_dstUTC))
		calcTimeChanges(y);

	// daylight time not observed in this tz
	if(m_stdUTC == m_dstUTC)
		return false;

	// northern hemisphere
	if(m_stdUTC > m_dstUTC)
		return (utc >= m_dstUTC) && (utc < m_stdUTC);

	// southern hemisphere
	return (utc >= m_dstUTC) || (utc < m_stdUTC);
}

/*
 * Determine whether the given Local time_t is within the DST interval
 * or the Standard time interval.
 */
bool Timezone::locIsDST(time_t local)
{
	// recalculate the time change points if needed
	int y = year(local);
	if(y != year(m_dstLoc))
		calcTimeChanges(y);

	// daylight time not observed in this tz
	if(m_stdUTC == m_dstUTC)
		return false;

	// northern hemisphere
	if(m_stdLoc > m_dstLoc)
		return (local >= m_dstLoc && local < m_stdLoc);

	// southern hemisphere
	return (local >= m_dstLoc) || (local < m_stdLoc);
}

/*
 * Calculate the DST and standard time change points for the given
 * given year as local and UTC time_t values.
 */
void Timezone::calcTimeChanges(int yr)
{
	m_dstLoc = toTime_t(m_dst, yr);
	m_stdLoc = toTime_t(m_std, yr);
	m_dstUTC = m_dstLoc - m_std.offset * SECS_PER_MIN;
	m_stdUTC = m_stdLoc - m_dst.offset * SECS_PER_MIN;
}

/*
 * Convert the given time change rule to a time_t value for the given year.
 */
time_t Timezone::toTime_t(TimeChangeRule r, int yr)
{
	// working copies of r.month and r.week which we may adjust
	uint8_t m = r.month;
	uint8_t w = r.week;

	// is this a "Last week" rule?
	if(w == week_t::Last) {
		// yes, for "Last", go to the next month
		if(++m > month_t::Dec) {
			m = month_t::Jan;
			++yr;
		}
		// and treat as first week of next month, subtract 7 days later
		w = week_t::First;
	}

	// calculate first day of the month, or for "Last" rules, first day of the next month
	DateTime dt;
	dt.Hour = r.hour;
	dt.Minute = 0;
	dt.Second = 0;
	dt.Day = 1;
	dt.Month = m - month_t::Jan; // Zero-based
	dt.Year = yr;
	time_t t = dt;

	// add offset from the first of the month to r.dow, and offset for the given week
	t += ((r.dow - dayOfWeek(t) + 7) % 7 + (w - 1) * 7) * SECS_PER_DAY;
	// back up a week if this is a "Last" rule
	if(r.week == 0)
		t -= 7 * SECS_PER_DAY;

	return t;
}

void Timezone::init(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart)
{
	m_dst = dstStart;
	m_std = stdStart;
	// force calcTimeChanges() at next conversion call
	m_dstUTC = 0;
	m_stdUTC = 0;
	m_dstLoc = 0;
	m_stdLoc = 0;
}

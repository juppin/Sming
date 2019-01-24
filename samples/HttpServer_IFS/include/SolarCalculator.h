/*
 * Solar calculator.
 *
 * Class to obtain apparent time of sunrise and sunset.
 *
 *
 * 'Original' code created by DM Kishi <dm.kishi@gmail.com> on 2017-02-01.
 *  <https://github.com/dmkishi/Dusk2Dawn>
 *
 *  This is straight port of the code used by
 *    https://www.esrl.noaa.gov/gmd/grad/solcalc/
 *
 *  Javascript reference:
 *    https://www.esrl.noaa.gov/gmd/grad/solcalc/main.js
 *
 *
 *  Note: Months are 1-based
 *
 */

#ifndef __SOLAR_CALCULATOR_H
#define __SOLAR_CALCULATOR_H

#include "DateTime.h"

struct SolarRef {
	float latitude;
	float longitude;
};

class SolarCalculator
{
public:
	SolarCalculator()
	{
	}

	SolarCalculator(const SolarRef& ref) : m_ref(ref)
	{
	}

	SolarRef& ref()
	{
		return m_ref;
	}

	void setRef(const SolarRef& ref)
	{
		m_ref = ref;
	}

	int sunRiseSet(bool isRise, int y, int m, int d);

	int sunrise(int y, int m, int d)
	{
		return sunRiseSet(true, y, m, d);
	}

	int sunset(int y, int m, int d)
	{
		return sunRiseSet(false, y, m, d);
	}

private:
	/*
	 * Though most time zones are offset by whole hours, there are a few zones
	 * offset by 30 or 45 minutes, so the argument must be declared as a float.
	 *
	 * Royal Observatory, Greenwich, seems an appropriate default setting
	 */
	SolarRef m_ref = {51.4769, 0.0005};
};

#endif // __SOLAR_CALCULATOR_H

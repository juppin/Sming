#include "SolarCalculator.h"

#ifdef SMING_INCLUDED

#include <WiringFrameworkDependencies.h>
#define M_PI PI

#else

#include <math.h>

#endif

static float equationOfTime(float t);
static float meanObliquityOfEcliptic(float t);
static float eccentricityEarthOrbit(float t);
static float sunDeclination(float t);
static float sunApparentLong(float t);
static float sunTrueLong(float t);
static float sunEqOfCenter(float t);
static float hourAngleSunrise(float lat, float solarDec);
static float obliquityCorrection(float t);
static float geomMeanLongSun(float t);
static float geomMeanAnomalySun(float t);
static float jDay(int year, int month, int day);
static float fractionOfCentury(float jd);
static float radToDeg(float rad);
static float degToRad(float deg);

/* ---------------------------- EQUATION OF TIME ---------------------------- */
/* The difference between mean solar time (as shown by clocks) and apparent
 * solar time (indicated by sundials), which varies with the time of year.
 */
float equationOfTime(float t)
{
	float epsilon = obliquityCorrection(t);
	float l0 = geomMeanLongSun(t);
	float e = eccentricityEarthOrbit(t);
	float m = geomMeanAnomalySun(t);

	float y = tan(degToRad(epsilon) / 2);
	y *= y;

	float sin2l0 = sin(2.0 * degToRad(l0));
	float sinm = sin(degToRad(m));
	float cos2l0 = cos(2.0 * degToRad(l0));
	float sin4l0 = sin(4.0 * degToRad(l0));
	float sin2m = sin(2.0 * degToRad(m));

	float Etime =
		y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0 - 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;
	return radToDeg(Etime) * 4.0; // in minutes of time
}

/* Obliquity of the ecliptic is the term used by astronomers for the inclination
 * of Earth's equator with respect to the ecliptic, or of Earth's rotation axis
 * to a perpendicular to the ecliptic.
 */
float meanObliquityOfEcliptic(float t)
{
	float seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
	// in degrees
	return 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
}

float eccentricityEarthOrbit(float t)
{
	// e is unitless
	return 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
}

/* --------------------------- SOLAR DECLINATION ---------------------------- */
float sunDeclination(float t)
{
	float e = obliquityCorrection(t);
	float lambda = sunApparentLong(t);
	float sint = sin(degToRad(e)) * sin(degToRad(lambda));
	// theta in degrees
	return radToDeg(asin(sint));
}

float sunApparentLong(float t)
{
	float omega = 125.04 - 1934.136 * t;
	// lambda in degrees
	return sunTrueLong(t) - 0.00569 - 0.00478 * sin(degToRad(omega));
}

float sunTrueLong(float t)
{
	// in degrees
	return geomMeanLongSun(t) + sunEqOfCenter(t);
}

float sunEqOfCenter(float t)
{
	float m = geomMeanAnomalySun(t);
	float mrad = degToRad(m);
	float sinm = sin(mrad);
	float sin2m = sin(mrad * 2);
	float sin3m = sin(mrad * 3);
	// in degrees
	return sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
}

/* ------------------------------- HOUR ANGLE ------------------------------- */
float hourAngleSunrise(float lat, float solarDec)
{
	float latRad = degToRad(lat);
	float sdRad = degToRad(solarDec);
	float HAarg = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));
	// HA in radians (for sunset, use -HA)
	return acos(HAarg);
}

/* ---------------------------- SHARED FUNCTIONS ---------------------------- */
float obliquityCorrection(float t)
{
	float omega = 125.04 - 1934.136 * t;
	// in degrees
	return meanObliquityOfEcliptic(t) + 0.00256 * cos(degToRad(omega));
}

float geomMeanLongSun(float t)
{
	float L0 = 280.46646 + t * (36000.76983 + t * 0.0003032);

	// Get value between 0 and 360 degrees
	while(L0 > 360)
		L0 -= 360;
	while(L0 < 0)
		L0 += 360;

	return L0;
}

float geomMeanAnomalySun(float t)
{
	// in degrees
	return 357.52911 + t * (35999.05029 - 0.0001537 * t);
}

/* --------------------------- UTILITY FUNCTIONS ---------------------------- */

/*
 * Convert Gregorian calendar date to Julian Day.
 *
 * @param year      Full year
 * @param month     Month, 1 - 12
 * @param day       Day of month, 1-31
 */
float jDay(int year, int month, int day)
{
	if(month <= 2) {
		year -= 1;
		month += 12;
	}

	int A = floor(year / 100);
	int B = 2 - A + floor(A / 4);
	return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + B - 1524.5;
}

/* Return fraction of time elapsed this century, AD 2000–2100.
 *
 * NOTE: 2,451,545 was the Julian day starting at noon UTC on 1 January AD 2000.
 *       36,525 is a Julian century.
 */
float fractionOfCentury(float jd)
{
	return (jd - 2451545.0) / 36525.0;
}

float radToDeg(float rad)
{
	return 180.0 * rad / M_PI;
}

float degToRad(float deg)
{
	return M_PI * deg / 180.0;
}

float sunRiseSetUTC(bool isRise, float jday, float latitude, float longitude)
{
	float t = fractionOfCentury(jday);
	float eqTime = equationOfTime(t);
	float solarDec = sunDeclination(t);
	float hourAngle = hourAngleSunrise(latitude, solarDec);
	float delta = longitude + radToDeg(isRise ? hourAngle : -hourAngle);
	return 720.0 - (4.0 * delta) - eqTime; // in minutes
}

/******************************************************************************/
/*                                  PRIVATE                                   */
/******************************************************************************/

/*
 * Calculate a sunrise or sunset figure for a given day.
 *
 * @param isRise  True for sunrise, False for sunset
 * @param y       Absolute year
 * @param m       Month number (1 - 12)
 * @param d       Day of month (1 - 31)
 *
 * @returns minutes since midnight, -1 if there is no sunrise/sunset
 */
int SolarCalculator::sunRiseSet(bool isRise, int y, int m, int d)
{
	float jday = jDay(y, m, d);
	float timeUTC = sunRiseSetUTC(isRise, jday, m_ref.latitude, m_ref.longitude);

	// Advance the calculated time by a fraction of itself; why?
	float newTimeUTC = sunRiseSetUTC(isRise, jday + timeUTC / 1440.0, m_ref.latitude, m_ref.longitude);

	// Check there is a sunrise or sunset, e.g. in the (ant)arctic.
	if(std::isnan(newTimeUTC))
		return -1;

	//  printf("UTC = %f\n", newTimeUTC);

	return round(newTimeUTC);
}

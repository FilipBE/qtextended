/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

/* This software is based on public domain software as described
 * in the notices below, however for licensing simplicity, THIS copy
 * is NOT in the public domain, and may include modifications that are
 * only available as described above.
 *
 * For purely public domain versions of this file, please seek older
 * copies. Otherwise, the above notices are in force.
 */

/*
 * Sun clock.  X11 version by John Mackin.
 *
 * This program was derived from, and is still in part identical to, the
 * Suntools Sun clock program whose author's comment appears immediately
 * below.  Please preserve both notices.
 *
 * The X11R3/4 version of this program was written by John Mackin, at the
 * Basser Department of Computer Science, University of Sydney, Sydney,
 * New South Wales, Australia; <john@cs.su.oz.AU>.  This program, like
 * the one it was derived from, is in the public domain: `Love is the
 * law, love under will.'
 */

/*

        Sun clock

        Designed and implemented by John Walker in November of 1988.

        Version for the Sun Workstation.

    The algorithm used to calculate the position of the Sun is given in
    Chapter 18 of:

    "Astronomical  Formulae for Calculators" by Jean Meeus, Third Edition,
    Richmond: Willmann-Bell, 1985.  This book can be obtained from:

       Willmann-Bell
       P.O. Box 35025
       Richmond, VA  23235
       USA
       Phone: (804) 320-7016

    This program was written by:

       John Walker
       Autodesk, Inc.
       2320 Marinship Way
       Sausalito, CA  94965
       USA
       Fax:   (415) 389-9418
       Voice: (415) 332-2344 Ext. 2829
       Usenet: {sun,well,uunet}!acad!kelvin
           or: kelvin@acad.uu.net

    modified for interactive maps by

        Stephen Martin
        Fujitsu Systems Business of Canada
        smartin@fujitsu.ca

    This  program is in the public domain: "Do what thou wilt shall be the
    whole of the law".  I'd appreciate  receiving  any  bug  fixes  and/or
    enhancements,  which  I'll  incorporate  in  future  versions  of  the
    program.  Please leave the original attribution information intact  so
    that credit and blame may be properly apportioned.

    Revision history:

        1.0  12/21/89  Initial version.
              8/24/89  Finally got around to submitting.

        1.1   8/31/94  Version with interactive map.
        1.2  10/12/94  Fixes for HP and Solaris, new icon bitmap
        1.3  11/01/94  Timezone now shown in icon
        1.4  03/29/98  Fixed city drawing, added icon animation

*/

#include "qworldmap_sun_p.h"
#include <math.h>
#include <time.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef E
#define E 2.7182818284590452354
#endif

#define abs(x) ((x) < 0 ? (-(x)) : x)                     /* Absolute value */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))       /* Extract sign */
#define dtr(x) ((x) * (PI / 180.0))                       /* Degree->Radian */
#define rtd(x) ((x) / (PI / 180.0))                       /* Radian->Degree */
#define fixangle(a) ((a) - 360.0 * ((int)floor((a) / 360.0)))  /* Fix angle       */

#define TERMINC  100               // Circle segments for terminator

/* Project illuminated area on the map. */

void
projillum(short *wtab, int xdots, int ydots, double dec)
{
    int i, ftf = 1, ilon, ilat, lilon = 0, lilat = 0, xt;
    double m, x, y, z, th, lon, lat, s, c;

    // Clear unoccupied cells in width table
    for (i = 0; i < ydots; i++)
        wtab[i] = -1;

    // Build transformation for declination
    s = sin(-dtr(dec));
    c = cos(-dtr(dec));

    // Increment over a semicircle of illumination
    for (th = -(PI / 2); th <= PI / 2 + 0.001; th += PI / TERMINC) {
        // Transform the point through the declination rotation.
        x = -s * sin(th);
        y = cos(th);
        z = c * sin(th);

        // Transform the resulting co-ordinate through the
        // map projection to obtain screen co-ordinates.
        lon = (y == 0 && x == 0) ? 0.0 : rtd(atan2(y, x));
        lat = rtd(asin(z));

        ilat = int(ydots - (lat + 90) * (ydots / 180.0));
        ilon = int(lon * (xdots / 360.0));

        if (ftf) {
            // First time.  Just save start co-ordinate.
            lilon = ilon;
            lilat = ilat;
            ftf = 0;
        } else {
            // Trace out the line and set the width table.
            if (lilat == ilat) {
                wtab[(ydots - 1) - ilat] = ilon == 0 ? 1 : ilon;
            } else {
                m = ((double) (ilon - lilon)) / (ilat - lilat);
                for (i = lilat; i != ilat; i += sgn(ilat - lilat)) {
                    xt = int(lilon + (int)floor((m * (i - lilat)) + 0.5));
                    wtab[(ydots - 1) - i] = xt == 0 ? 1 : xt;
                }
            }
            lilon = ilon;
            lilat = ilat;
        }
    }

    // Now tweak the widths to generate full illumination for the correct pole.
    if (dec < 0.0) {
        ilat = ydots - 1;
        lilat = -1;
    } else {
        ilat = 0;
        lilat = 1;
    }

    for (i = ilat; i != ydots / 2; i += lilat) {
        if (wtab[i] != -1) {
            while (1) {
                wtab[i] = xdots / 2;
                if (i == ilat)
                    break;
                i -= lilat;
            }
            break;
        }
    }
}

/* Convert internal GMT date and time to Julian day and fraction. */

long
jdate(struct tm* t)
{
    long c, m, y;

    y = t->tm_year + 1900;
    m = t->tm_mon + 1;
    if (m > 2)
        m = m - 3;
    else {
        m = m + 9;
        y--;
    }
    c = y / 100L;              /* Compute century */
    y -= 100L * c;
    return t->tm_mday + (c * 146097L) / 4 + (y * 1461L) / 4 +
        (m * 153L + 2) / 5 + 1721119L;
}

/* Convert internal GMT date and time to astronomical Julian time
   (i.e. Julian date plus day fraction, expressed as a double). */

double
jtime(struct tm* t)
{
    return (jdate(t) - 0.5) +
           (((long) t->tm_sec) +
             60L * (t->tm_min + 60L * t->tm_hour)) / 86400.0;
}

/* Solve the equation of Kepler. */

double
kepler(double m, double ecc)
{
    double e, delta;
#define EPSILON 1E-6

    e = m = dtr(m);
    do {
        delta = e - ecc * sin(e) - m;
        e -= delta / (1 - ecc * cos(e));
    } while (fabs(delta) > EPSILON);
    return e;
}

/* Calculate position of the Sun. JD is the Julian date of the instant for
   which the position is desired and APPARENT should be nonzero if the
   apparent position (corrected for nutation and aberration) is desired.
   The Sun's co-ordinates are returned in RA and DEC, both specified in
   degrees (divide RA by 15 to obtain hours).  The radius vector to the Sun
   in astronomical units is returned in RV and the Sun's longitude (true
   or apparent, as desired) is returned as degrees in SLONG. */

void
sunpos(double jd, int apparent, double *ra, double *dec, double *rv, double* slong)
{
    double t, t2, t3, l, m, e, ea, v, theta, omega, eps;

    // Time, in Julian centuries of 36525 ephemeris days,
    // measured from the epoch 1900 January 0.5 ET.
    t = (jd - 2415020.0) / 36525.0;
    t2 = t * t;
    t3 = t2 * t;

    // Geometric mean longitude of the Sun, referred to the
    // mean equinox of the date.
    l = fixangle(279.69668 + 36000.76892 * t + 0.0003025 * t2);

    // Sun's mean anomaly.
    m = fixangle(358.47583 + 35999.04975*t - 0.000150*t2 - 0.0000033*t3);

    // Eccentricity of the Earth's orbit.
    e = 0.01675104 - 0.0000418 * t - 0.000000126 * t2;

    // Eccentric anomaly.
    ea = kepler(m, e);

    // True anomaly
    v = fixangle(2 * rtd(atan(sqrt((1 + e) / (1 - e))  * tan(ea / 2))));

    // Sun's true longitude.
    theta = l + v - m;

    // Obliquity of the ecliptic.
    eps = 23.452294 - 0.0130125 * t - 0.00000164 * t2 + 0.000000503 * t3;

    // Corrections for Sun's apparent longitude, if desired.
    if (apparent) {
        omega = fixangle(259.18 - 1934.142 * t);
        theta = theta - 0.00569 - 0.00479 * sin(dtr(omega));
        eps += 0.00256 * cos(dtr(omega));
    }

    // Return Sun's longitude and radius vector
    *slong = theta;
    *rv = (1.0000002 * (1 - e * e)) / (1 + e * cos(dtr(v)));

    // Determine solar co-ordinates.
    *ra = fixangle(rtd(atan2(cos(dtr(eps)) * sin(dtr(theta)), cos(dtr(theta)))));
    *dec = rtd(asin(sin(dtr(eps)) * sin(dtr(theta))));
}

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

#include "qtimestring.h"

#include <qtopiaapplication.h>
#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QLocale>
#include <qtopialog.h>

static const char *const unTranslatedFullMonthNames[] = {
    QT_TRANSLATE_NOOP( "QDate", "January" ),
    QT_TRANSLATE_NOOP( "QDate", "February" ),
    QT_TRANSLATE_NOOP( "QDate", "March" ),
    QT_TRANSLATE_NOOP( "QDate", "April" ),
    QT_TRANSLATE_NOOP( "QDate", "May" ),
    QT_TRANSLATE_NOOP( "QDate", "June" ),
    QT_TRANSLATE_NOOP( "QDate", "July" ),
    QT_TRANSLATE_NOOP( "QDate", "August" ),
    QT_TRANSLATE_NOOP( "QDate", "September" ),
    QT_TRANSLATE_NOOP( "QDate", "October" ),
    QT_TRANSLATE_NOOP( "QDate", "November" ),
    QT_TRANSLATE_NOOP( "QDate", "December" )
};

static const char *const unTranslatedMonthNames[] = {
    QT_TRANSLATE_NOOP( "QDateShort", "Jan" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Feb" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Mar" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Apr" ),
    QT_TRANSLATE_NOOP( "QDateShort", "May" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Jun" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Jul" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Aug" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Sep" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Oct" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Nov" ),
    QT_TRANSLATE_NOOP( "QDateShort", "Dec" )
};

static const char *const unTranslatedFullDayNames[] = {
    QT_TRANSLATE_NOOP( "QDate", "Monday" ),
    QT_TRANSLATE_NOOP( "QDate", "Tuesday" ),
    QT_TRANSLATE_NOOP( "QDate", "Wednesday" ),
    QT_TRANSLATE_NOOP( "QDate", "Thursday" ),
    QT_TRANSLATE_NOOP( "QDate", "Friday" ),
    QT_TRANSLATE_NOOP( "QDate", "Saturday" ),
    QT_TRANSLATE_NOOP( "QDate", "Sunday" )
};

static const char *const unTranslatedDayNames[] = {
    QT_TRANSLATE_NOOP( "QDate", "Mon" ),
    QT_TRANSLATE_NOOP( "QDate", "Tue" ),
    QT_TRANSLATE_NOOP( "QDate", "Wed" ),
    QT_TRANSLATE_NOOP( "QDate", "Thu" ),
    QT_TRANSLATE_NOOP( "QDate", "Fri" ),
    QT_TRANSLATE_NOOP( "QDate", "Sat" ),
    QT_TRANSLATE_NOOP( "QDate", "Sun" )
};


/************ QTimeStringData ******************/
/* helper class for QTimeString */

class QTimeStringData : public QObject
{
    Q_OBJECT
protected:
    QTimeStringData();
    ~QTimeStringData();

public:
    static QTimeStringData* getInstance();

    bool currentAMPM() const { return ampm; }
    QString currentFormat() const;
    QString currentNumberFormat() const;

    QString numberDate(const QDate &d, QTimeString::Length length) const;

    void reinitialize();

private:
    static QPointer<QTimeStringData> theInstance;
    QString mCurrentFormat; //current format

    bool ampm;
};

QPointer<QTimeStringData> QTimeStringData::theInstance = 0;

QTimeStringData::QTimeStringData()
    : QObject(qApp)
{
    reinitialize();
}

QTimeStringData::~QTimeStringData()
{
}

QTimeStringData* QTimeStringData::getInstance()
{
    if (!theInstance)
        theInstance = new QTimeStringData();

    return theInstance;
}

QString QTimeStringData::currentFormat() const
{
    QString format = mCurrentFormat;
    format.remove(QChar('%'));
    return format;
}

void QTimeStringData::reinitialize()
{
    QSettings config("Trolltech","qpe");

    config.beginGroup( "Time" );
    ampm = config.value("AMPM",false).toBool();
    config.endGroup();

    config.beginGroup( "Date" );
    QVariant v = config.value("DateFormat");
    if (!v.isValid())
        mCurrentFormat = QString();
    else
        mCurrentFormat = v.toString();
}

QString QTimeStringData::currentNumberFormat() const
{
    QString format = mCurrentFormat;

    if ( format.isEmpty() ) {
        //the format is given by current locale
        QLocale loc;
        format = loc.dateFormat( QLocale::ShortFormat );
        format.replace( QRegExp( "y+" ), "%Y" );
        format.replace( QRegExp( "M+" ), "%M" );
        format.replace( QRegExp( "d+" ), "%D" );
    }

    return format;
}


QString QTimeStringData::numberDate(const QDate &d, QTimeString::Length length) const
{
    QString format = currentNumberFormat();
    switch(length)
    {
        case QTimeString::Short:
            {
                format.replace(QString("%D"), QString("d"));
                format.replace(QString("%M"), QString("M"));
                format.replace(QString("%Y"), QString("yy"));
            }
            break;
        default:
        case QTimeString::Medium:
            {
                format.replace(QString("%D"), QString("dd"));
                format.replace(QString("%M"), QString("MM"));
                format.replace(QString("%Y"), QString("yy"));
            }
            break;
        case QTimeString::Long:
            {
                format.replace(QString("%D"), QString("dd"));
                format.replace(QString("%M"), QString("MM"));
                format.replace(QString("%Y"), QString("yyyy"));
            }
            break;
    }
    return d.toString(format);
}


/****************** QTimeString ***********************/

//We cannot use QDateTimes translations because they rely on system locale.
//QTimeString provides a localised timestring independent of the system locale.

/*!
  \class QTimeString
    \inpublicgroup QtBaseModule

  \brief The QTimeString class provides localized strings for times and dates.

  It is recommended to use QTimeString rather than QDateTime::toString()
  because QTimeString returns localized strings for a given date and/or time.
  Each function expects a parameter that determines the length of a returned string.
  If the user changes the date or time format all strings obtained from QTimeString
  have to be invalidated. The QtopiaApplication::dateFormatChanged() and
  QtopiaApplication::clockChanged() signals will indicate when this happens.
  The handler for these signals must ensure that all time/date strings are refreshed.

  \warning The length of the localized string can vary among languages.

  \sa QtopiaApplication::dateFormatChanged(), QtopiaApplication::clockChanged()

  \ingroup time
*/


/*!
  \enum QTimeString::Length

  This enum specifies the different string lengths supported by QTimeString.

  \value Short Returns shortest possible string (mostly used on phones).
  \value Medium Returns default length string.
  \value Long Returns verbose string.
*/

static QString shortDayName( int weekday )
{
    switch ( weekday ) {
        case 1: return qApp->translate("QDate", "M", "Single character representing Monday in given language");
        case 2: return qApp->translate("QDate", "T", "Single character representing Tuesday in given language");
        case 3: return qApp->translate("QDate", "W", "Single character representing Wednesday in given language");
        case 4: return qApp->translate("QDate", "T", "Single character representing Thursday in given language");
        case 5: return qApp->translate("QDate", "F", "Single character representing Friday in given language");
        case 6: return qApp->translate("QDate", "S", "Single character representing Saturday in given language");
        case 7: return qApp->translate("QDate", "S", "Single character representing Sunday in given language");
        default: return QString();
    }
}


/*!
  \fn QString QTimeString::nameOfWeekDay( int day, Length len)

  Returns the name for the \a day of the week in the given \a len.

  \list
    \o \c{QTimeString::Long} - Monday
    \o \c{QTimeString::Medium} - Mon
    \o \c{QTimeString::Short} - M
  \endlist
*/
QString QTimeString::nameOfWeekDay( int d, Length len)
{
    if (d < 1 || d > 7) {
        qWarning("QTimeString::nameOfWeekDay: not a valid day");
        d = 1;
    }

    if (qApp) {
        switch(len) {
            case Short:
                return ::shortDayName(d);
            case Medium:
                return qApp->translate("QDate", unTranslatedDayNames[d-1]);
            case Long:
                return qApp->translate("QDate", unTranslatedFullDayNames[d-1]);
            default: ;
        }
    }
    // use system locale
    return len == Long ? QDate().longDayName(d) : QDate().shortDayName(d);
}

/*!
  \fn QString QTimeString::nameOfMonth( int month, Length len )

  Returns the name of \a month in the given \a len.

  \list
    \o \c{QTimeString::Long} - January
    \o \c{QTimeString::Medium} - Jan
    \o \c{QTimeString::Short} - Jan
  \endlist
*/
QString QTimeString::nameOfMonth( int m, Length len)
{
    if (m < 1 || m > 12) {
        qWarning("QTimeString::nameOfMonth: not a valid month");
        m = 1;
    }

    if (qApp) {
        if (len == Long)
            return qApp->translate("QDate", unTranslatedFullMonthNames[m-1]);
        else
            return qApp->translate("QDateShort", unTranslatedMonthNames[m-1]);
    }
    //use system locale
    return len == Long ? QDate().longMonthName(m) : QDate().shortMonthName(m);
}

/*!
  Returns true if 12 hour time is preferred over 24 hour time; otherwise returns false.
*/
bool QTimeString::currentAMPM()
{
    QTimeStringData* d = QTimeStringData::getInstance();
    return d->currentAMPM();
}

/*!
   \internal
*/
void QTimeString::updateFormats()
{
    QTimeStringData::getInstance()->reinitialize();
}

/*!
  Returns the date format used by QTimeString (e.g. D.M.Y) if set;
  otherwise returns an empty string.

  In case no format is set the default format which is returned
  by QLocale::dateFormat() in QLocale::ShortFormat is used.

  \sa QTimeString::formatOptions()
*/
QString QTimeString::currentFormat()
{
    QTimeStringData *d = QTimeStringData::getInstance();
    return d->currentFormat();
}

/*!
  Returns additional format options for a date string. The default format
  is the format of the current locale as returned
  by QLocale::dateFormat() in QLocale::ShortFormat.

  \sa QTimeString::currentFormat()
*/
QStringList QTimeString::formatOptions()
{
    QStringList mFormats;
    //add standard options
    mFormats.append("D/M/Y");
    mFormats.append("D.M.Y");
    mFormats.append("M/D/Y");
    mFormats.append("Y-M-D");

    return mFormats;
}

/*!
  \fn QString QTimeString::numberDateString( const QDate &date, Length len )

  Returns a localized string for \a date
  showing day, month and year as a number in the given \a len.

  \list
    \o \c{QTimeString::Long} - 07/08/2005
    \o \c{QTimeString::Medium} - 07/08/05
    \o \c{QTimeString::Short} - 7/8/05
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::numberDateString( const QDate &date, Length len )
{
    QTimeStringData *d = QTimeStringData::getInstance();
    return d->numberDate(date, len);
}

/*!
  Returns a localized string for \a hour,
  in 12 hour if QTimeString::currentAMPM() is true;
  otherwise in 24 hour format.

  If \a hour is greater than 23 or less then 0 then hour will default to 0.
*/
QString QTimeString::localH( int hour )
{
    QString s;
    if ( (hour < 0) || (hour > 23))
        hour = 0;

    QTime t(hour, 0);
    if ( QTimeString::currentAMPM() ) {
        s = t.toString("hap"); // 5pm
    } else {
        s = t.toString("hh"); // 17
    }
    return s;
}

QString timeString(const QTime &t, bool seconds, QTimeString::Length len)
{
    bool ampm = QTimeString::currentAMPM();
    QString format, result;
    format.append(QChar('h',0));

    if (len == QTimeString::Long)
        format.append(QChar('h',0));
    format.append(QString(":mm"));  //no tr
    if (seconds)
        format.append(QString(":ss")); //no tr
    if (ampm)
        format.append(QString(" AP")); //no tr
    result = t.toString(format);

    if (ampm && len == QTimeString::Short) { //remove am/pm
        result.remove(result.length()-3, 3);
    }
    return result;
}

/*!
  Returns a localized string for \a t in the given \a len,
  showing hours and minutes.

  \list
    \o \c{QTimeString::Long} - 02:05 PM
    \o \c{QTimeString::Medium} - 2:05 PM
    \o \c{QTimeString::Short} - 2:05
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localHM( const QTime &t, Length len )
{
    return timeString(t, false, len);
}

/*!
  Returns a localized string for \a t in the given \a len,
  showing hours, minutes, and seconds.

  \list
    \o \c{QTimeString::Long} - 02:05:12 PM
    \o \c{QTimeString::Medium} - 2:05:12 PM
    \o \c{QTimeString::Short} - 2:05:12
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localHMS( const QTime &t, Length len)
{
    return timeString(t, true, len);
}

/*!
  Returns a localized string for \a t in the given \a len,
  showing hours, minutes, and day of the week.

  \list
    \o \c{QTimeString::Long} - Monday 02:04 PM
    \o \c{QTimeString::Medium} - Mon 2:40 PM
    \o \c{QTimeString::Short} - M 2:40
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localHMDayOfWeek( const QDateTime &t, Length len )
{
    QString strTime = QTimeStringData::tr( "%1 %2", "1=Mon 2=15:40 - translation determines order -> 'Mon 15:40' or '15:40 Mon'")
        .arg(QTimeString::nameOfWeekDay(t.date().dayOfWeek(), len))
        .arg(timeString(t.time(), false, len));
    return strTime;
}

/*!
  Returns a localized string for \a t in the given \a len,
  showing hours, minutes, seconds, and day of the week.

  \list
    \o \c{QTimeString::Long} - Monday 02:04:14 PM
    \o \c{QTimeString::Medium} - Mon 2:40:14 PM
    \o \c{QTimeString::Short} - M 2:40:14
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localHMSDayOfWeek( const QDateTime &t, Length len )
{
    QString strTime = QTimeStringData::tr( "%1 %2", "1=Mon 2=15:40 - translation determines order -> 'Mon 15:40' or '15:40 Mon'")
        .arg(QTimeString::nameOfWeekDay(t.date().dayOfWeek(), len))
        .arg(timeString( t.time(), true, len ));
    return strTime;
}

/*!
  Returns a localized string for \a dt in the given \a len,
  showing month and date.

  \list
    \o \c{QTimeString::Long} - 27 September
    \o \c{QTimeString::Medium} - 27 Sep
    \o \c{QTimeString::Short} - 27 Sep
  \endlist

  The format, including the order of month and date depends on the user's current locale.
*/
QString QTimeString::localMD( const QDate &dt, Length len )
{
    QString format = QTimeStringData::tr("%D %M", "Date format that contains only day and month."
                            " Swap the order of day and month for a preferred display in a chosen language."
                            " Add dots or commas or suffix if that is preferred but do not translate %D or %M."
                            " They will be replaced by the appropriate values"
                            " e.g. '%D. %M' -> '23. Aug' or '%M %D,'->'Aug 23,'.");

    switch ( len ) {
        case QTimeString::Long:
            format.replace( QLatin1String("%D"), QLatin1String("dd") );
            format.replace( QLatin1String("%M"), QLatin1String("MMMM") );
            break;
        case QTimeString::Medium:
            format.replace( QLatin1String("%D"), QLatin1String("dd") );
            format.replace( QLatin1String("%M"), QLatin1String("MMM") );
            break;
        case QTimeString::Short:
            format.replace( QLatin1String("%D"), QLatin1String("d") );
            format.replace( QLatin1String("%M"), QLatin1String("MMM") );
            break;
    }
    return dt.toString( format );
}


/*!
  Returns a localized string for \a dt in the given \a len
  showing year, month, and date.

  \list
    \o \c{QTimeString::Long} - Monday 27 September 2005
    \o \c{QTimeString::Medium} - 27 Sep 2005
    \o \c{QTimeString::Short} - the same as the medium string
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localYMD( const QDate &dt, Length len )
{
    QLocale loc;
    QString format;
    if ( len == QTimeString::Long )
        return loc.toString(dt, QLocale::LongFormat );
    else  //since Qtopia overrides the short format the returned value will be the same as the numberDate
        return loc.toString(dt, QLocale::ShortFormat );
}

/*!
  Returns a localized string for \a dt in the given \a len,
  showing year, month, date, hours, minutes, and seconds.

  \list
    \o \c{QTimeString::Long} - 27 September 2005 02:54:22 PM
    \o \c{QTimeString::Medium} - 27 Sep 05 2:54:22 PM
    \o \c{QTimeString::Short} - 27 Sep 05 2:54:22
  \endlist

  The format, including order depends on the user's settings.
*/
QString QTimeString::localYMDHMS( const QDateTime &dt, Length len )
{
    const QDate& d = dt.date();
    const QTime& t = dt.time();
    return QTimeStringData::tr("%1 %2","1=date,2=time - determines order")
        .arg(localYMD(d,len)).arg(localHMS(t, len));
}

/*!
  \fn QString QTimeString::localDayOfWeek( const QDate& date, Length len )
  Returns a localized name of the week day on \a date in the given \a len.

  \list
    \o \c{QTimeString::Long} - Monday
    \o \c{QTimeString::Medium} - Mon
    \o \c{QTimeString::Short} - M
  \endlist
*/
QString QTimeString::localDayOfWeek( const QDate& d, Length len )
{
    return nameOfWeekDay(d.dayOfWeek(), len);
}

#include "qtimestring.moc"

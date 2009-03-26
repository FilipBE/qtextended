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
#include <qtopiadesktoplog.h>
#include <desktopsettings.h>

Q_GLOBAL_STATIC_WITH_ARGS(DesktopSettings, logSettings, ("Log"));
Q_GLOBAL_STATIC(QList<char*>, logCache);

void qtopiadesktopLogSemiVolatile(char *mem)
{
    QList<char*> &list(*logCache());
    if (!list.contains(mem))
        list.append(mem);
}

bool qtopiadesktopLogEnabled( const char *_category )
{
    if (_category == 0) {
        logSettings()->sync();
        foreach (char *mem, *logCache())
            (*mem) = 0;
        return false;
    }
    QLatin1String category( _category );
    DesktopSettings &settings(*logSettings());
    QVariant v = settings.value( category );
    if ( v.isNull() ) {
        v = DesktopSettings::debugMode()?1:0;
        if ( category == "TRACE" )
            v = 0;
        settings.setValue( category, v );
        settings.sync();
    }
    bool ret = v.toBool();
    //qDebug() << "category" << category << ret;
    return ret;
}

/*!
  \headerfile <qtopiadesktoplog.h>
  \title <qtopiadesktoplog.h>
  \ingroup headers
  \brief The <qtopiadesktoplog.h> header contains the category definitions used in Qt Extended Sync Agent.

  The <qtopiadesktoplog.h> header contains the category definitions used in Qt Extended Sync Agent.

  \quotefromfile libraries/qtopiadesktop/qtopiadesktoplog.h
  \skipto QD_LOG_OPTION(QDA)
  \printuntil QDSync

  \sa <qlog.h>
*/

/*!
  \macro QDLOG_OPTION(CATEGORY)
  \relates <qtopiadesktoplog.h>

  Register a category for conditional logging. This enables qLog(\a{CATEGORY}) but
  the log messages are not displayed unless \a CATEGORY is enabled. This macro uses the
  qtopiadesktopLogEnabled() function to check if \a CATEGORY is enabled.
*/

/*!
  \fn bool qtopiadesktopLogEnabled( const char *category )
  \relates <qtopiadesktoplog.h>
  This function returns true if \a category has been enabled.

  \quotefromfile libraries/qtopiadesktop/qtopiadesktoplog.cpp
  \skipto DesktopSettings
  \printuntil return
*/

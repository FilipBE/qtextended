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

#include "abstractdialfilter.h"
#include <QSettings>

/*!
  \class AbstractDialFilter
    \inpublicgroup QtTelephonyModule
  \brief The AbstractDialFilter class provides an interface for dial sequence filters.
  \ingroup QtopiaServer::Telephony
  \ingroup QtopiaServer::Task::Interfaces

  Dial filter allow the preprocessing of dial sequences. A GSM/3G device may use a filter
  to process key commands based on e.g. 3GPP TS 22.030 whereas a normal land line phone may need to 
  play a dialtone for an outside line.

  Dial related user interfaces should use defaultFilter() to access the filter.
  The default filter can be initialised by setting the \c{DialFilter/Default} key
  in the \c {Trolltech/Phone} configuration file. If this setting is missing defaultFilter()
  will return 0.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa GSMDialFilter
*/

/*!
  \fn AbstractDialFilter::AbstractDialFilter( QObject *parent )
  \internal
  */

/*!
  \fn AbstractDialFilter::~AbstractDialFilter()
  \internal
  */

/*!
  \enum AbstractDialFilter::Action
  This enum returns the type of action taken as a result of the filter matching.

  \value Reject Reject the call because it has been blocked.
  \value Continue Continue the dial sequence.
  \value DialIfNoMoreDigits The number should be dialed if the user does not type 
                            any more digits within a reasonable timeout period.
  \value Dialtone Generate a dialtone for an outside line.
  \value DialImmediately Immediately dial the number
  \value ActionTaken The filter has taken action and does not require any action by the calling party anymore.
  */

/*!
  \fn AbstractDialFilter::Action AbstractDialFilter::filterInput( const QString& input, bool sendPressed, bool takeNoAction )

  The function returns the result of filtering the given \a input. \a sendPressed indicates whether
  \a input was followed by the \c SEND function. Usually the \c SEND function is represented
  by the call button on the keypad.

  Some dial strings can trigger immediate actions by the dial filter. Usually such a call would return
  AbstractDialFilter::ActionTaken. If \a takeNoAction is \c true the filter will not perform those
  immediate actions. This may be necessary if the caller intends to just test whether an action
  would be performed by the filter. \a sendPressed is ignored if \a takeNoAction is \c true.
  */

/*!
  \fn QByteArray AbstractDialFilter::type() const

  Returns the filter type.
  */

class AbstractDialFilterPrivate
{
public:
    AbstractDialFilterPrivate() 
        : instance(0)
    {
        QSettings cfg("Trolltech", "Phone");
        QByteArray defaultType = cfg.value("DialFilter/Default", QByteArray()).toByteArray();
        
        QList<AbstractDialFilter *> filters = qtopiaTasks<AbstractDialFilter>();
        foreach( AbstractDialFilter* filter, filters ) {
            if ( filter->type() == defaultType ) {
                instance = filter;
                break;
            }
        }
    }
 
    AbstractDialFilter *instance;
};
Q_GLOBAL_STATIC(AbstractDialFilterPrivate, fp);

/*!
  Returns the default dial filter or 0 if none is set. filteringRequired() should be tested 
  before using the returned filter.
  \sa 
  */
AbstractDialFilter* AbstractDialFilter::defaultFilter() 
{
    return fp()->instance;
}

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

#include <qtopiafeatures.h>
#include "screenclick.h"
#include <qwindowsystem_qws.h>
#include <QSettings>
#include <QtopiaChannel>

/*!
  \class ScreenClick
    \inpublicgroup QtMediaModule
  \ingroup QtopiaServer::Task
  \brief The ScreenClick class enables an audible clicking sound
         whenever the stylus is used.

  The ScreenClick class is not a true task.  Instead, a real task should derive
  from the ScreenClick baseclass and implement the ScreenClick::screenClick()
  virtual method.

  For example, an implementation may look like this:

  \code
  class MyScreenClick : public ScreenClick
  {
  protected:
      virtual void screenClick(bool pressed)
      {
          // Make click noise
      }
  };
  QTOPIA_TASK(MyScreenClick, MyScreenClick);
  \endcode

  Screen clicking will only occur when the \c {Trolltech/Sound/System/Touch}
  configuration variable is true, otherwise ScreenClick::screenClick() will not
  be called. If a settings application updates this variable it should send the message
  "updateAudibleScreenClick()" on the "QPE/System" channel to allow the value to be reread.
  Creating this class automatically enables the "AudibleScreenClick" QtopiaFeature.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa KeyClick
 */

/*!
  Construct a new ScreenClick instance.
 */
ScreenClick::ScreenClick()
: m_clickenabled(false), m_up(true)
{
    QtopiaServerApplication::instance()->installQWSEventFilter(this);
    QtopiaChannel* sysChannel = new QtopiaChannel("QPE/System", this);
    connect(sysChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(sysMessage(QString,QByteArray)));
    readSettings();
    QtopiaFeatures::setFeature("AudibleScreenClick");
}

/*!
  Destroys the ScreenClick instance.
 */
ScreenClick::~ScreenClick()
{
    QtopiaServerApplication::instance()->removeQWSEventFilter(this);
}

/*!
  \fn void ScreenClick::screenClick(bool pressed)

  Called whenever the user taps or releases the screen.  \a pressed will be
  true when the user taps the screen, and false when they release.
 */

/*! \internal */
bool ScreenClick::qwsEventFilter(QWSEvent *e)
{
    if ( e->type == QWSEvent::Mouse ) {
        QWSMouseEvent *me = (QWSMouseEvent *)e;
        if ( me->simpleData.state&Qt::LeftButton ) {
            if ( m_up ) {
                m_up = false;
                if(m_clickenabled)
                    screenClick(true);
            }
        } else if ( !m_up ) {
            m_up = true;
            if(m_clickenabled)
                screenClick(false);
        }
    }
    return false;
}

void ScreenClick::sysMessage(const QString& message, const QByteArray &data)
{
    Q_UNUSED(data);
    if (message == "updateAudibleScreenClick()") {
        readSettings();
    }
}

void ScreenClick::readSettings()
{
    QSettings cfg("Trolltech","Sound");
    cfg.beginGroup("System");
    m_clickenabled = cfg.value("Touch").toBool();
}


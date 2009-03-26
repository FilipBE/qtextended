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

#include "serverthemeview.h"
#include "uifactory.h"
#include <qtopiaservices.h>
#include <qtopianamespace.h>
#include <qvaluespace.h>
#include <qtopiaipcenvelope.h>
#include <QContent>

QSet<PhoneThemedView *> PhoneThemedView::m_themedViews;

// declare PhoneThemedViewMonitor
class PhoneThemedViewMonitor : public QObject
{
Q_OBJECT
public:
    PhoneThemedViewMonitor();
    void add(PhoneThemedView *);

private:
    void setThemeText(const QString &name, const QString &text, const QString &shortText=QString());
    void setThemeStatus(const QString &name, bool enable);
    void setThemeItemActive(const QString &name, bool active);
};
Q_GLOBAL_STATIC(PhoneThemedViewMonitor, phoneThemedViewMonitor);

/*
  \class PhoneThemedView
    \inpublicgroup QtBaseModule
  \brief The PhoneThemedView class provides a ThemedView supporting all the
         common Qt Extended UI theme tags.
  \ingroup QtopiaServer::PhoneUI

  Deriving from this class gives your themed view the following value and
  interactive item support.

  Hopefully this class will be replaced by themes sourcing data from the
  valuespace.

  \section1 Common value items

  \section1 Interactive items

  \table
  \header
    \o Item
    \o Action
  \row
    \o messages
    \o Launch message viewer
  \row
    \o calls
    \o Shows call history
  \row
    \o roaming
    \o Starts call networks
  \row
    \o signal
    \o Starts call networks
  \row
    \o calldivert
    \o Starts call forwarding
  \row
    \o alarm
    \o Edits alarm
  \row
    \o battery
    \o Starts light-and-power
  \row
    \o time
    \o Edits time
  \row
    \o date
    \o Edits date
  \row
    \o profile
    \o Starts ring profile
  \row
    \o dialer
    \o Shows dialer
  \row
    \o presence
    \o Starts presence editor
  \row
    \o voipsettings
    \o Starts VoIP settings application
  \endtable

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

PhoneThemedView::PhoneThemedView(QWidget *parent, Qt::WFlags f)
: ThemedView(parent, f)
{
    QObject::connect(this, SIGNAL(itemClicked(ThemeItem*)),
                     this, SLOT(myItemClicked(ThemeItem*)));
    m_themedViews.insert(this);
    phoneThemedViewMonitor()->add(this);
}

PhoneThemedView::~PhoneThemedView()
{
    m_themedViews.remove(this);
}

QWidget *PhoneThemedView::newWidget(ThemeWidgetItem *input, const QString &name)
{
    Q_UNUSED(name);
    if (!input)
        return 0;

    // First check if the theme widget is a homescreen widget
    // we cannot use \a name because it is lower case and we need the case sensitive name
    QWidget* widget = UIFactory::createWidget(input->itemName().toUtf8(), this );
    if (widget != 0)
        return widget;
    return 0;
}

void PhoneThemedView::myItemClicked(ThemeItem *item)
{
    if( !item->isInteractive() )
        return;
    QString in = item->itemName();
    if( in == "messages" ) {
        QtopiaServiceRequest e( "Messages", "viewNewMessages(bool)" );
        e << true;
        e.send();
    } else if( in == "calls" ) {
        QtopiaServiceRequest e( "CallHistory", "showCallHistory(QCallList::ListType,QString)");
        //we don't want to depend on telephony hence we dont use QCallList directly
        e << 3;     /*QASSERT(QCallList::Missed == 3)*/
        e << QString(); // don't apply filtering
        e.send();
    } else if( in == "callhistory" ) {
        QtopiaIpcEnvelope e("QPE/Application/callhistory", "raise()");
    } else if( in == "inbox" ) {
        QtopiaIpcEnvelope e("QPE/Application/qtmail", "raise()");
    } else if( in == "dialer" ) {
        QtopiaIpcEnvelope e("QPE/Application/dialer", "raise()");
    } else if( in == "home" ) {
        QtopiaIpcEnvelope e("QPE/System", "showHomeScreen()");
    } else if( in == "roaming" || in == "signal" ) {
        QContent app( QtopiaService::app( "CallNetworks" ));
        if( app.isValid() )
            app.execute();
    } else if( in == "calldivert" ) {
        QContent app( QtopiaService::app( "CallForwarding" ));
        if( app.isValid() )
            app.execute();
    } else if( in == "alarm" ) {
        QtopiaServiceRequest env( "Alarm", "editAlarm()" );
        env.send();
    } else if( in == "battery" ) {
        // TODO: show more battery info
        Qtopia::execute("light-and-power");
    } else if( in == "time" ) {
        QtopiaServiceRequest env( "Time", "editTime()" );
        env.send();
    } else if( in == "date" ) {
        QtopiaServiceRequest env( "Date", "editDate()" );
        env.send();
    } else if( in == "profile" ) {
        QtopiaServiceRequest e( "RingProfiles", "showRingProfiles()" );
        e.send();
    } else if (in == "contacts") {
        QtopiaIpcEnvelope e("QPE/Application/contacts", "raise()");
    } else if( in == "media" ) {
        QContent app( QtopiaService::app( "PlayMedia" ));
        if( app.isValid() )
            app.execute();
    }
}

QSet<PhoneThemedView *> PhoneThemedView::themedViews()
{
    return m_themedViews;
}

// define PhoneThemedViewMonitor
PhoneThemedViewMonitor::PhoneThemedViewMonitor()
: QObject(0)
{
}

void PhoneThemedViewMonitor::add(PhoneThemedView *)
{
}

void PhoneThemedViewMonitor::setThemeText(const QString &name, const QString &text, const QString &shortText)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Text);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeTextItem *textItem = (ThemeTextItem *)(*it);
            if (textItem) {
                if (textItem->shortLabel() && !shortText.isEmpty())
                    textItem->setText(shortText);
                else
                    textItem->setText(text);
            }
        }
    }
}

void PhoneThemedViewMonitor::setThemeStatus(const QString &name, bool enable)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Status);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeStatusItem *status = (ThemeStatusItem *)(*it);
            if (status)
                status->setOn(enable);
        }
    }
    setThemeItemActive(name, enable);
}

void PhoneThemedViewMonitor::setThemeItemActive(const QString &name, bool active)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Item);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeItem *item = *it;
            if (item && item->rtti() != ThemedView::Status)
                item->setActive(active);
        }
    }
}

#include "serverthemeview.moc"


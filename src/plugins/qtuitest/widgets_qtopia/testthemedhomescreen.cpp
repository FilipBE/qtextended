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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "testthemedhomescreen.h"
#include "localwidget.h"
#include "testwidgetslog.h"

#include <QWidget>
#include <QVariant>
#include <Qtopia>
#include <ThemeItem>
#include <ThemedView>

using namespace QtUiTest;

TestThemedHomeScreen::TestThemedHomeScreen(QObject *_q)
    : TestWidget(_q),
      q(qobject_cast<QWidget*>(_q)),
      m_dialer(0),
      m_themedView(0)
{
    QObject *launcher = LocalWidget::find("PhoneLauncher");
    if (launcher) m_themedView = qobject_cast<ThemedView*>(LocalWidget::find("PhoneThemedView", launcher));
}

bool TestThemedHomeScreen::canEnter(QVariant const& item) const
{
    ThemeTextItem* infobox = static_cast<ThemeTextItem*>(m_themedView->findItem("infobox", ThemedView::Text));
    ThemeItem* pinbox = m_themedView->findItem("pinbox");
    TestWidgetsLog() << pinbox << infobox << (infobox ? infobox->text() : "");
    if (pinbox && pinbox->active() || infobox && infobox->active() ) {
        return item.canConvert<QString>();
    }
    TestWidgetsLog() << item << dialer();
    if (InputWidget* iw = qtuitest_cast<InputWidget*>(dialer()))
        return iw->canEnter(item);
    return false;
}

bool TestThemedHomeScreen::enter(QVariant const& item, bool noCommit)
{
    ThemeTextItem* infobox = static_cast<ThemeTextItem*>(m_themedView->findItem("infobox", ThemedView::Text));
    ThemeItem* pinbox = m_themedView->findItem("pinbox");
    TestWidgetsLog() << item.toString() << pinbox << infobox << (infobox ? infobox->text() : "");

    /*
        Pinbox: widget for entering PIN/PUK numbers in mouse-preferred mode.
            Never visible in keypad mode.
        Infobox: generic info box which says (e.g.) "No modem" or "Airplane mode".
            When entering a PIN/PUK, it has a message like "Please enter PIN:"
    */
    if (pinbox && pinbox->active() || infobox && infobox->active()
            && (infobox->text().contains("PIN") || infobox->text().contains("PUK"))) {
        TestWidgetsLog() << "Going to enter using pinbox/infobox.";
        return enterPinbox(item);
    }
    TestWidgetsLog() << item << dialer();
    Widget* w = qtuitest_cast<Widget*>(dialer());
    if (!w) {
        setErrorString(QString(
            "When entering %1 at the homescreen, could not find a pinbox, infobox "
            "or dialer to enter into.").arg(item.toString()));
        return false;
    }

    if (Qtopia::mousePreferred() && !w->isVisible()) {
        if (!raiseDialer()) {
            setErrorString(QString(
                "When entering %1 at the homescreen, could not raise the dialer "
                "to enter into.").arg(item.toString()));
            return false;
        }
    }

    if (InputWidget* iw = qtuitest_cast<InputWidget*>(dialer()))
        return iw->canEnter(item) && iw->enter(item, noCommit);

    QString error = QString(
        "When entering %1 at the homescreen, dialer is not an input widget.")
        .arg(item.toString());
    QDebug(&error) << " Dialer:" << dialer();
    setErrorString(error);
    return false;
}

bool TestThemedHomeScreen::enterPinbox(QVariant const& item)
{
    if (!canEnter(item))
        return false;
    if (!Qtopia::mousePreferred()){
        foreach (QChar c, item.toString()) {
            keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
        }
        return true;
    }

    struct DialerMap {
        static QString charToLabel(QChar const& c) {
            if ('1' == c) return "one";
            if ('2' == c) return "two";
            if ('3' == c) return "three";
            if ('4' == c) return "four";
            if ('5' == c) return "five";
            if ('6' == c) return "six";
            if ('7' == c) return "seven";
            if ('8' == c) return "eight";
            if ('9' == c) return "nine";
            if ('*' == c) return "star";
            if ('0' == c) return "zero";
            if ('#' == c) return "hash";
            return QString();
        }
    };

    SelectWidget *sw = qtuitest_cast<SelectWidget*>(m_themedView);
    if (!sw) {
        QString error;
        QDebug(&error) << "While entering" << item.toString() << "at home screen, "
            "could not find a SelectWidget for the dialer themed view. "
            "Themed view:" << m_themedView;
        setErrorString(error);
        return false;
    }
    foreach (QChar c, item.toString()) {
        if (!sw->select(DialerMap::charToLabel(c)))
            return false;
    }
    return true;
}

QString TestThemedHomeScreen::text() const
{ return qtuitest_cast<TextWidget*>(m_themedView)->text(); }

QStringList TestThemedHomeScreen::list() const
{ return qtuitest_cast<ListWidget*>(m_themedView)->list(); }

QRect TestThemedHomeScreen::visualRect(QString const& item) const
{
    ListWidget* lw = qtuitest_cast<ListWidget*>(m_themedView);
    Widget* w = qtuitest_cast<Widget*>(m_themedView);

    QRect r = lw->visualRect(item);
    if (!r.isNull()) {
        r.moveTopLeft(mapFromGlobal(w->mapToGlobal(r.topLeft())));
    }
    return r;
}

bool TestThemedHomeScreen::canSelect(QString const& item) const
{ return qtuitest_cast<SelectWidget*>(m_themedView)->canSelect(item); }

bool TestThemedHomeScreen::select(QString const& item)
{ return qtuitest_cast<SelectWidget*>(m_themedView)->select(item); }

QWidget* TestThemedHomeScreen::dialer() const
{
    if (m_dialer) return m_dialer;

    /* The dialer is instantiated lazily, so we must bring up the dialer
     * screen to ensure it has been created.
     */
    if (!(m_dialer = qobject_cast<QWidget*>(LocalWidget::find("QAbstractDialerScreen")))) {
        if (raiseDialer()) {
            TestWidgetsLog() << "Raised dialer to instantiate it";
            if (hideDialer()) {
                TestWidgetsLog() << "Hid dialer after instantiating it";
                m_dialer = qobject_cast<QWidget*>(LocalWidget::find("QAbstractDialerScreen"));
            } else {
                TestWidgetsLog() << "Failed to hide dialer after instantiating it";
            }
        } else {
            TestWidgetsLog() << "Failed to raised dialer to instantiate it";
        }
    }

    return m_dialer;
}

bool TestThemedHomeScreen::raiseDialer() const
{
    if (!Qtopia::mousePreferred()) {
        keyClick(Qt::Key_1);
        return true;
    } else {
        QObject *themedView = LocalWidget::find("PhoneLauncher");
        TestWidgetsLog() << "phonelauncher" << themedView;
        if (themedView) themedView = LocalWidget::find("PhoneThemedView", themedView);
        TestWidgetsLog() << "themedview" << themedView;
        if (SelectWidget *sw = qtuitest_cast<SelectWidget*>(themedView)) {
            if (!sw->select("dialer")) return false;
            SelectWidget *sm = qtuitest_cast<SelectWidget*>(findWidget(SoftMenu));
            if (!sm) return false;
            else {
                for (int i = 0;
                     i < QtUiTest::maximumUiTimeout() && !sm->canSelect("Cancel");
                     i += 50, QtUiTest::wait(50))
                {}
            }
            return sm->canSelect("Cancel");
        }
    }
    return false;
}

bool TestThemedHomeScreen::hideDialer() const
{
    if (!Qtopia::mousePreferred()) {
        keyClick(Qt::Key_Backspace);
        return true;
    } else {
        if (SelectWidget *sw = qtuitest_cast<SelectWidget*>(findWidget(SoftMenu))) {
            bool ret = sw->select("Cancel");
            TestWidgetsLog() << "select cancel from soft menu gave" << ret;
            for (int i = 0;
                i < QtUiTest::maximumUiTimeout() && sw->canSelect("Cancel") && ret;
                i += 50, QtUiTest::wait(50))
            {}
            return ret && !sw->canSelect("Cancel");
        } else {
            TestWidgetsLog() << "Could not get soft menu";
        }
    }
    return false;
}

bool TestThemedHomeScreen::canWrap(QObject *o)
{ return o && o->inherits("ThemedHomeScreen"); }


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

#include "testtext.h"
#include "testwidgetslog.h"

#include <QPointer>
#include <QVariant>

using namespace QtUiTest;

bool TestText::enterText(QObject* q, QString const& item, QString const& expectedText, bool commit)
{
    QByteArray textChanged = textChangedSignal(q);
    TextWidget* tw = qtuitest_cast<TextWidget*>(q);

    foreach (QChar const& c, item) {
        TestWidgetsLog() << asciiToModifiers(c.toLatin1());
        if (!keyClick(q, textChanged, asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) ))
            return false;
    }

    QString newText = tw->text();
    if (newText != expectedText) {
        setErrorString(QString("After entering text, expected text of:\n%1\nBut text was:\n%2")
            .arg(expectedText)
            .arg(newText));
        return false;
    }

    if (commit) {
        QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(q);
        if (w) return w->setEditFocus(false);
    }

    return true;
}

bool TestText::enterTextByProxy(QtUiTest::InputWidget* proxy, QObject* q, QString const& item, QString const& expectedText, bool commit)
{
    TextWidget* tw = qtuitest_cast<TextWidget*>(q);
    QByteArray textChanged = textChangedSignal(q);
    QPointer<QObject> safeQ(q);

    if (!proxy->enter(item, !commit)) return false;

    QString newText;

    for (int i = expectedText.length(); i != 0; --i) {
        if (!safeQ) {
            setErrorString("Widget was destroyed while entering text.");
            return false;
        }
        newText = tw->text();
        if (newText == expectedText) {
            break;
        }
        if (!waitForSignal(q, textChanged)) {
            setErrorString("Text did not change on focused widget due to key clicks");
            return false;
        }
    }

    if (newText != expectedText) newText = tw->text();
    if (newText != expectedText) {
        setErrorString(QString("After entering text, expected text of:\n%1\nBut text was:\n%2")
            .arg(expectedText)
            .arg(newText));
        return false;
    }

    return true;
}


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

#include "testlineedit.h"
#include "testtext.h"
#include "testwidgetslog.h"

#include <QDesktopWidget>
#include <QLayout>
#include <QLineEdit>
#include <QPointer>
#include <QValidator>
#include <QVariant>

TestLineEdit::TestLineEdit(QObject *_q)
    : TestGenericTextWidget(_q), q(qobject_cast<QLineEdit*>(_q))
{ connect(q, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString))); }

void TestLineEdit::onTextEdited(QString const& item)
{ emit entered(item); }

bool TestLineEdit::canEnter(QVariant const& item) const
{
    if (!item.canConvert<QString>()) return false;
    if (!q->validator())             return true;

    int dontcare = 0;
    QString text = item.toString();
    return (QValidator::Acceptable==q->validator()->validate(text, dontcare));
}

static void mouseLinearStroke(QPoint const &start, QPoint const &end)
{
    int ticks = 15;
    QPoint diff = (end - start)/ticks;
    QPoint p = start;

    for (int i = 0; i < ticks; p += diff, ++i) {
        QtUiTest::mousePress( p, Qt::LeftButton );
        QtUiTest::wait(20);
    }
    QtUiTest::wait(200);
    QtUiTest::mouseRelease( end );
}

bool TestLineEdit::erase_mouse()
{
    QObject *inputMethod = QtUiTest::findWidget(QtUiTest::InputMethod);

    QtUiTest::Widget *iw = qtuitest_cast<QtUiTest::Widget*>(inputMethod);
    if (!iw) return false;

    int rectCenterY = rect().height()/2;
    bool backstroke = true;
    QPoint hlStart, hlEnd;
    while ( !text().isEmpty() ) {
        /* Set the points based on whether we are doing a forward or back stroke */
        if (backstroke) {
            hlStart = mapToGlobal( QPoint(rect().width() - 5, rectCenterY) );
            hlEnd = QPoint(0, 0);
        } else {
            hlStart = mapToGlobal( QPoint(5, rectCenterY) );
            hlEnd = QPoint(qApp->desktop()->availableGeometry(q).width()-1, 0);
        }
        backstroke = !backstroke;

        /* Highlight the visible text */
        mouseLinearStroke(hlStart, hlEnd);

        /* Do the delete stroke */
        QPoint start = iw->mapToGlobal(QPoint(iw->rect().width() - 20, iw->rect().height()/2));
        QPoint end = iw->mapToGlobal(QPoint(20, iw->rect().height()/2));
        mouseLinearStroke(start, end);
        QtUiTest::wait(100);
    }

    return true;
}

bool TestLineEdit::enter(QVariant const& item, bool noCommit)
{
    if (!canEnter(item)) return false;

    bool hadEditFocus = hasEditFocus();

    if (!hadEditFocus && !setEditFocus(true)) return false;

    using namespace QtUiTest;


    QPointer<QObject> safeThis = this;

    QObject *inputMethod = findWidget(InputMethod);

    if (!safeThis) {
        setErrorString("Widget was destroyed while entering text.");
        return false;
    }

    QString oldText = text();
    QString itemString = item.toString();
    QString expectedText = oldText;
    expectedText.insert(q->cursorPosition(), itemString);

    /* If there's text currently in the field, and we don't already have
       edit focus, then erase it first. */
    if (!oldText.isEmpty() && !hadEditFocus) {
        if (QtUiTest::mousePreferred()) {
            if (!erase_mouse()) return false;
        } else {
            if (!TestText::eraseTextByKeys(q)) return false;
        }
        expectedText = itemString;
    }

    InputWidget* iw = qtuitest_cast<InputWidget*>(inputMethod);

    if (!safeThis) {
        setErrorString("Widget was destroyed while entering text.");
        return false;
    }

    TestWidgetsLog() << iw;

    if (iw) {
        if (!TestText::enterTextByProxy(iw, q, itemString, expectedText, !noCommit)) return false;
    } else {
        if (!TestText::enterText(q, itemString, expectedText, !noCommit)) return false;
    }

    if (!safeThis) {
        setErrorString("Widget was destroyed while entering text.");
        return false;
    }

    if (!noCommit && hasEditFocus()) {
        if (!setEditFocus(false)) {
            return false;
        }
    }

    return true;
}

bool TestLineEdit::canWrap(QObject *o)
{ return qobject_cast<QLineEdit*>(o); }


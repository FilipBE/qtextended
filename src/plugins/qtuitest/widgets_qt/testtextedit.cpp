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
#include "testtextedit.h"
#include "testwidgetslog.h"

#include <QLayout>
#include <QPointer>
#include <QTextDocumentFragment>
#include <QTextEdit>

TestTextEdit::TestTextEdit(QObject *_q)
    : TestWidget(_q)
      , lastEntered()
      , lastCursorPosition(-1)
      , committed(false)
      , q(qobject_cast<QTextEdit*>(_q))
{
    lastCursorPosition = cursorPosition();
    connect(q, SIGNAL(textChanged()),           this, SLOT(onTextChanged()));
    connect(q, SIGNAL(textChanged()),           this, SIGNAL(textChanged()));
    connect(q, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));
}

QString TestTextEdit::text() const
{ return q->toPlainText(); }

QString TestTextEdit::selectedText() const
{ return q->textCursor().selection().toPlainText(); }

bool TestTextEdit::canEnter(QVariant const& item) const
{
    return item.canConvert<QString>();
}

bool TestTextEdit::enter(QVariant const& item, bool noCommit)
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
    expectedText.insert(q->textCursor().position(), itemString);

    /* If there's text currently in the field, and we committed it,
       then erase it first. */
    if (!oldText.isEmpty() && committed) {
        if (QtUiTest::mousePreferred()) {
            if (!TestText::eraseTextByMouse(this)) return false;
        } else {
            if (!TestText::eraseTextByKeys(this)) return false;
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

    committed = !noCommit;

    return true;
}

bool TestTextEdit::canWrap(QObject *o)
{ return qobject_cast<QTextEdit*>(o); }

int TestTextEdit::cursorPosition() const
{
    return q->textCursor().position();
}

void TestTextEdit::onTextChanged()
{
    if (!hasFocus() || !isVisible()) return;
    if (q->toPlainText() == lastEntered) return;
    lastEntered = q->toPlainText();
    emit entered(lastEntered);
}

void TestTextEdit::onCursorPositionChanged()
{
    int prev = lastCursorPosition;
    lastCursorPosition = cursorPosition();
    emit cursorPositionChanged(prev, lastCursorPosition);
}


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

#include "testpredictivekeyboard.h"
#include "localwidget.h"
#include "testwidgetslog.h"

#include <QVariant>
#include <QPoint>
#include <QRect>
#include <QWidget>
#include <QTimer>

TestKeyboardWidget::TestKeyboardWidget(QObject *_q)
    : TestWidget(_q), q(qobject_cast<QWidget*>(_q))
{}

bool TestKeyboardWidget::canEnter(QVariant const& item) const
{
    return item.canConvert<QString>();
}

void mouseLinearStroke(QPoint const &start, QPoint const &end, int ticks = 15, int delay = 20)
{
    QPoint diff = (end - start)/ticks;
    QPoint p = start;
    int i = 0;

    for (; i < ticks; p += diff, ++i) {
        QtUiTest::mousePress( p, Qt::LeftButton );
        QtUiTest::wait(delay);
    }
    QtUiTest::mouseRelease( p );
}

bool TestKeyboardWidget::enter(QVariant const& item, bool noCommit)
{
    Q_UNUSED(noCommit);
    if (!canEnter(item)) return false;
    QString text = item.toString();
    /* Insert spaces around new line chars so we split properly */
    text.replace( '\n', QString(" \n ") );

    using namespace QtUiTest;

    QStringList lastDisplayedWords;
    foreach (QString word, text.split(" ", QString::SkipEmptyParts)) {
        int predictive = 1;
        if (word.data()->isLetterOrNumber()) {
            TestWidgetsLog() << "entering word" << word << "with predictive keyboard";
        } else {
            predictive = 0;
            TestWidgetsLog() << "entering word" << word << "manually since it starts with a symbol";
        }
        while (predictive >= 0) {
            lastDisplayedWords.clear();
            foreach (QChar c, word) {
                /* Replace new line escape characters into unicode */
                if (c == '\n')
                    c = QChar(0x21b5);

                QRect r;
                QPoint p;

                for (int i = 0; i < 5; ++i) {
                    if (!QMetaObject::invokeMethod(q, "rectForCharacter", Q_RETURN_ARG(QRect,r), Q_ARG(QChar, c))) {
                        qWarning("QtUitest: can't invoke rectForCharacter on predictive keyboard");
                        return false;
                    }

                    if (!r.isNull())
                        p = r.center();

                    if (!p.isNull()) break;

                    TestWidgetsLog() << "character" << c << "is not on current board, doing up stroke";

                    /* OK, character is not on this board; use upwards stroke to change board */
                    QPoint start = mapToGlobal(QPoint(rect().width()/2, rect().height() - 5));
                    QPoint end   = start - QPoint(0, q->height() - 10);
                    mouseLinearStroke( start, end );
                    QtUiTest::wait(300);
                }

                if (p.isNull()) {
                    qWarning() << "QtUitest: can't find character" << c << "in predictive keyboard";
                    return false;
                }

                TestWidgetsLog() << "predictive keyboard says coord of" << c << "is" << p;

                if (predictive) {
                    mouseClick( mapToGlobal(p), Qt::LeftButton );
                    /* Process events for keyboard update */
                    QtUiTest::wait(10);
                } else {
                    mousePress( mapToGlobal(p), Qt::LeftButton );
                    QtUiTest::waitForSignal(q, SIGNAL(pressedAndHeld()), 3000);
                    QtUiTest::wait(350); // 300 ms is the length of the animation timeline for show and hide of popup window
                    mouseRelease( mapToGlobal(p), Qt::LeftButton );
                    QtUiTest::wait(350);
                }

                /* Now check if we can select the word */
                if (canSelect(word) || word == QString('\n')) break;
                if (predictive) {
                    QStringList displayedWords = list();
                    /* Check that the displayed words changed; if not, skip to non-predictive mode */
                    if (!lastDisplayedWords.isEmpty() && lastDisplayedWords == displayedWords) {
                        TestWidgetsLog() << "predictive keyboard didn't change words after last character";
                        break;
                    }
                    lastDisplayedWords = displayedWords;
                }
            }
            if (canSelect(word) || word == QString('\n')) break;

            // OK, couldn't find word; need to backspace and try again
            if (predictive && !list().isEmpty()) {
                TestWidgetsLog() << "predictive keyboard says" << word << "isn't in dictionary, backspace and try again";
                QPoint start = mapToGlobal(QPoint(rect().width() - 20, rect().height()/2));
                QPoint end   = mapToGlobal(QPoint(20, rect().height()/2));
                mouseLinearStroke(start, end);
                QtUiTest::wait(100);
            }
            --predictive;
        }

        if ( word == QString('\n') ) continue;

        if (!canSelect(word)) {
            qWarning() << "QtUitest: typed all letters but keyboard did not detect word" << word;
            return false;
        }

        select(word);

        /* Wait slightly for animation to complete */
        QtUiTest::wait(500);
    }

    /* Finished with the keyboard; click on the options window to make it go away. */
    mouseClick( mapToGlobal(QPoint(rect().width()/2, -15)), Qt::LeftButton );
    QtUiTest::wait(500);

    /* Now backspace once to clear precommit text (trailing space) */
    QPoint start = mapToGlobal(QPoint(rect().width() - 20, rect().height()/2));
    QPoint end   = mapToGlobal(QPoint(20, rect().height()/2));
    mouseLinearStroke(start, end);
    QtUiTest::wait(100);

    return true;
}

bool TestKeyboardWidget::canSelect(QString const& item) const
{
    return list().contains(item);
}

bool TestKeyboardWidget::select(QString const& item)
{
    if (!canSelect(item)) return false;

    QPoint c = visualRect(item).center();
    QtUiTest::mouseClick( mapToGlobal(c), Qt::LeftButton );
    return true;
}

QStringList TestKeyboardWidget::list() const
{
    QStringList ret;
    if (!QMetaObject::invokeMethod(q, "words", Q_RETURN_ARG(QStringList,ret))) {
        qWarning("QtUitest: can't invoke words on predictive keyboard");
    }
    return ret;
}

QRect TestKeyboardWidget::visualRect(QString const& item) const
{
    QRect r;
    if (!QMetaObject::invokeMethod(q, "rectForWord", Q_RETURN_ARG(QRect,r), Q_ARG(QString, item))) {
        qWarning("QtUitest: can't invoke rectForWord on predictive keyboard");
    }
    return r;
}

bool TestKeyboardWidget::inherits(QtUiTest::WidgetType type) const
{ return (QtUiTest::InputMethod == type); }

bool TestKeyboardWidget::canWrap(QObject *o)
{ return o->inherits("KeyboardWidget"); }


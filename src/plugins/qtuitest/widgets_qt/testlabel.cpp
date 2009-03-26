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

#include "testlabel.h"
#include "testwidgetslog.h"

#include <QLabel>
#include <QGroupBox>

TestLabel::TestLabel(QObject* _q)
    : TestGenericTextWidget(_q), q(qobject_cast<QLabel*>(_q))
{ TestWidgetsLog(); }

QString TestLabel::text() const
{
    TestWidgetsLog();
    QString text = TestGenericTextWidget::text();
    if (q->textFormat() != Qt::PlainText)
        text = convertToPlainText(text);

    return text;
}

bool TestLabel::canWrap(QObject *o)
{ return qobject_cast<QLabel*>(o); }

QString TestLabel::convertToPlainText(QString const &richText)
{
    static QRegExp rxBr("<br */?>");
    static QRegExp rxHtml("<[^>]+>");

    QString ret(richText);
    ret.replace(rxBr, "\n");
    ret.replace(rxHtml, QString());
    return ret;
}

// Returns a groupbox prefix for widgets that are owned by a groupbox
QString TestLabel::groupBoxName() const
{
    QWidget *parent = q;
    QGroupBox *gb;
    QString ret;
    do {
        parent = qobject_cast<QWidget*>(parent->parent());
        gb = qobject_cast<QGroupBox*>(parent);
        if (gb && !gb->title().isEmpty())
            ret += gb->title() + "/";
    } while (parent);

    ret.chop(1);
    return ret;
}


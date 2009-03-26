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

#include "testgenerictextwidget.h"
#include "testwidgetslog.h"

#include <QWidget>
#include <QVariant>

QList<QByteArray> TestGenericTextWidget::textProperties = QList<QByteArray>()
    << "text"
    << "plainText"
    << "value"
    << "title"
;
QList<QByteArray> TestGenericTextWidget::selectedTextProperties = QList<QByteArray>()
    << "selectedText"
;

TestGenericTextWidget::TestGenericTextWidget(QObject* _q)
    : TestWidget(_q), q(qobject_cast<QWidget*>(_q))
{
    TestWidgetsLog() << _q;
    foreach (QByteArray property, textProperties) {
        if (-1 != q->metaObject()->indexOfProperty(property)) {
            textProperty = property;
            break;
        }
    }
    foreach (QByteArray property, selectedTextProperties) {
        if (-1 != q->metaObject()->indexOfProperty(property)) {
            selectedTextProperty = property;
            break;
        }
    }
}

QString TestGenericTextWidget::text() const
{ TestWidgetsLog(); return q->property(textProperty).toString(); }

QString TestGenericTextWidget::selectedText() const
{
    TestWidgetsLog();
    QString ret;
    if (!selectedTextProperty.isEmpty())
        ret = q->property(selectedTextProperty).toString();
    if (ret.isEmpty())
        ret = text();
    return ret;
}

bool TestGenericTextWidget::canWrap(QObject *o)
{
    QWidget *w;
    if (!(w = qobject_cast<QWidget*>(o)))
        return false;

    foreach (QByteArray property, textProperties)
        if (-1 != o->metaObject()->indexOfProperty(property))
            return true;

    return false;
}


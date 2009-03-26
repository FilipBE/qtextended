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

#include "testgenericcheckwidget.h"
#include "testwidgetslog.h"

#include <QWidget>
#include <QVariant>

QList<QByteArray> TestGenericCheckWidget::checkProperties = QList<QByteArray>()
    << "checked"
;

TestGenericCheckWidget::TestGenericCheckWidget(QObject* _q)
    : TestWidget(_q), q(qobject_cast<QWidget*>(_q))
{
    TestWidgetsLog();
    foreach (QByteArray property, checkProperties) {
        if (-1 != q->metaObject()->indexOfProperty(property)) {
            checkProperty = property;
            break;
        }
    }
}

Qt::CheckState TestGenericCheckWidget::checkState() const
{
    TestWidgetsLog();
    return q->property(checkProperty).toBool()
        ? Qt::Checked
        : Qt::Unchecked;
}

bool TestGenericCheckWidget::canWrap(QObject *o)
{
    QWidget *w;
    if (!(w = qobject_cast<QWidget*>(o)))
        return false;

    foreach (QByteArray property, checkProperties)
        if (-1 != o->metaObject()->indexOfProperty(property))
            return true;

    return false;
}


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

#include "qalternatestack_p.h"

struct QAlternateStackPrivate
{
    // All instances of QAlternateStack.
    static QList<QAlternateStack*> instances;
};
QList<QAlternateStack*> QAlternateStackPrivate::instances;

QAlternateStack::QAlternateStack(quint32 size, QObject* parent)
    : QObject(parent),
      d(new QAlternateStackPrivate)
{
    Q_ASSERT_X(QAlternateStack::isAvailable(), "QAlternateStack",
            "QAlternateStack is not available on this platform!");

    QAlternateStackPrivate::instances << this;
}

QAlternateStack::~QAlternateStack()
{
    delete d;
    d = 0;
    QAlternateStackPrivate::instances.removeAll(this);
}

void QAlternateStack::start(QAlternateStackEntryPoint entry, const QVariant& data)
{
    Q_ASSERT_X(!isActive(), "QAlternateStack::start",
            qPrintable(QString("`start' called while already active.")));
}

void QAlternateStack::switchTo()
{
    // We must not be the currently active stack.
    Q_ASSERT_X(!isCurrentStack(), "QAlternateStack::switchTo",
            qPrintable(QString("`switchTo' called in currently active stack.")));
}

void QAlternateStack::switchFrom()
{
    // We must be the currently active stack.
    Q_ASSERT_X(isCurrentStack(), "QAlternateStack::switchFrom",
        qPrintable(QString("`switchFrom' called from wrong stack.")));
}

bool QAlternateStack::isActive() const
{ return false; }

bool QAlternateStack::isCurrentStack() const
{ return false; }

QList<QAlternateStack*> QAlternateStack::instances()
{ return QAlternateStackPrivate::instances; }

bool QAlternateStack::isAvailable()
{
    // Not implemented for Windows yet.
    return false;
}


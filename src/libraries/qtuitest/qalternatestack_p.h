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

#ifndef QALTERNATESTACK_P_H
#define QALTERNATESTACK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QVariant>
#include <qtuitestglobal.h>

class QAlternateStack;
typedef void (*QAlternateStackEntryPoint)(QAlternateStack*,QVariant const&);

class QAlternateStackPrivate;

class QTUITEST_EXPORT QAlternateStack : public QObject
{
    Q_OBJECT

public:
    QAlternateStack(quint32 = 65536, QObject* = 0);
    virtual ~QAlternateStack();

    bool isActive() const;
    bool isCurrentStack() const;

    static QList<QAlternateStack*> instances();
    static bool isAvailable();

public slots:
    void start(QAlternateStackEntryPoint, const QVariant& = QVariant());
    void switchTo();
    void switchFrom();

private:
    QAlternateStackPrivate* d;
    friend class QAlternateStackPrivate;
};

#endif


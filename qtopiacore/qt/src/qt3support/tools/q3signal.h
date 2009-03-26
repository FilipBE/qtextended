/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef Q3SIGNAL_H
#define Q3SIGNAL_H

#include <QtCore/qvariant.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3Signal : public QObject
{
    Q_OBJECT

public:
    Q3Signal(QObject *parent=0, const char *name=0);
    ~Q3Signal();

    bool	connect(const QObject *receiver, const char *member);
    bool	disconnect(const QObject *receiver, const char *member=0);

    void	activate();

    bool	isBlocked()	 const		{ return QObject::signalsBlocked(); }
    void	block(bool b)		{ QObject::blockSignals(b); }
#ifndef QT_NO_VARIANT
    void	setParameter(int value);
    int		parameter() const;
#endif

#ifndef QT_NO_VARIANT
    void	setValue(const QVariant &value);
    QVariant	value() const;
#endif
Q_SIGNALS:
#ifndef QT_NO_VARIANT
    void signal(const QVariant&);
#endif
    void intSignal(int);

private:
    Q_DISABLE_COPY(Q3Signal)

#ifndef QT_NO_VARIANT
    QVariant val;
#endif

};

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3SIGNAL_H

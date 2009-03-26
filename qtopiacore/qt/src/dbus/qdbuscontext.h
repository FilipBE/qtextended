/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
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

#ifndef QDBUSCONTEXT_H
#define QDBUSCONTEXT_H

#include <QtCore/qstring.h>
#include <QtDBus/qdbuserror.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(DBus)

class QDBusConnection;
class QDBusMessage;

class QDBusContextPrivate;
class QDBUS_EXPORT QDBusContext
{
public:
    QDBusContext();
    ~QDBusContext();

    bool calledFromDBus() const;
    QDBusConnection connection() const;
    const QDBusMessage &message() const;

    // convenience methods
    bool isDelayedReply() const;
    // yes, they are const, so that you can use them even from const methods
    void setDelayedReply(bool enable) const;
    void sendErrorReply(const QString &name, const QString &msg = QString()) const;
    void sendErrorReply(QDBusError::ErrorType type, const QString &msg = QString()) const;

private:
    QDBusContextPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QDBusContext)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif

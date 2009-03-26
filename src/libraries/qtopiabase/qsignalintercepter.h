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

#ifndef QSIGNALINTERCEPTER_H
#define QSIGNALINTERCEPTER_H

#include <qobject.h>
#include <qvariant.h>
#include <qlist.h>
#include <qtopiaglobal.h>

class QSignalIntercepterPrivate;

class QTOPIABASE_EXPORT QSignalIntercepter : public QObject
{
    // Note: Do not put Q_OBJECT here.
    friend class QSlotInvoker;
    friend class QCopProxy;
public:
    QSignalIntercepter( QObject *sender, const QByteArray& signal,
                        QObject *parent=0 );
    ~QSignalIntercepter();

    QObject *sender() const;
    QByteArray signal() const;

    bool isValid() const;

    static const int QVariantId = -243;

    static int *connectionTypes( const QByteArray& member, int& nargs );

protected:
    int qt_metacall( QMetaObject::Call c, int id, void **a );
    virtual void activated( const QList<QVariant>& args ) = 0;

private:
    QSignalIntercepterPrivate *d;

    static int typeFromName( const QByteArray& name );
};

#endif

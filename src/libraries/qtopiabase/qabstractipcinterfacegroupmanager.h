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

#ifndef QABSTRACTIPCINTERFACEGROUPMANAGER_H
#define QABSTRACTIPCINTERFACEGROUPMANAGER_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class QAbstractIpcInterfaceGroupManagerPrivate;

class QTOPIABASE_EXPORT QAbstractIpcInterfaceGroupManager : public QObject
{
    Q_OBJECT
public:
    explicit QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, QObject *parent = 0 );
    explicit QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, const QString& interfaceName, QObject* parent = 0 );
    ~QAbstractIpcInterfaceGroupManager();

    QStringList groups() const;
    QStringList interfaces( const QString& group ) const;

    template <typename T> QStringList supports() const
        { return supports
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

    template <typename T> bool supports( const QString& group ) const
        { return supports
            ( group,
              reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

    template <typename T> int priority( const QString& group ) const
        { return priority
            ( group,
              reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

signals:
    void groupsChanged();
    void groupAdded( const QString& group );
    void groupRemoved( const QString& group );

protected:
    void connectNotify( const char *signal );

private slots:
    void groupsChangedInner();

private:
    QAbstractIpcInterfaceGroupManagerPrivate *d;

    QStringList supports( const QString& iface ) const;
    bool supports( const QString& group, const QString& iface ) const;
    int priority( const QString& group, const QString& iface ) const;
};

#endif

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

#ifndef QABSTRACTIPCINTERFACEGROUP_H
#define QABSTRACTIPCINTERFACEGROUP_H

#include <qabstractipcinterface.h>

class QAbstractIpcInterfaceGroupPrivate;

class QTOPIABASE_EXPORT QAbstractIpcInterfaceGroup : public QObject
{
    Q_OBJECT
public:
    explicit QAbstractIpcInterfaceGroup( const QString& groupName, QObject *parent = 0 );
    ~QAbstractIpcInterfaceGroup();

    QString groupName() const;

    virtual void initialize();

    template <typename T> bool supports() const
        { return _supports
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }
    template <typename T> T *interface() const
        { return qobject_cast<T *>( _interface
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ) ); }

protected:
    virtual void addInterface( QAbstractIpcInterface *interface );
    template <typename T> void suppressInterface()
        { _suppressInterface
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

private slots:
    void interfaceDestroyed();

private:
    bool _supports( const char *interfaceName ) const;
    QAbstractIpcInterface *_interface( const char *interfaceName ) const;
    void _suppressInterface( const char *interfaceName );

    QAbstractIpcInterfaceGroupPrivate *d;
};

#endif

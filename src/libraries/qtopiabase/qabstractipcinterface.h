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

#ifndef QABSTRACTIPCINTERFACE_H
#define QABSTRACTIPCINTERFACE_H

#include <qtopiaipcadaptor.h>

class QAbstractIpcInterfacePrivate;
class QAbstractIpcInterfaceGroup;

class QTOPIABASE_EXPORT QAbstractIpcInterface : public QObject
{
    Q_OBJECT
    friend class QAbstractIpcInterfaceGroup;
public:
    enum Mode
    {
        Client,
        Server,
        Invalid
    };

    QAbstractIpcInterface
        ( const QString& valueSpaceLocation,
          const QString& interfaceName,
          const QString& groupName = QString(),
          QObject *parent = 0, QAbstractIpcInterface::Mode mode = Client );
    ~QAbstractIpcInterface();

    QString groupName() const;
    QString interfaceName() const;
    QAbstractIpcInterface::Mode mode() const;
    bool available() const { return ( mode() != Invalid ); }

protected:
    enum SyncType
    {
        Immediate,
        Delayed
    };

    void setPriority( int value );

    void proxy( const QByteArray& member );
    void proxyAll( const QMetaObject& meta );
    void proxyAll( const QMetaObject& meta, const QString& subInterfaceName );

    QtopiaIpcSendEnvelope invoke( const QByteArray& name );
    void invoke( const QByteArray& name, QVariant arg1 );
    void invoke( const QByteArray& name, QVariant arg1, QVariant arg2 );
    void invoke( const QByteArray& name, QVariant arg1, QVariant arg2, QVariant arg3 );
    void invoke( const QByteArray& name, const QList<QVariant>& args );

    void setValue
        ( const QString& name, const QVariant& value,
          QAbstractIpcInterface::SyncType sync=Immediate );
    QVariant value
        ( const QString& name, const QVariant& def = QVariant() ) const;
    void removeValue
        ( const QString& name,
          QAbstractIpcInterface::SyncType sync=Immediate );
    QList<QString> valueNames( const QString& path = QString() ) const;

    virtual void groupInitialized( QAbstractIpcInterfaceGroup *group );

    void connectNotify( const char *signal );

signals:
    void disconnected();

private slots:
    void remoteDisconnected();

private:
    QAbstractIpcInterfacePrivate *d;
};

#endif

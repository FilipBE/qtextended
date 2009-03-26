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

#ifndef QPINMANAGER_H
#define QPINMANAGER_H

#include <qcomminterface.h>
#include <qtopiaapplication.h>

class QPinOptionsPrivate;

class QTOPIAPHONE_EXPORT QPinOptions
{
public:
    QPinOptions();
    QPinOptions( const QPinOptions& other );
    ~QPinOptions();

    enum Format
    {
        Number,
        PhoneNumber,
        Words,
        Text
    };

    QPinOptions& operator=( const QPinOptions& other );

    QString prompt() const;
    void setPrompt( const QString& value );

    QPinOptions::Format format() const;
    void setFormat( QPinOptions::Format value );

    int minLength() const;
    void setMinLength( int value );

    int maxLength() const;
    void setMaxLength( int value );

    bool canCancel() const;
    void setCanCancel( bool value );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QPinOptionsPrivate *d;
};

class QTOPIAPHONE_EXPORT QPinManager : public QCommInterface
{
    Q_OBJECT
public:
    explicit QPinManager( const QString& service = QString(),
                          QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QPinManager();

    enum Status
    {
        NeedPin,
        NeedPuk,
        Valid,
        Locked
    };

public slots:
    virtual void querySimPinStatus();
    virtual void enterPin( const QString& type, const QString& pin );
    virtual void enterPuk( const QString& type, const QString& puk,
                           const QString& newPin );
    virtual void cancelPin( const QString& type );
    virtual void changePin( const QString& type, const QString& oldPin,
                            const QString& newPin );
    virtual void requestLockStatus( const QString& type );
    virtual void setLockStatus
        ( const QString& type, const QString& password, bool enabled );

signals:
    void pinStatus( const QString& type, QPinManager::Status status,
                    const QPinOptions& options );
    void changePinResult( const QString& type, bool valid );
    void lockStatus( const QString& type, bool enabled );
    void setLockStatusResult( const QString& type, bool valid );
};

Q_DECLARE_USER_METATYPE_ENUM(QPinManager::Status)
Q_DECLARE_USER_METATYPE(QPinOptions)

#endif

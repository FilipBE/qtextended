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
#ifndef QSIMCONTROLEVENT_H
#define QSIMCONTROLEVENT_H

#include <qtopiaipcmarshal.h>

class QSimControlEventPrivate;

class QTOPIAPHONE_EXPORT QSimControlEvent
{
public:
    QSimControlEvent();
    QSimControlEvent( const QSimControlEvent& value );
    ~QSimControlEvent();

    enum Type
    {
        Call  = 0,
        Sms   = 1
    };

    enum Result
    {
        Allowed                     = 0,
        NotAllowed                  = 1,
        AllowedWithModifications    = 2
    };

    QSimControlEvent::Type type() const;
    void setType( QSimControlEvent::Type value );

    QSimControlEvent::Result result() const;
    void setResult( QSimControlEvent::Result value );

    QString text() const;
    void setText( const QString& value );

    QByteArray extensionData() const;
    void setExtensionData( QByteArray value );

    QByteArray extensionField( int tag ) const;
    void addExtensionField( int tag, const QByteArray& value );

    static QSimControlEvent fromPdu( QSimControlEvent::Type type, const QByteArray& pdu );
    QByteArray toPdu() const;

    QSimControlEvent& operator=( const QSimControlEvent & );
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSimControlEventPrivate *d;
};

Q_DECLARE_USER_METATYPE(QSimControlEvent)

#endif

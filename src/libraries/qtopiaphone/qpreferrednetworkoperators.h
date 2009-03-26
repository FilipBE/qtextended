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

#ifndef QPREFERREDNETWORKOPERATORS_H
#define QPREFERREDNETWORKOPERATORS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QPreferredNetworkOperators : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(List)
public:
    explicit QPreferredNetworkOperators( const QString& service = QString::null,
                                         QObject *parent = 0,
                                         QCommInterface::Mode mode = Client );
    ~QPreferredNetworkOperators();

    enum List
    {
        Current,
        UserControlled,
        OperatorControlled,
        HPLMN
    };

    struct Info
    {
        uint index;
        uint format;
        uint id;
        QString name;
        QStringList technologies;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

    struct NameInfo
    {
        QString name;
        uint id;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

    static QList<QPreferredNetworkOperators::Info> resolveNames
            ( const QList<QPreferredNetworkOperators::Info>& opers,
              const QList<QPreferredNetworkOperators::NameInfo>& names );

public slots:
    virtual void requestOperatorNames();
    virtual void requestPreferredOperators
        ( QPreferredNetworkOperators::List list );
    virtual void writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper );

signals:
    void operatorNames
        ( const QList<QPreferredNetworkOperators::NameInfo>& names );
    void preferredOperators
        ( QPreferredNetworkOperators::List list,
          const QList<QPreferredNetworkOperators::Info>& opers );
    void writePreferredOperatorResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QPreferredNetworkOperators::List)
Q_DECLARE_USER_METATYPE(QPreferredNetworkOperators::Info)
Q_DECLARE_USER_METATYPE(QPreferredNetworkOperators::NameInfo)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::NameInfo>)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::Info>)

#endif

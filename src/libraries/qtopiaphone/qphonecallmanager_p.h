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
#ifndef QPHONECALLMANAGER_P_H
#define QPHONECALLMANAGER_P_H

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

#include <qphonecallmanager.h>
#include <qphonecallprovider.h>
#include <qtopiaipcadaptor.h>
#include <qvaluespace.h>
#include <qmap.h>

class QPhoneCallManagerPrivate : public QObject
{
    Q_OBJECT
    friend class QPhoneCall;
    friend class QPhoneCallPrivate;
public:
    explicit QPhoneCallManagerPrivate( QObject *parent = 0 );
    ~QPhoneCallManagerPrivate();

    static QPhoneCallManagerPrivate *instance();

    QPhoneCall create( const QString& service, const QString& callType );
    void loadCallTypes();

    QList<QPhoneCall> calls;
    QValueSpaceItem *item;
    QStringList callTypes;
    QMap< QString, QStringList > callTypeMap;

signals:
    void newCall( const QPhoneCall& call );
    void callTypesChanged();
    void statesChanged( const QList<QPhoneCall>& calls );

private slots:
    void callStateChanged( const QString& identifier, QPhoneCall::State state,
                           const QString& number, const QString& service,
                           const QString& callType, int actions );
    void callStateTransaction( const QByteArray& transaction );
    void trackStateChanged( const QPhoneCall& call );
    void contentsChanged();

private:
    QtopiaIpcAdaptor *request;
    QtopiaIpcAdaptor *response;
};

#endif

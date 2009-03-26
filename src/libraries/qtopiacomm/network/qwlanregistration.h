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

#ifndef QWLANREGISTRATION_H
#define QWLANREGISTRATION_H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QCommInterface>
#include <qtopiaglobal.h>

class QWlanRegistrationPrivate;
class QTOPIACOMM_EXPORT QWlanRegistration : public QCommInterface
{
    Q_OBJECT
public:
    explicit QWlanRegistration( const QString& devHandle,
                        QObject* parent = 0, QAbstractIpcInterface::Mode mode = QAbstractIpcInterface::Client );
    ~QWlanRegistration();

    QString currentESSID() const;
    QStringList knownESSIDs() const;

signals:
    void accessPointChanged();

private:
    QWlanRegistrationPrivate* dptr;
};
#endif //NO_WIRELESS_LAN
#endif

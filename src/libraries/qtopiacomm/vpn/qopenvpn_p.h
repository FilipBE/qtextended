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

#ifndef QOPENVPN_P_H
#define QOPENVPN_P_H

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

#ifndef QTOPIA_NO_OPENVPN

#include "qvpnclient.h"
#include <QProcess>

class QOpenVPN : public QVPNClient
{
    Q_OBJECT
public:
    explicit QOpenVPN( QObject* parent = 0 );
    explicit QOpenVPN( bool serverMode, uint vpnID, QObject* parent = 0 );
    ~QOpenVPN();

    QVPNClient::Type type() const;
    QVPNClient::State state() const;
    QString name() const;

    QDialog* configure( QWidget* parent = 0 );

    void connect();
    void disconnect();

    void cleanup();

protected:
    void timerEvent( QTimerEvent* e );

private slots:
    void stateChanged( QProcess::ProcessState newState );
    void stateChanged();

private:
    int logIndex;
    int pendingID;
    int killID;
    QStringList parameter(bool *error) const;
    QVPNClient::State lastState;
};

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QTabWidget;

class GeneralOpenVPNPage;
class CertificateOpenVPNPage;
class OptionsOpenVPNPage;

class QOpenVPNDialog : public QDialog
{
    Q_OBJECT
public:
    QOpenVPNDialog( const QString& config, QWidget* parent = 0 );
    ~QOpenVPNDialog();

public slots:
    void accept();
private:
    void init();

private slots:
    void listSelected(QListWidgetItem*);
    void showUserHint( QListWidgetItem*, QListWidgetItem* );

private:
    QString config;
    QLabel* userHint;
    QStackedWidget* stack;
    QListWidget* list;
};
#endif //QTOPIA_NO_OPENVPN
#endif

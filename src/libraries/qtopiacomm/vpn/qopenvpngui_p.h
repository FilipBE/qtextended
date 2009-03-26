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

#ifndef QOPENVPNGUI_P_H
#define QOPENVPNGUI_P_H

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

#include <QWidget>
#include <QHash>
#include <qcontent.h>

class VPNConfigWidget : public QWidget {
    Q_OBJECT
public:
    explicit VPNConfigWidget( QWidget* parent = 0 )
        : QWidget( parent )
    {
    }

    ~VPNConfigWidget()
    {
    }

    void setConfigFile( const QString& cfg )
    {
        config = cfg;
        init();
    }

    virtual void init() = 0;
    virtual void save() = 0;
protected:
    void showEvent( QShowEvent* e ) {
        init();
        QWidget::showEvent(e);
    }

    void hideEvent( QHideEvent* e ) {
        save();
        QWidget::hideEvent( e );
    }

protected:
    QString config;
};


#include "ui_generalopenvpnbase.h"
class GeneralOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    GeneralOpenVPNPage( QWidget* parent = 0 );
    ~GeneralOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void forceRemoteName();

private:
    Ui::GeneralOpenVPNBase ui;
};

#include "ui_deviceopenvpnbase.h"
class DeviceOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    DeviceOpenVPNPage( QWidget* parent = 0 );
    ~DeviceOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void resetRemoteLabel( int newDevType );
private:
    Ui::DeviceOpenVPNBase ui;
};

#include "ui_certificateopenvpnbase.h"
class CertificateOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    CertificateOpenVPNPage( QWidget* parent = 0 );
    ~CertificateOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void authenticationChanged( int idx );
    void selectFile( );
private:
    Ui::CertificateOpenVPNBase ui;

    QHash<QToolButton*,QContent> toDocument;
};

#include "ui_optionsopenvpnbase.h"
class OptionsOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    OptionsOpenVPNPage( QWidget* parent = 0 );
    ~OptionsOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void selectConfigScript();
private:
    Ui::OptionsOpenVPNBase ui;
    QContent configScript;
};

#endif //QTOPIA_NO_OPENVPN
#endif

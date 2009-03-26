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
#ifndef CALLCONTROLS_H
#define CALLCONTROLS_H

#include <QWidget>

class ConnectionDialog;
class QHostAddress;
class QIcon;
class QLabel;
class QMediaRtpPayload;
class QMediaRtpStream;
class QPushButton;
class QToolButton;

class CallControls : public QWidget
{
    Q_OBJECT
public:
    CallControls(QMediaRtpStream *stream, QWidget *parent = 0);

    void setInboundLabel(const QString &text);
    void setInboundIcon(const QIcon &icon);
    void setInboundHost(const QHostAddress &address, int port);

    void setOutboundLabel(const QString &text);
    void setOutboundIcon(const QIcon &icon);
    void setOutboundHost(const QHostAddress &address, int port);
    void setOutboundPayloads(const QList<QMediaRtpPayload> &payloads);

private slots:
    void connectInbound(bool connect);
    void inboundConnected();
    void inboundDisconnected();
    void inboundPayloadChanged(const QMediaRtpPayload &payload);

    void connectOutbound(bool connect);
    void outboundConnected();
    void outboundDisconnected();

private:
    QMediaRtpStream *m_stream;
    QToolButton *m_inboundButton;
    QLabel *m_inboundLabel;
    ConnectionDialog *m_inboundDialog;
    QToolButton *m_outboundButton;
    QLabel *m_outboundLabel;
    ConnectionDialog *m_outboundDialog;
};

#endif

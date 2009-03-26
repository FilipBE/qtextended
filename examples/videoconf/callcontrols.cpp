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
#include "callcontrols.h"

#include "payloadmodel.h"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMediaRtpStream>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QToolButton>
#include <QtopiaApplication>

class ConnectionDialog : public QDialog
{
    Q_OBJECT
public:
    ConnectionDialog(bool outbound, QWidget *parent = 0);

    QHostAddress hostAddress() const;
    int hostPort() const;
    void setHost(const QHostAddress &address, int port);

    QMediaRtpPayload selectedPayload() const;
    void setSelectedPayload(const QMediaRtpPayload &payload);

    QList<QMediaRtpPayload> payloads() const;
    void setPayloads(const QList<QMediaRtpPayload> &payloads);

    QMediaRtpPayload telephonyEventPayload() const;
    void setTelephonyEventPayload(const QMediaRtpPayload &payload);

private:
    QLineEdit *m_addressEdit;
    QSpinBox *m_portEdit;
    QComboBox *m_telephonyEventEdit;
    PayloadModel *m_payloadModel;
    QMediaRtpPayload m_telephonyEventPayload;
};

ConnectionDialog::ConnectionDialog(bool outbound, QWidget *parent)
    : QDialog(parent)
    , m_addressEdit(0)
    , m_portEdit(0)
    , m_telephonyEventEdit(0)
    , m_payloadModel(0)
{
    QFormLayout *layout = new QFormLayout;

    if (outbound) {
        m_payloadModel = new PayloadModel(this);

        QTableView *payloadView = new QTableView;
        payloadView->setModel(m_payloadModel);

        m_telephonyEventEdit = new QComboBox;
        m_telephonyEventEdit->addItem(tr("Tone"));
        m_telephonyEventEdit->addItem(tr("Packet"));

        layout->addRow(tr("Payload"), payloadView);
        layout->addRow(tr("DTMF"), m_telephonyEventEdit);

        m_telephonyEventEdit->hide();
    }

    m_addressEdit = new QLineEdit;
    QtopiaApplication::setInputMethodHint(m_addressEdit, QLatin1String("netmask"));

    m_portEdit = new QSpinBox;
    m_portEdit->setRange(1025, 65535);

    layout->addRow(tr("IP Address"), m_addressEdit);
    layout->addRow(tr("Port"), m_portEdit);


    setLayout(layout);
}

QHostAddress ConnectionDialog::hostAddress() const
{
    return QHostAddress(m_addressEdit->text());
}

int ConnectionDialog::hostPort() const
{
    return m_portEdit->value();
}

void ConnectionDialog::setHost(const QHostAddress &address, int port)
{
    m_addressEdit->setText(address.toString());
    m_portEdit->setValue(port);
}

QMediaRtpPayload ConnectionDialog::selectedPayload() const
{
    return m_payloadModel
            ? m_payloadModel->selectedPayload()
            : QMediaRtpPayload();
}

void ConnectionDialog::setSelectedPayload(const QMediaRtpPayload &payload)
{
    if (m_payloadModel)
        m_payloadModel->setSelectedPayload(payload);
}

QList<QMediaRtpPayload> ConnectionDialog::payloads() const
{
    return m_payloadModel
            ? m_payloadModel->payloads()
            : QList<QMediaRtpPayload>();
}

void ConnectionDialog::setPayloads(const QList<QMediaRtpPayload> &payloads)
{
    if (m_payloadModel) {
        QList<QMediaRtpPayload> filteredPayloads;

        m_telephonyEventPayload = QMediaRtpPayload();

        foreach (QMediaRtpPayload payload, payloads) {
            if (payload.encodingName().compare(
                QLatin1String("telephone-event"), Qt::CaseInsensitive) == 0) {
                m_telephonyEventPayload = payload;
            } else {
                filteredPayloads.append(payload);
            }
        }
        m_payloadModel->setPayloads(filteredPayloads);

        m_telephonyEventEdit->setVisible(!m_telephonyEventPayload.isNull());
    }
}

QMediaRtpPayload ConnectionDialog::telephonyEventPayload() const
{
    return m_telephonyEventEdit && m_telephonyEventEdit->currentIndex() == 1
            ? m_telephonyEventPayload
            : QMediaRtpPayload();
}

void ConnectionDialog::setTelephonyEventPayload(const QMediaRtpPayload &payload)
{
    if (m_telephonyEventEdit) {
        if (!payload.isNull()) {
            m_telephonyEventPayload = payload;

            m_telephonyEventEdit->setCurrentIndex(1);
        } else {
            m_telephonyEventEdit->setCurrentIndex(0);
        }
    }
}

CallControls::CallControls(QMediaRtpStream *stream, QWidget *parent)
    : QWidget(parent)
    , m_stream(stream)
    , m_inboundButton(0)
    , m_inboundLabel(0)
    , m_inboundDialog(0)
    , m_outboundButton(0)
    , m_outboundLabel(0)
    , m_outboundDialog(0)
{
    m_inboundButton = new QToolButton;
    m_inboundButton->setCheckable(true);
    m_inboundLabel = new QLabel;
    m_outboundButton = new QToolButton;
    m_outboundButton->setCheckable(true);
    m_outboundLabel = new QLabel;

    QFormLayout *layout = new QFormLayout;
    layout->setMargin(0);
    layout->addRow(m_inboundButton, m_inboundLabel);
    layout->addRow(m_outboundButton, m_outboundLabel);
    setLayout(layout);

    if (m_stream) {
        m_inboundDialog = new ConnectionDialog(false, this);
        m_outboundDialog = new ConnectionDialog(true, this);

        connect(m_inboundButton, SIGNAL(clicked(bool)), this, SLOT(connectInbound(bool)));
        connect(m_outboundButton, SIGNAL(clicked(bool)), this, SLOT(connectOutbound(bool)));

        connect(m_stream, SIGNAL(inboundConnected()), this, SLOT(inboundConnected()));
        connect(m_stream, SIGNAL(inboundDisconnected()), this, SLOT(inboundDisconnected()));
        connect(m_stream, SIGNAL(inboundPayloadChanged(QMediaRtpPayload)),
                this, SLOT(inboundPayloadChanged(QMediaRtpPayload)));

        connect(m_stream, SIGNAL(outboundConnected()), this, SLOT(outboundConnected()));
        connect(m_stream, SIGNAL(outboundDisconnected()), this, SLOT(outboundDisconnected()));
    } else {
        m_inboundButton->setEnabled(false);
        m_outboundButton->setEnabled(false);
    }
}

void CallControls::setInboundLabel(const QString &text)
{
    m_inboundButton->setText(text);
}

void CallControls::setInboundIcon(const QIcon &icon)
{
    m_inboundButton->setIcon(icon);
}

void CallControls::setInboundHost(const QHostAddress &address, int port)
{
    if (m_inboundDialog)
        m_inboundDialog->setHost(address, port);
}

void CallControls::setOutboundLabel(const QString &text)
{
    m_outboundButton->setText(text);
}

void CallControls::setOutboundIcon(const QIcon &icon)
{
    m_outboundButton->setIcon(icon);
}

void CallControls::setOutboundHost(const QHostAddress &address, int port)
{
    if (m_outboundDialog)
        m_outboundDialog->setHost(address, port);
}

void CallControls::setOutboundPayloads(const QList<QMediaRtpPayload> &payloads)
{
    if (m_outboundDialog)
        m_outboundDialog->setPayloads(payloads);
}

void CallControls::connectInbound(bool connect)
{
    if (connect) {
        if (QtopiaApplication::execDialog(m_inboundDialog) == QDialog::Accepted) {
            m_inboundButton->setEnabled(false);

            m_stream->connectInbound(
                    m_inboundDialog->hostAddress(), m_inboundDialog->hostPort());
        } else {
            m_inboundButton->setChecked(false);
        }
    } else {
        m_inboundButton->setEnabled(false);

        m_stream->disconnectInbound();
    }
}

void CallControls::inboundConnected()
{
    m_inboundButton->setEnabled(true);
}

void CallControls::inboundDisconnected()
{
    m_inboundButton->setEnabled(true);
    m_inboundButton->setChecked(false);
}

void CallControls::inboundPayloadChanged(const QMediaRtpPayload &payload)
{
    m_inboundLabel->setText(payload.encodingName());
}

void CallControls::connectOutbound(bool connect)
{
    if (connect) {
        if (QtopiaApplication::execDialog(m_outboundDialog) == QDialog::Accepted) {
            m_outboundButton->setEnabled(false);
            m_outboundLabel->setText(m_outboundDialog->selectedPayload().encodingName());

            if (m_stream->type() == QMediaRtpStream::Audio) {
                static_cast<QMediaRtpAudioStream *>(m_stream)->setTelephonyEventPayload(
                        m_outboundDialog->telephonyEventPayload());
            }

            m_stream->setOutboundPayload(m_outboundDialog->selectedPayload());
            m_stream->connectOutbound(
                    m_outboundDialog->hostAddress(), m_outboundDialog->hostPort());
        } else {
            m_outboundButton->setChecked(false);
        }
    } else {
        m_outboundButton->setEnabled(false);

        m_stream->disconnectOutbound();
    }
}

void CallControls::outboundConnected()
{
    m_outboundButton->setEnabled(true);
}

void CallControls::outboundDisconnected()
{
    m_outboundButton->setEnabled(true);
    m_outboundButton->setChecked(false);
}

#include "callcontrols.moc"

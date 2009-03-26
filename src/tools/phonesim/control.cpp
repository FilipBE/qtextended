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

#include "control.h"
#include "ui_controlbase.h"
#include <qpushbutton.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <QtGui/qmessagebox.h>
#include <qfiledialog.h>
#include <Qt>
#include <qbuffer.h>
#include <qtimer.h>
#include <qevent.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include "attranslator.h"

#define TWO_BYTE_MAX 65535
#define FOUR_CHAR 4
#define HEX_BASE 16

class ControlWidget : public QWidget
{
Q_OBJECT
public:
    ControlWidget( const QString&, Control*);

    void handleFromData( const QString& );
    void handleToData( const QString& );

private slots:
    void sendSQ();
    void sendBC();
    void chargingChanged(int state);
    void sendOPS();
    void sendREG();
    void sendCBM();
    void sendSMSMessage();
    void sendMGD();
    void selectFile();
    void sendSMSDatagram();
    void sendCall();
    void atChanged();
    void resetTranslator();

signals:
    void unsolicitedCommand(const QString &);
    void command(const QString &);
    void variableChanged(const QString &, const QString &);
    void switchTo(const QString &);
    void startIncomingCall(const QString &);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui_ControlBase *ui;
    Control *p;
    AtTranslator *translator;
};
#include "control.moc"


ControlWidget::ControlWidget(const QString &ruleFile, Control *parent)
    : QWidget(), p(parent)
{
    QFileInfo info( ruleFile );
    QString specFile = info.absolutePath() + "/GSMSpecification.xml";
    if (!QFile::exists(specFile))
        specFile = QDir::currentPath() + "/GSMSpecification.xml";
    translator = new AtTranslator(specFile);

    ui = new Ui_ControlBase;
    ui->setupUi(this);

    connect(ui->hsSignalQuality, SIGNAL(valueChanged(int)), this, SLOT(sendSQ()));
    connect(ui->hsBatteryCharge, SIGNAL(valueChanged(int)), this, SLOT(sendBC()));
    connect(ui->hsBatteryCharging, SIGNAL(stateChanged(int)), this, SLOT(chargingChanged(int)));
    connect(ui->pbSelectOperator, SIGNAL(clicked()), this, SLOT(sendOPS()));
    connect(ui->pbRegistration, SIGNAL(clicked()), this, SLOT(sendREG()));
    connect(ui->pbSendCellBroadcast, SIGNAL(clicked()), this, SLOT(sendCBM()));
    connect(ui->pbSendSMSMessage, SIGNAL(clicked()), this, SLOT(sendSMSMessage()));
    connect(ui->pbFile, SIGNAL(clicked()), this, SLOT(selectFile()));
    connect(ui->pbSendSMSDatagram, SIGNAL(clicked()), this, SLOT(sendSMSDatagram()));
    connect(ui->pbIncomingCall, SIGNAL(clicked()), this, SLOT(sendCall()));
    connect(ui->openSpecButton, SIGNAL(clicked()), this, SLOT(resetTranslator()));
    connect(ui->atCheckBox, SIGNAL(clicked()), this, SLOT(atChanged()));

    show();
}

void ControlWidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}

Control::Control(const QString& ruleFile, QObject *parent)
        : HardwareManipulator(parent), widget(new ControlWidget(ruleFile, this))
{
    QList<QByteArray> proxySignals;
    proxySignals
        << SIGNAL(unsolicitedCommand(QString))
        << SIGNAL(command(QString))
        << SIGNAL(variableChanged(QString,QString))
        << SIGNAL(switchTo(QString))
        << SIGNAL(startIncomingCall(QString));

    foreach (QByteArray sig, proxySignals)
        connect(widget, sig, this, sig);
}

Control::~Control()
{
    delete widget;
}

void Control::setPhoneNumber( const QString &number )
{
    widget->setWindowTitle("Phonesim - Number: " + number);
}

void Control::warning( const QString &title, const QString &message )
{
    QMessageBox::warning(widget, title, message, "OK");
}

void ControlWidget::sendSQ()
{
    emit variableChanged("SQ",QString::number(ui->hsSignalQuality->value())+",99");
    emit unsolicitedCommand("+CSQ: "+QString::number(ui->hsSignalQuality->value())+",99");
}


void ControlWidget::sendBC()
{
    bool charging = ui->hsBatteryCharging->checkState() == Qt::Checked;
    emit variableChanged("BC",QString::number(charging)+","+QString::number(ui->hsBatteryCharge->value()));
    emit unsolicitedCommand("+CBC: "+QString::number(charging)+","+QString::number(ui->hsBatteryCharge->value()));
}

void ControlWidget::chargingChanged(int state)
{
    bool charging = state  == Qt::Checked;
    ui->hsBatteryCharge->setEnabled(!charging);
    emit variableChanged("BC",QString::number(charging)+","+QString::number(ui->hsBatteryCharge->value()));
    emit unsolicitedCommand("+CBC: "+QString::number(charging)+","+QString::number(ui->hsBatteryCharge->value()));
}

void ControlWidget::sendOPS()
{
    emit variableChanged("OP", ui->leOperatorName->text());
    emit unsolicitedCommand("+CREG: 5");
}

void ControlWidget::sendREG()
{
    QString commandString = "+CREG: "+QString::number(ui->cbRegistrationStatus->currentIndex());

    if ( ui->chkLocationInfo->checkState() == Qt::Checked ) {
        bool ok;

        int LAC = p->convertString(ui->leLAC->text(),TWO_BYTE_MAX,FOUR_CHAR,HEX_BASE, &ok);
        if (!ok) {
            p->warning(tr("Invalid LAC"),
                    tr("Location Area Code must be 4 hex digits long"));
            return;
        }

        int cellID = p->convertString(ui->leCellID->text(),TWO_BYTE_MAX,FOUR_CHAR,HEX_BASE, &ok);
        if (!ok) {
            p->warning(tr("Invalid Cell ID"),
                    tr("Cell ID must be 4 hex digits long"));
            return;
        }

        commandString.append("," + QString::number(LAC) + "," + QString::number(cellID));
    }

    emit unsolicitedCommand(commandString);
}

void ControlWidget::sendCBM()
{
    QString pdu = p->constructCBMessage(ui->leMessageCode->text(),ui->cbGeographicalScope->currentIndex(),
                       ui->leUpdateNumber->text(),ui->leChannel->text(),ui->leScheme->text(),
                       ui->cbLanguage->currentIndex(),ui->leNumPages->text(),ui->lePage->text(),
                       ui->teContent->toPlainText());

    emit unsolicitedCommand(QString("+CBM: ")+QString::number(pdu.length()/2)+'\r'+'\n'+ pdu);
}

void ControlWidget::sendSMSMessage()
{
    if (ui->leSMSClass->text().isEmpty()) {
        p->warning(tr("Invalid Sender"),
                tr("Sender must not be empty"));
        return;
    }

    if (ui->leSMSServiceCenter->text().isEmpty() || ui->leSMSServiceCenter->text().contains(QRegExp("\\D"))) {
        p->warning(tr("Invalid Service Center"),
                tr("Service Center must not be empty and contain "
                   "only digits"));
        return;
    }
    p->constructSMSMessage(ui->leSMSClass->text().toInt(), ui->leMessageSender->text(), ui->leSMSServiceCenter->text(), ui->teSMSText->toPlainText());
}

void ControlWidget::sendMGD()
{
    emit command("AT+CMGD=1");
}

void ControlWidget::selectFile()
{
    ui->leFile->setText(QFileDialog::getOpenFileName(this, "Select File", "/home", "Files (*.*)"));
}

void ControlWidget::sendSMSDatagram()
{
    QString portStr = ui->lePort->text();
    if ( portStr.contains(QRegExp("\\D")) ) {
        p->warning(tr("Invalid Port"), tr("Port number can contain only digits" ));
        return;
    }
    int port = portStr.toInt();

    QString sender = ui->leDatagramSender->text();

    //obtain data from either file or text edit
    QFile file(ui->leFile->text());
    QByteArray data;
    if ( ui->chkAppData->checkState() ==Qt::Checked ) {
        if ( !file.open(QIODevice::ReadOnly) ) {
            p->warning(tr("Invalid File"), tr("File could not be opened"));
            return;
        }
        data = file.readAll();
    } else {
       data = ui->teAppData->toPlainText().toUtf8();
    }

    //obtain Content-Type if required
    QByteArray contentType;
    if ( ui->chkContentType->checkState() == Qt::Checked ) {
        QString contentTypeStr = ui->leContentType->text();
        if ( contentTypeStr.length() == 0 ) {
            p->warning( tr("Invalid ContentType"),
                     tr("Please check the Content Type") );
            return;
        }
        contentType = contentTypeStr.toUtf8();
    }

    //construct and place SMS datagram in SMSList
    p->constructSMSDatagram(port, sender, data, contentType);
}

void ControlWidget::sendCall()
{
    emit startIncomingCall( ui->leCaller->text() );
}

void ControlWidget::handleFromData( const QString& cmd )
{
    ui->atViewer->append(cmd);
    /*QStringList dataList = cmd.split("\n");
    QString dataItem;
    foreach( dataItem, dataList ){
        if( dataItem != "" ){
            ui->atViewer->append(translator->translate(dataItem));
            ui->atViewer->append(dataItem);
        }
    }*/
}

void Control::handleFromData( const QString& cmd )
{
    widget->handleFromData(cmd);
}

void ControlWidget::handleToData( const QString& cmd )
{
    QStringList dataList = cmd.split("\n");
    QString dataItem;
    foreach( dataItem, dataList ){
        if( dataItem != "" ){
            ui->atViewer->append( dataItem + " : " + translator->translateCommand(dataItem) );
        }
    }
}

void Control::handleToData( const QString& cmd )
{
    widget->handleToData(cmd);
}

void ControlWidget::resetTranslator()
{
    QString fileName = QFileDialog::getOpenFileName(this,
     tr("Open Specification File"), QDir::homePath(), tr("Specification files (*.xml)"));
    if(fileName != 0)
        translator->resetSpecification(fileName);
}

void ControlWidget::atChanged()
{
    ui->atGroupBox->setVisible( ui->atCheckBox->isChecked() );
}


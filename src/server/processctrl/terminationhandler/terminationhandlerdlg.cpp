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

#include <QObject>
#include <QtopiaServiceRequest>
#include <QMessageBox>
#include "qterminationhandlerprovider.h"
#include "qtopiaserverapplication.h"
#include "qabstractmessagebox.h"

class TerminationDialogTask: public QObject
{
    Q_OBJECT
public:
    TerminationDialogTask( QObject* parent = 0 );

private slots:
    void applicationTerminated( const QString&, const QString& text,
                                const QPixmap &icon, const QString &buttonText,
                                QtopiaServiceRequest &buttonAction);
                                
};

TerminationDialogTask::TerminationDialogTask( QObject* parent )
    : QObject( parent )
{
    QTerminationHandlerProvider* thp = new QTerminationHandlerProvider(this);
    QObject::connect(thp, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)), 
                     this, SLOT(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)));
}

void TerminationDialogTask::applicationTerminated( const QString &, const QString &text,
        const QPixmap &icon, const QString &buttonText,
        QtopiaServiceRequest &buttonAction)
{
    bool executeAction = !buttonAction.isNull();
    if( !text.isEmpty() ) {
        bool ba = !buttonAction.isNull() && !buttonText.isEmpty();
        ba = true;
        QString error_title = tr("Application terminated");

        QAbstractMessageBox* box = QAbstractMessageBox::messageBox( 0, error_title, text, QAbstractMessageBox::Critical );
        //we cannot set the pixmap at this stage
        if(!icon.isNull() )
            box->setIconPixmap(icon);
        if ( ba )
            box->setButtons( tr("OK"), buttonText, QLatin1String(""), QAbstractMessageBox::Ok, -1  );
        int rv = QtopiaApplication::execDialog(box );
        if ( ba ) {
            //QAbstractMessageBox::setButtons(QString,QString) uses a slightly 
            //different concept for return values
            executeAction = (rv == 1);
        } else {
            executeAction = false;
        }
    }

    if(executeAction)
        buttonAction.send();
}
QTOPIA_TASK( TerminationDialogTask,TerminationDialogTask)

#include "terminationhandlerdlg.moc"

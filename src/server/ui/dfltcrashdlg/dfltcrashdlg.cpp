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

#include "dfltcrashdlg.h"
#include "qtopiaserverapplication.h"
#include "qabstractmessagebox.h"

/*!
  \class DefaultCrashDialogTask
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::GeneralUI
  \brief The DefaultCrashDialogTask class provides a crash dialog that notifies the
  user about crashing applications.

  The DefaultCrashDialogTask provides a Qt Extended Server Task. The crash dialog controlled by
  this task appears whenever an unfiltered application
  crash occurs.  An application crash may be filtered by the
  ApplicationTerminationHandler interface.  The default implementation is a
  standard message box alerting the user to the name of the crashed application.

  The dialog appears whenever the ApplicationLauncher::applicationTerminated()
  signal is emitted with the filtered parameter set to false.
 */

/*!
  \internal
  */
DefaultCrashDialogTask::DefaultCrashDialogTask( QObject* parent )
    : QObject( parent ), ata(0)
{
    ApplicationLauncher *launcher = qtopiaTask<ApplicationLauncher>();
    if(launcher)
        QObject::connect(launcher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)), this, SLOT(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)));
} 

/*!
  \internal
  */
DefaultCrashDialogTask::~DefaultCrashDialogTask()
{
    if (ata) {
        delete ata;
        ata = 0;
    }
}

/*!
  \internal
  Raise the "Application terminated" message in an information
  message box. Don't raise it if the application terminated
  normally or because it was killed.
 */
void DefaultCrashDialogTask::applicationTerminated(const QString &name,
                               ApplicationTypeLauncher::TerminationReason reason,
                               bool filtered)
{
    if (ApplicationTypeLauncher::Normal == reason ||
	ApplicationTypeLauncher::Killed == reason ||
	filtered)
        return;

    QContent app(name,false);
    if(app.isNull()) return;
    QString appname = Qtopia::dehyphenate(app.name());

    if ( !ata || !ata->isVisible() )
        ata_list.clear();

    if ( !ata ) {
        ata = QAbstractMessageBox::messageBox( 0, QString(), QString(), 
                                               QAbstractMessageBox::Critical );
        if ( !ata )
            return;
    }
    QString error_title;
    QString error_details;
    if ( !ata_list.contains(appname) )
        ata_list.append(appname);
    QString l = ata_list.join(", ");
    if ( ata_list.count() == 1 ) {
        error_title = tr("Application terminated");
        error_details = tr("<qt><b>%1</b> was terminated due to application error.</qt>").arg(l);
    } else {
        error_title = tr("Applications terminated");
        error_details = tr("<qt><b>%1</b> were terminated due to application errors.</qt>").arg(l);
    }

    ata->setTitle(error_title);
    ata->setText(error_details);
    QtopiaApplication::showDialog( ata );
}

QTOPIA_TASK( DefaultCrashDialogTask, DefaultCrashDialogTask );

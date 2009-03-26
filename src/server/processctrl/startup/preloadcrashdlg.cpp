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

#include "startupapps.h"
#include "qabstractmessagebox.h"
#include <QObject>

class PreloadCrashDlg : public QObject
{
    Q_OBJECT
public:
    PreloadCrashDlg(QObject* parent = 0) : QObject(parent)
    {
        StartupApplications* startup = qtopiaTask<StartupApplications>();
        if ( startup ) {
            connect(startup, SIGNAL(preloadCrashed(QString)),
                    this, SLOT(preloadCrashed(QString)));
        }
    }

private slots:
    void preloadCrashed(const QString& name)
    {
        QContent app(name,false);
        if(app.isNull()) return;
        QString appname = Qtopia::dehyphenate(app.name());

        QString error_title = tr("Application terminated");
        QString error_details = tr("<qt><b>%1</b> was terminated due to application error.  (Fast loading has been disabled. Select application icon and activate FastLoad option to reenable it.)</qt>").arg(appname);

        QAbstractMessageBox::information(0, error_title, error_details);
    }
};

QTOPIA_TASK(PreloadCrash,PreloadCrashDlg)
#include "preloadcrashdlg.moc"

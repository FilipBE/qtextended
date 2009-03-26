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
// #include <qtopia.h>

#include "memory.h"
#include "load.h"
#include "storage.h"
#include "versioninfo.h"
#include "sysinfo.h"
#include "dataview.h"
#include "securityinfo.h"
#include "networkinfo.h"

#ifdef QTOPIA_CELL
#include "siminfo.h"
#include "modeminfo.h"
#endif

#include <qsoftmenubar.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <QScrollArea>
#include <QTimer>
#include <QCoreApplication>

SystemInfo::SystemInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
    , wizard(0)
{
    setWindowTitle( tr("System Info") );

    QVBoxLayout *lay = new QVBoxLayout( this );
    lay->setMargin( 1 );
    tab = new QTabWidget( this );
    lay->addWidget( tab );

    tab->addTab( wrapWithScrollArea(new VersionInfo( tab )), tr("Version") );
    // we have the first tab created, so, delay creation of the other tabs until we've got time to start processing them.
    QTimer::singleShot(1, this, SLOT(delayedInit()));

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);

    service = new CleanupWizardService(this);
    wizard = 0;
}

void SystemInfo::startCleanupWizard()
{
    delete(wizard);
    wizard = new CleanupWizard(this);
    wizard->showMaximized();
}

QScrollArea *SystemInfo::wrapWithScrollArea(QWidget *widget)
{
    QScrollArea *sv = new QScrollArea();
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);
    sv->setWidget( widget );
    sv->setFocusPolicy( Qt::TabFocus );
    return sv;
}

void SystemInfo::delayedInit()
{
    tab->addTab( new StorageInfoView(tab), tr("Storage"));
    QCoreApplication::processEvents();
    tab->addTab( wrapWithScrollArea(new LoadInfo(tab)), tr("CPU") );
    QCoreApplication::processEvents();
    tab->addTab( wrapWithScrollArea(new MemoryInfo(tab)), tr("Memory") );
    QCoreApplication::processEvents();
    tab->addTab( wrapWithScrollArea(new DataView(tab)), tr("Data") );
    QCoreApplication::processEvents();
    tab->addTab( new SecurityInfo(tab), tr("Security") );
    QCoreApplication::processEvents();
#ifdef QTOPIA_CELL
    tab->addTab( new ModemInfo(tab), tr("Modem") );
    QCoreApplication::processEvents();
    tab->addTab( wrapWithScrollArea(new SimInfo(tab)), tr("SIM") );
    QCoreApplication::processEvents();
#endif
    tab->addTab( new NetworkInfoView(tab), tr("Network") );
}

/*!
    \service CleanupWizardService CleanupWizard
    \inpublicgroup QtEssentialsModule

    \brief The CleanupWizardService class provides the CleanupWizard service.

    The \i CleanupWizard service enables applications to pop up the
    cleanup wizard for deleting messages, events and documents.
*/
/*!
    \internal
*/
CleanupWizardService::~CleanupWizardService()
{
}

/*!
    Start the cleanup wizard. The wizard allows the deletion of documents,
    the cleanup of the mailbox and purges old (and finished) events.

    This slot corresponds to the QCop service message
    \c{CleanupWizard::showCleanupWizard()}.
*/
void CleanupWizardService::showCleanupWizard()
{
   parent->startCleanupWizard();
}


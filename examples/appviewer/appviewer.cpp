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

#include "appviewer.h"
#include <qdocumentselector.h>
#include <qtopianamespace.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QKeyEvent>
#include <QVBoxLayout>

/*
 *  Constructs a AppViewer which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
AppViewer::AppViewer( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    setWindowTitle( tr( "App Viewer" ));

    QVBoxLayout *vbox = new QVBoxLayout( this );

    textArea = new QTextEdit();
    textArea->setReadOnly(true);
    appSelector = new QDocumentSelector();
    appSelector->setFilter( QContentFilter(QContent::Application) );

    vbox->addWidget( appSelector );
    vbox->addWidget( textArea );

    connect( appSelector, SIGNAL(documentSelected(QContent)),
            this, SLOT(documentSelected(QContent)) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AppViewer::~AppViewer()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * Call the appSelector widget to allow the user to choose an application
 * to display information about
 */
void AppViewer::openApplicationInfo()
{
    QContentFilter appFilter = QContentFilter( QContent::Application );
    appSelector->setFilter( appFilter );
    appSelector->showMaximized();
}

/*
 * Respond to the documentSelected signal from the appSelector, by displaying
 * information about the selected application.
 */
void AppViewer::documentSelected( const QContent &appContent )
{
    if ( appContent.isValid() )
    {
        textArea->setHtml( getInformation( appContent ));
    }
    else
    {
        textArea->setHtml(
                tr( "<font color=\"#CC0000\">Could not find information about %1</font>" )
                .arg( appContent.name() ));
        qWarning() << "Application " << appContent.file() << " not found";
    }
}

/*
 * Find information about an Application, including what other installed applications
 * there are which have binaries bigger or smaller than this one.
 * Pre-requisite - the appContent is valid, ie has a backing file.
 */
QString AppViewer::getInformation( const QContent &appContent )
{
    QFileInfo fi( appContent.file() );
    QString info = tr( "Binary is: <b>%1</b><br>" ).arg( fi.fileName() );

    qint64 chosenAppSize = appContent.size();
    info += tr( "Size is: <b>%1 bytes</b><br>" ).arg( chosenAppSize );

    enum Count { SMALL, LARGE };
    int qtopiaCounts[2] = { 0, 0 };
    int packageCounts[2] = { 0, 0 };
    int *currentCount;
    QStringList paths = Qtopia::installPaths();
    foreach ( QString p, paths )
    {
        QDir qDir( p + "bin" );
        if ( qDir.exists() )
        {
            QFileInfoList binaries = qDir.entryInfoList( QDir::Executable );
            currentCount = ( p == Qtopia::packagePath() ) ? packageCounts : qtopiaCounts;
            foreach ( QFileInfo f, binaries )
                if ( f.size() > chosenAppSize )
                    ++currentCount[LARGE];
                else
                    ++currentCount[SMALL];
        }
    }

    info += tr( ", bigger than <font color=\"#CC0000\">%1 percent</font> of Qt Extended binaries" )
        .arg( (int)( (float)qtopiaCounts[SMALL] * 100.0 / (float)qtopiaCounts[LARGE] ));

    if ( packageCounts[SMALL] > 0 || packageCounts[LARGE] > 0 )
    {
        info += tr( ", and bigger than <font color=\"#CC0000\">%1 percent</font> of package binaries" )
            .arg( (int)( (float)packageCounts[SMALL] * 100.0 / (float)packageCounts[LARGE] ));
    }

    return info;
}

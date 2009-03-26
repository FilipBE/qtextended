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

#include "textviewer.h"
#include <qdocumentselector.h>
#include <qsoftmenubar.h>
#include <qpushbutton.h>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QDebug>
#include <QKeyEvent>

/*
 *  Constructs a TextViewer which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
    TextViewer::TextViewer( QWidget *parent, Qt::WFlags f )
: QMainWindow( parent, f )
{
    textArea = new QTextEdit(this);
    textArea->setReadOnly(true);
    setCentralWidget(textArea);
    docSelector = new QDocumentSelector();

    QAction *actionOpen = new QAction(tr("Open Document"), this );
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(openDocument()));
    QMenu* menu = QSoftMenuBar::menuFor(textArea);
    menu->addAction(actionOpen);
    connect(docSelector, SIGNAL(documentSelected(QContent)), this, SLOT(documentSelected(QContent)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
TextViewer::~TextViewer()
{
    delete docSelector;
    // no need to delete child widgets, Qt does it all for us
}


/*
 * A matter of convenience pressing Key_Select will cause a document to be opened
 */
void TextViewer::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Select){
        e->accept();
        openDocument();
    }else{
        QMainWindow::keyPressEvent(e);
    }
}

void TextViewer::openDocument()
{
    // request that the matching documents be sorted in reverse alphanumeric order
    QContentFilter docFilter = QContentFilter(QContentFilter::MimeType, "text/*");
    docSelector->setFilter(docFilter);
    docSelector->showMaximized();
}

void TextViewer::documentSelected(const QContent & docContent)
{
    // make use of the document selected by the QDocumentSelector widget
    docSelector->hide();
    if (docContent.isValid()){
        QFile f(docContent.file());
        if (f.open(QIODevice::ReadOnly)){
            QTextStream fstream(&f);
            textArea->setHtml(fstream.readAll());
        }else{
            qWarning() << "Unable to read content from file" << docContent.file();
        }

    }else{
        qWarning()<< "Document " << docContent.file() << " is invalid";
    }
}


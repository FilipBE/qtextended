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

#include <QtGui>
#include <QDateTime>
#include <QColorSelector>
#include <QLayout>
#include <QColorSelectorDialog>
#include <QSpinBox>
#include <QDialog>
#include <qmessagebox.h>

#include <qtopia/qtopiaapplication.h>
#include <qtopia/qsoftmenubar.h>
#include <QDocumentSelectorService>
#include <QMimeType>
#include <QImageDocumentSelectorDialog>
#include <QContent>

#include "mainwindow.h"
#include "scribblearea.h"

MainWindow::MainWindow( QWidget *parent , Qt::WFlags f )
        :  QMainWindow( parent, f )
        , selector( 0 )

{
    scribbleArea = new ScribbleArea(this);
    setCentralWidget(scribbleArea);
    createActions();
    createMenus();

    setWindowTitle(tr("Scribble"));

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::open()
{
    QImageDocumentSelectorDialog selectorDialog;

    if(QtopiaApplication::execDialog( &selectorDialog ) == QDialog::Accepted) {

        QContent document = selectorDialog.selectedDocument();
        QByteArray format = QMimeType::fromId( document.type()).extension().toUpper().toLatin1();

        scribbleArea->openImage( document.fileName());
    }
}

bool MainWindow::save()
{
    createSelector();

    QContent document = selector->selectedDocument();

    if (!document.isNull() && saveTypes.contains(document.type()) ) {
        if (selector->saveDocument(this)) {
            QByteArray format = QMimeType::fromId(selector->selectedDocument().type()).extension().toUpper().toLatin1();

            return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
        }
    } else {
        return saveAs();
    }
    return false;
}

bool MainWindow::saveAs()
{
    createSelector();

    QContent document = selector->selectedDocument();

    if (document.isNull()) {
        QString date = QString("Scribble %1").arg(QTimeString::localYMDHMS(QDateTime::currentDateTime(),QTimeString::Short));

        if (selector->newDocument( date,saveTypes,this)) {
            QByteArray format = QMimeType::fromId( selector->selectedDocument().type() ).extension().toUpper().toLatin1();

            return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
        }
    } else if (selector->newDocument(tr("Copy of %1").arg(document.name()),saveTypes,this)) {
        QByteArray format = QMimeType::fromId( selector->selectedDocument().type() ).extension().toUpper().toLatin1();

        return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
    }
    return false;
}

void MainWindow::penColor()
{
  cselect = new QColorSelectorDialog(this);
  connect(cselect,SIGNAL(selected(QColor)),
          this,SLOT(colorSelected(QColor)));

  cselect->showMaximized();
}

void MainWindow::penWidth()
{
    dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Pen Width"));

    QLayout *layout;
    layout = new QVBoxLayout(dialog);

    spinbox = new QSpinBox(dialog);
    spinbox->setMinimum(1);
    layout->addWidget(spinbox);

    connect(dialog,SIGNAL(accepted()),this,SLOT(setPenWidth()));

    dialog->showMaximized();

}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Scribble"),
            tr("<p><b>Scribble</b> is kewl</p>"));
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("Open..."), this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("Save"), this);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save As..."), this);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

//     printPdfAct = new QAction(tr("&Print as PDF"), this);
//     connect(printPdfAct, SIGNAL(triggered()), scribbleArea, SLOT(printPdf()));

    penColorAct = new QAction(tr("Pen Color..."), this);
    connect(penColorAct, SIGNAL(triggered()), this, SLOT(penColor()));

    penWidthAct = new QAction(tr("Pen Width..."), this);
    connect(penWidthAct, SIGNAL(triggered()), this, SLOT(penWidth()));

    clearScreenAct = new QAction(tr("Clear Screen"), this);

    connect(clearScreenAct, SIGNAL(triggered()),
            scribbleArea, SLOT(clearImage()));

//     aboutAct = new QAction(tr("About"), this);
//     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    QMenu *contextMenu;
    contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction(openAct);
    contextMenu->addAction(saveAct);
    contextMenu->addAction(saveAsAct);
    contextMenu->addSeparator();

      //   optionMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(penColorAct);
    contextMenu->addAction(penWidthAct);
    contextMenu->addSeparator();
    contextMenu->addAction(clearScreenAct);

      //   fileMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addSeparator();
}

bool MainWindow::maybeSave()
{
    if (scribbleArea->isModified()) {
         QMessageBox *box = new QMessageBox( tr("Save?"),
                          tr("<qt>The image changed.<br>"
                             "Save it?</qt>"),
                          QMessageBox::Warning,
                          QMessageBox::Yes | QMessageBox::Default,
                          QMessageBox::Cancel | QMessageBox::Escape, 0);

         int result = QtopiaApplication::execDialog(box, true);

        switch(result) {
            case QMessageBox::Yes:
                  return save();
                  break;
          };
    }
    return true;
}

void MainWindow::createSelector()
{
    if (!selector) {
        selector = new QDocumentSelectorService(this);

        QContentFilter filter;
        foreach (QByteArray format, QImageReader::supportedImageFormats()) {
            filter |= QContentFilter(QMimeType::fromExtension(QString(format)));
        }
        selector->setFilter( filter );

        foreach (QByteArray format, QImageWriter::supportedImageFormats()) {
            QString type = QMimeType::fromExtension( QString( format ) ).id();

            if (!type.isEmpty() && !saveTypes.contains(type)) {
                if (type == QLatin1String("image/png"))
                    saveTypes.prepend(type);
                else
                    saveTypes.append(type);
            }
        }
    }
}

void MainWindow::colorSelected(const QColor &newColor)
{
  cselect->hide();
  scribbleArea->setPenColor(newColor);
  cselect->close();

}


void MainWindow::setPenWidth()
{
  dialog->hide();
  scribbleArea->setPenWidth( spinbox->value());
  dialog->close();

  scribbleArea->setFocus();
}

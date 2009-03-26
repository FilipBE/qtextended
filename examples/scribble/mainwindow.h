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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class QIODevice;
class QDocumentSelectorService;
class QColorSelectorDialog;
class QSpinBox;
class ScribbleArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = 0, Qt::WFlags f = 0 );

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void open();
    bool save();
    bool saveAs();
    void penColor();
    void penWidth();
    void about();
    void colorSelected(const QColor &);
    void setPenWidth();

private:
    void createActions();
    void createMenus();
    void createSelector();
    bool maybeSave();

    ScribbleArea *scribbleArea;


    QMenu *fileMenu;
    QMenu *optionMenu;
    QMenu *helpMenu;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;

    QAction *exitAct;
    QAction *penColorAct;
    QAction *penWidthAct;
    QAction *printPdfAct;
    QAction *clearScreenAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QColorSelectorDialog *cselect;
    QSpinBox *spinbox;
    QDialog *dialog;
    QDocumentSelectorService *selector;
    QStringList saveTypes;


};

#endif

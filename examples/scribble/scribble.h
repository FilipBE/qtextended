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

#ifndef SCRIBBLE_H
#define SCRIBBLE_H

#include <qdocumentselector.h>

#include <qmainwindow.h>

class Scribbler;

class QContent;

class QAction;
class QListView;
class QMenuBar;
class QString;
class QToolBar;

class Scribble: public QMainWindow
{
    Q_OBJECT

public:
    Scribble(QWidget *parent = 0, const char *name = 0, int wFlags = 0);
    ~Scribble(void);

private slots:
    void            newScribble(void);
    void            editScribble(const QContent &f);

private:
    void            updateActions(void);
    bool            load(const QString &filename, Scribbler &s);
    void            save(const QString &filename, const Scribbler &s);

    Scribbler       *scribbleWindow;

    QAction         *newAction;
    QMenuBar        *menubar;
    QToolBar        *toolbar;

    QDocumentSelector    *filelist;
};

#endif

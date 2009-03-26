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

#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

#include <QDialog>
#include <QPixmap>
#include <QTimer>
#include <QKeyEvent>

#include "ui_simpleplayerbase.h"

#include "basicmedia.h"

class QTimer;
class QPixmap;
class QDocumentSelectorDialog;


class SimplePlayer : public QWidget, public Ui_SimplePlayerBase
{
    Q_OBJECT
public:
    SimplePlayer(QWidget* p, Qt::WFlags f);
    ~SimplePlayer();

    void showEvent( QShowEvent *e );

public slots:
    void newFile();
    void fileSelector();
    void play();
    void pause();
    void stop();

protected:
    void keyReleaseEvent( QKeyEvent * );

private:
    BasicMedia*               picture;
    QDocumentSelectorDialog*  docs;
};

#endif

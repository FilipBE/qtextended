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

#ifndef QTMAILWINDOW_H
#define QTMAILWINDOW_H

#include <qevent.h>
#include <qlist.h>
#include "emailclient.h"
#include "readmail.h"

class MailListView;
class WriteMail;
class StatusDisplay;

class QStackedWidget;

class QTMailWindow : public QWidget
{
    Q_OBJECT

public:
    QTMailWindow(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~QTMailWindow();

    static QTMailWindow *singleton();

    void forceHidden(bool hidden);
    void setVisible(bool visible);

    QWidget* currentWidget() const;

public slots:
    void raiseWidget(QWidget *, const QString &);

    void closeEvent(QCloseEvent *e);
    void setDocument(const QString &);

protected:
    void init();
    void showEvent(QShowEvent *e);

    EmailClient *emailClient;
    QStackedWidget *views;
    StatusDisplay *status;
    bool noShow;

    static QTMailWindow *self; //singleton
};

#endif

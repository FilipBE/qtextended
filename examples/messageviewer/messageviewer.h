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

#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H
#include "ui_messageviewerbase.h"

class QMailMessageId;

class MessageViewer : public QWidget, public Ui_MessageViewerBase
{
    Q_OBJECT
public:
    MessageViewer( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MessageViewer();

private slots:
    void showMessageList();
    void viewMessage(const QMailMessageId&);
    void showContactList();

private:
    QWidget* contactSelector;
    QWidget* messageSelector;
};

#endif

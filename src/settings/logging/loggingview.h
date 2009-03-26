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
#ifndef LOGGINGVIEW_H
#define LOGGINGVIEW_H

#include <QTextEdit>
#include <QBasicTimer>

class QProcess;

class LoggingView : public QTextEdit
{
    Q_OBJECT

public:
    LoggingView(QWidget* parent = 0, Qt::WFlags = 0);
    ~LoggingView();

    void resizeEvent(QResizeEvent*);
    void showEvent(QShowEvent*);
    bool isAvailable() const;

private slots:
    void settings();
    void readLogFollow();
    void init();

private:
    void closeLogFollow();
    QProcess *logfollow;
};

#endif

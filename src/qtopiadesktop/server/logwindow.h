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
#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>

class QTextEdit;

class LogWindow : public QWidget
{
    Q_OBJECT
private:
    LogWindow();
public:
    virtual ~LogWindow();

    static LogWindow *getInstance();

private slots:
    void statusMessage( const QString &message, const QByteArray &data );
    void connectionMessage( const QString &message, const QByteArray &data );

private:
    void closeEvent( QCloseEvent *e );
    void addText( const QString &msg );

    QTextEdit *textView;
};

#endif

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

#ifndef QTESTVERIFYDLG_P_H
#define QTESTVERIFYDLG_P_H

#include <QDialog>

class QLabel;
class QPushButton;
class QTabWidget;

class QTestVerifyDlg : public QDialog
{
    Q_OBJECT

public:
    QTestVerifyDlg(QWidget* parent = 0);
    ~QTestVerifyDlg();

    void setData( const QPixmap &actual, const QPixmap &expected, const QString &comment = QString() );

private:
    QLabel *actualLabel;
    QLabel *expectedLabel;
    QLabel *questionLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QTabWidget *leftTabWidget;
    QTabWidget *rightTabWidget;

    QString questionText;
};

#endif

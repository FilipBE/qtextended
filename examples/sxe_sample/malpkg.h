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

#ifndef MALPKG_H
#define MALPKG_H

#include <qdialog.h>
#include <QWizard>
class QWizardPage;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QCheckBox;

class MalPkg : public QWizard
{
    Q_OBJECT

public:
    MalPkg( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~MalPkg();

private slots:
    void showPage( int );
    void appMessage( const QString&, const QByteArray& );
    void pwdExploitDo();
    void qcopExploitDo();

private:
    QWizardPage *initPwdExPage();
    QWizardPage *initQcopExPage();
    QString statusMsg( int );

    QWizard *wiz;
    QTextEdit *pwdExMsgs;
    QLineEdit *pwdExLine;
    QPushButton *pwdExButton;
    QCheckBox *pwdExCheck;
    QTextEdit *qcopExMsgs;
    QLineEdit *qcopExLine;
    QLineEdit *qcopExLineNum;
    QLineEdit *qcopExLineMsg;
    QPushButton *qcopExButton;
    QCheckBox *qcopExCheck;
};

#endif

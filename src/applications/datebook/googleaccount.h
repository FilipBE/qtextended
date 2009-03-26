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
#ifndef GOOGLEACCOUNT_H
#define GOOGLEACCOUNT_H

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;
class QLabel;

#include <private/qgooglecontext_p.h>

#ifndef QT_NO_OPENSSL
class GoogleAccount : public QDialog
{
    Q_OBJECT
public:
    GoogleAccount( QWidget *parent = 0 );
    ~GoogleAccount();

    void setEmail(const QString &);
    void setPassword(const QString &);
    void setName(const QString &);
    void setFeedType(QGoogleCalendarContext::FeedType);

    void accept();

    QString name() const;
    QString email() const;
    QString password() const;
    QGoogleCalendarContext::FeedType feedType() const;

private:
    QLineEdit *emailText;
    QLineEdit *nameText;
    QLineEdit *passwordText;
    QLabel *nameLabel;
    QComboBox *accessCombo;
};
#endif // QT_NO_OPENSSL
#endif

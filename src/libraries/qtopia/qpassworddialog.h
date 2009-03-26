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
#ifndef QPASSWORDDIALOG_H
#define QPASSWORDDIALOG_H

#include <QDialog>

#include <qtopiaglobal.h>
#include <qstring.h>

class QPasswordWidget;

class QTOPIA_EXPORT QPasswordDialog : public QDialog
{
    Q_OBJECT
public:
    enum InputMode { Crypted, Plain, Pin };

    explicit QPasswordDialog( QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~QPasswordDialog();

    void setPrompt( const QString& prompt );
    QString prompt() const;

    void setInputMode( QPasswordDialog::InputMode mode );
    QPasswordDialog::InputMode inputMode() const;

    void reset();
    QString password() const;

    static QString getPassword( QWidget* parent,
                                const QString& prompt,
                                InputMode mode = QPasswordDialog::Crypted,
                                bool last = true );

    static void authenticateUser( QWidget* parent,
                                  bool atPowerOn = false );

    static bool authenticateUser( const QString &text );

private:
    QPasswordWidget* m_passw;
};

#endif

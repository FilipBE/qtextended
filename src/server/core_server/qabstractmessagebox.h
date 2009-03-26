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

#ifndef QABSTRACTMESSAGEBOX_H
#define QABSTRACTMESSAGEBOX_H

#include <QDialog>
#include <QPixmap>
#include "qtopiaserverapplication.h"

class QAbstractMessageBoxPrivate;
class QAbstractMessageBox : public QDialog
{
Q_OBJECT
public:
    enum Icon { NoIcon=0, Question=1, Information=2, Warning=3, Critical=4 };
    enum Button { NoButton=0, Ok=11, Cancel=12, Yes=13, No=14 };

    QAbstractMessageBox(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual void setButtons(Button button1, Button button2) = 0;
    virtual void setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
            int defaultButtonNumber, int escapeButtonNumber) = 0;

    virtual QString title() const = 0;
    virtual void setTitle(const QString &) = 0;

    virtual Icon icon() const = 0;
    virtual void setIcon(Icon) = 0;
    virtual void setIconPixmap(const QPixmap&) = 0;

    virtual QString text() const = 0;
    virtual void setText(const QString &) = 0;

    virtual void setTimeout(int timeoutMs, Button);

    static int critical(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int warning(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int information(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);
    static int question(QWidget *parent, const QString &title, const QString &text, Button button1 = NoButton, Button button2 = NoButton);

    static QAbstractMessageBox * messageBox(QWidget *parent, const QString &title, const QString &text, Icon icon, Button button0=QAbstractMessageBox::Ok, Button button1=QAbstractMessageBox::NoButton);
    static QAbstractMessageBox * messageBoxCustomButton(QWidget *parent, const QString &title, const QString &text, Icon icon,
    const QString & button0Text = QString(), const QString &button1Text = QString(),
    const QString &button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1 );

protected:
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

private:
    QAbstractMessageBoxPrivate *d;
};

#endif

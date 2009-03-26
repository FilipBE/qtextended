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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <qdialog.h>
#include "qabstractmessagebox.h"

class PhoneMessageBoxPrivate;
class PhoneMessageBox : public QAbstractMessageBox
{
Q_OBJECT
public:
    PhoneMessageBox(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual void setButtons(Button button1, Button button2);
    virtual void setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
            int defaultButtoNumber, int escapeButtonNumber);
    virtual QString title() const;
    virtual void setTitle(const QString &);
    virtual Icon icon() const;
    virtual void setIconPixmap(const QPixmap& pixmap);
    virtual void setIcon(Icon);
    virtual QString text() const;
    virtual void setText(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    void addContents(QWidget *);

private slots:
    void buttonClicked(int);

private:
    PhoneMessageBoxPrivate *d;
};

#endif


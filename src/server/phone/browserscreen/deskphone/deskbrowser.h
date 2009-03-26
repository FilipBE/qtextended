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

#ifndef DESKBROWSER_H
#define DESKBROWSER_H

#include "qabstractbrowserscreen.h"

class DeskphoneBrowserStack;
class PhoneThemedView;

class DeskphoneBrowserScreen : public QAbstractBrowserScreen
{
    Q_OBJECT
public:
    DeskphoneBrowserScreen(QWidget *parent, Qt::WFlags flags);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;

public slots:
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    virtual bool eventFilter(QObject *o, QEvent *e);
    virtual void keyPressEvent(QKeyEvent *ke);

private:
    bool homescreenKeyPress(QKeyEvent *e);

protected:
    DeskphoneBrowserStack *m_stack;
};

#endif
